#!/bin/bash
#
# Common variables for all scripts

set -euo pipefail

# Include llvm binaries
export PATH=$PATH:$EMSDK/upstream/bin

# Flags for code optimization, focus on size instead
# of speed
OPTIM_FLAGS=(
  -Os
)

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  # Use closure complier only in linux environment
  OPTIM_FLAGS=(
    "${OPTIM_FLAGS[@]}"
    --closure 1
  )
fi

# Convert array to string
OPTIM_FLAGS="${OPTIM_FLAGS[@]}"

# Root directory
ROOT_DIR=$PWD

# Directory to install headers and libraries
BUILD_DIR=$ROOT_DIR/build

# Directory to look for pkgconfig files
EM_PKG_CONFIG_PATH=$BUILD_DIR/lib/pkgconfig

# Toolchain file path for cmake
TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

CFLAGS="-s USE_PTHREADS=1 -I$BUILD_DIR/include $OPTIM_FLAGS -std=gnu11"
LDFLAGS="$CFLAGS -L$BUILD_DIR/lib"
FFMPEG_CONFIG_FLAGS_BASE=(
  --target-os=none        # use none to prevent any os specific configurations
  --arch=x86_32           # use x86_32 to achieve minimal architectural optimization
  --enable-cross-compile  # enable cross compile
  --disable-x86asm        # disable x86 asm
  --disable-inline-asm    # disable inline asm
  --disable-stripping     # disable stripping
  --disable-programs      # disable programs build (incl. ffplay, ffprobe & ffmpeg)
  --disable-doc           # disable doc
  --disable-debug         # disable debug info, required by closure
  --disable-runtime-cpudetect   # disable runtime cpu detect
  --disable-autodetect    # disable external libraries auto detect
  --extra-cflags="$CFLAGS"
  --extra-cxxflags="$CFLAGS"
  --extra-ldflags="$LDFLAGS"
  --pkg-config-flags="--static"
  --nm="llvm-nm"
  --ar=emar
  --ranlib=emranlib
  --cc=emcc
  --cxx=em++
  --objcc=emcc
  --dep-cc=emcc
)

echo "EMSDK=$EMSDK"
echo "OPTIM_FLAGS=$OPTIM_FLAGS"
echo "BUILD_DIR=$BUILD_DIR"
