#!/bin/bash

set -euo pipefail
source $(dirname $0)/var.sh

FLAGS=(
  "${FFMPEG_CONFIG_FLAGS_BASE[@]}"
  --disable-everything
  --enable-parser=h264
  --enable-protocol=file
  --enable-demuxer=matroska
  --enable-demuxer=avi
  --enable-demuxer=flv
  --enable-muxer=mp4
  --enable-decoder=h264
  --enable-decoder=mpeg4
  --enable-encoder=rawvideo
  --enable-filter=scale
  --enable-outdev=pixi
)
echo "FFMPEG_CONFIG_FLAGS=${FLAGS[@]}"
EM_PKG_CONFIG_PATH=${EM_PKG_CONFIG_PATH} emconfigure ./configure "${FLAGS[@]}"
