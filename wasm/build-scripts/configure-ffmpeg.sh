#!/bin/bash

set -euo pipefail
source $(dirname $0)/var.sh

FLAGS=(
  "${FFMPEG_CONFIG_FLAGS_BASE[@]}"
  --disable-everything
  --enable-protocol=file
  --enable-demuxer=mkv
  --enable-demuxer=avi
  --enable-demuxer=flv
  --enable-muxer=mp4
)
echo "FFMPEG_CONFIG_FLAGS=${FLAGS[@]}"
emconfigure ./configure "${FLAGS[@]}"
