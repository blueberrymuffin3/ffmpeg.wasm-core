/*
 * Copyright (c) 2012 Paul B Mahol
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avdevice.h"

typedef struct PIXIContext {
    AVClass         *class;
    AVFormatContext *ctx;
} PIXIContext;

static int pixi_write_trailer(AVFormatContext *s)
{
    PIXIContext *c = s->priv_data;

    return 0;
}

static int pixi_write_header(AVFormatContext *s)
{
    PIXIContext *c = s->priv_data;
    AVStream *st = s->streams[0];
    AVCodecParameters *encctx = st->codecpar;
    int ret, bpp;

    c->ctx = s;

    if (   s->nb_streams > 1
        || encctx->codec_type != AVMEDIA_TYPE_VIDEO
        || encctx->codec_id   != AV_CODEC_ID_RAWVIDEO) {
        av_log(s, AV_LOG_ERROR, "Only supports one rawvideo stream\n");
        return AVERROR(EINVAL);
    }

    if (encctx->format != AV_PIX_FMT_RGB24) {
        av_log(s, AV_LOG_ERROR,
               "Unsupported pixel format '%s', choose rgb24\n",
               av_get_pix_fmt_name(encctx->format));
        return AVERROR(EINVAL);
    }

    av_log(s, AV_LOG_INFO, "Frame Time: %ld μs", av_rescale_q(1, st->codec->time_base, AV_TIME_BASE_Q));

    bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(encctx->format));
    return 0;
}

static int pixi_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    PIXIContext *c = s->priv_data;

    // av_log(s, AV_LOG_INFO, "write_packet %hhu %hhu %hhu %hhu\n", pkt->data[0], pkt->data[1], pkt->data[2]);

    return 0;
}

#define OFFSET(x) offsetof(PIXIContext,x)
#define ENC AV_OPT_FLAG_ENCODING_PARAM

static const AVOption options[] = {
    { NULL },
};

static const AVClass pixi_class = {
    .class_name = "pixi outdev",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
    .category   = AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT,
};

AVOutputFormat ff_pixi_muxer = {
    .name           = "pixi",
    .long_name      = NULL_IF_CONFIG_SMALL("pixi output device"),
    .priv_data_size = sizeof(PIXIContext),
    .audio_codec    = AV_CODEC_ID_NONE,
    .video_codec    = AV_CODEC_ID_RAWVIDEO,
    .write_header   = pixi_write_header,
    .write_packet   = pixi_write_packet,
    .write_trailer  = pixi_write_trailer,
    .flags          = AVFMT_NOFILE,
    .priv_class     = &pixi_class,
};
