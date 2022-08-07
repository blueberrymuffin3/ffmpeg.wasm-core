#include "avdevice.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include <emscripten.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdint.h>

typedef struct PIXIContext {
  AVClass *class;
  AVFormatContext *ctx;
  AVStream *st;
  _Atomic int32_t timeMs;
} PIXIContext;

#define AV_TIME_BASE_MS                                                        \
  (AVRational) { 1, 1000 }

static int pixi_write_trailer(AVFormatContext *s) {
  PIXIContext *c = s->priv_data;

  MAIN_THREAD_ASYNC_EM_ASM({ window["PIXI_STREAM"] = null; });

  return 0;
}

static int pixi_write_header(AVFormatContext *s) {
  PIXIContext *c = s->priv_data;
  AVStream *st = s->streams[0];
  AVCodecParameters *encctx = st->codecpar;

  c->timeMs = 1;

  MAIN_THREAD_ASYNC_EM_ASM(
      {
        window["PIXI_STREAM"] = new Object();
        window["PIXI_STREAM"]["width"] = $0;
        window["PIXI_STREAM"]["height"] = $1;
        window["PIXI_STREAM"]["data"] = null;
        window["PIXI_STREAM"]["ptsMs"] = null;
        let index = $2 / 4;
        window["PIXI_STREAM"]["timeMs"] = HEAP32.subarray(index, index + 1);
      },
      st->codecpar->width, st->codecpar->height, &c->timeMs);

  int ret, bpp;

  c->ctx = s;
  c->st = st;

  if (s->nb_streams > 1 || encctx->codec_type != AVMEDIA_TYPE_VIDEO ||
      encctx->codec_id != AV_CODEC_ID_RAWVIDEO) {
    av_log(s, AV_LOG_ERROR, "Only supports one rawvideo stream\n");
    return AVERROR(EINVAL);
  }

  if (encctx->format != AV_PIX_FMT_RGB24) {
    av_log(s, AV_LOG_ERROR, "Unsupported pixel format '%s', choose rgb24\n",
           av_get_pix_fmt_name(encctx->format));
    return AVERROR(EINVAL);
  }

  bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(encctx->format));
  return 0;
}

static int pixi_write_packet(AVFormatContext *s, AVPacket *pkt) {
  PIXIContext *c = s->priv_data;

  int32_t timeMs = atomic_load(&c->timeMs);

  int32_t ptsMs = av_rescale_q(pkt->pts, c->st->time_base, AV_TIME_BASE_MS);
  int32_t ptsNextMs =
      av_rescale_q(pkt->pts + pkt->duration, c->st->time_base, AV_TIME_BASE_MS);

  if (timeMs > ptsNextMs) {
    av_log(s, AV_LOG_WARNING, "Playback lagging, %u ms behind\n",
           timeMs - ptsNextMs);
    return 0;
  }

  MAIN_THREAD_ASYNC_EM_ASM(
      {
        window["PIXI_STREAM"]["data"] = HEAPU8.subarray($0, $0 + $1);
        window["PIXI_STREAM"]["ptsMs"] = $2;
      },
      pkt->data, pkt->size, ptsMs);

  atomic_store(&c->timeMs, 0);

  av_log(s, AV_LOG_INFO, "Sent frame (pts: %d ms)\n", ptsMs);

  // TODO: See if you can use Atomics.wait to make this better
  while (atomic_load(&c->timeMs) == 0) {
    usleep(1000);
  }

  av_log(s, AV_LOG_INFO, "Frame recieved\n");

  return 0;
}

#define OFFSET(x) offsetof(PIXIContext, x)
#define ENC AV_OPT_FLAG_ENCODING_PARAM

static const AVOption options[] = {
    {NULL},
};

static const AVClass pixi_class = {
    .class_name = "pixi outdev",
    .item_name = av_default_item_name,
    .option = options,
    .version = LIBAVUTIL_VERSION_INT,
    .category = AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT,
};

AVOutputFormat ff_pixi_muxer = {
    .name = "pixi",
    .long_name = NULL_IF_CONFIG_SMALL("pixi output device"),
    .priv_data_size = sizeof(PIXIContext),
    .audio_codec = AV_CODEC_ID_NONE,
    .video_codec = AV_CODEC_ID_RAWVIDEO,
    .write_header = pixi_write_header,
    .write_packet = pixi_write_packet,
    .write_trailer = pixi_write_trailer,
    .flags = AVFMT_NOFILE,
    .priv_class = &pixi_class,
};
