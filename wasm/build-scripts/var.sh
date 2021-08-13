#!/bin/bash
#
# Common variables for all scripts

set -euo pipefail

# Include llvm binaries
export PATH=$PATH:$EMSDK/upstream/bin

# if yes, we are building a single thread version of
# ffmpeg.wasm-core, which is slow but compatible with
# most browsers as there is no SharedArrayBuffer.
DISABLE_PTHREAD=${DISABLE_PTHREAD:-no}

# Root directory
ROOT_DIR=$PWD

# Directory to install headers and libraries
BUILD_DIR=$ROOT_DIR/build

# Directory to look for pkgconfig files
EM_PKG_CONFIG_PATH=$BUILD_DIR/lib/pkgconfig

# Toolchain file path for cmake
TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

# Flags for code optimization, focus on speed instead
# of size
# OPTIM_FLAGS="-O3"

# if [[ "$OSTYPE" == "linux-gnu"* ]]; then
#   # Use closure complier only in linux environment
#   OPTIM_FLAGS="$OPTIM_FLAGS --closure 1"
# fi

# Unset OPTIM_FLAGS can speed up build
OPTIM_FLAGS=""

CFLAGS_BASE="$OPTIM_FLAGS -I$BUILD_DIR/include"
CFLAGS="$CFLAGS_BASE  -s USE_PTHREADS=1"

# if [[ "$DISABLE_PTHREAD" == "yes" ]]; then
#   CFLAGS="$CFLAGS_BASE"
#   EXTRA_FFMPEG_CONF_FLAGS="--disable-pthreads --disable-w32threads --disable-os2threads"
# fi

export CFLAGS=$CFLAGS
export CXXFLAGS=$CFLAGS
export LDFLAGS="$CFLAGS -L$BUILD_DIR/lib"
export STRIP="llvm-strip"
export EM_PKG_CONFIG_PATH=$EM_PKG_CONFIG_PATH

echo "EMSDK=$EMSDK"
echo "DISABLE_PTHREAD=$DISABLE_PTHREAD"
echo "CFLAGS(CXXFLAGS)=$CFLAGS"
echo "BUILD_DIR=$BUILD_DIR"
