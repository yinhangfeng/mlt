#!/bin/bash

set -euo pipefail
echo
echo "###################### build-ffmpeg ######################"
echo

LIB_PATH=third_party/ffmpeg

cd $LIB_PATH

build.sh \
build-zlib \
build-x264 \
build-x265 \
build-libvpx \
build-wavpack \
build-lame \
build-fdk-aac \
build-ogg \
build-vorbis \
build-theora \
build-opus \
build-libwebp \
build-freetype2 \
build-fribidi \
build-harfbuzz \
build-libass \
configure-ffmpeg \
build-ffmpeg

cd ../../
