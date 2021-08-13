#!/bin/bash

set -euo pipefail
echo
echo "###################### build-libxml2 ######################"
echo
source $(dirname $0)/var.sh

LIB_PATH=third_party/libxml2
CM_FLAGS=(
  -DCMAKE_INSTALL_PREFIX=$BUILD_DIR
  # -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE
  -DBUILD_SHARED_LIBS=OFF
  -DLIBXML2_WITH_DEBUG=OFF
  -DLIBXML2_WITH_C14N=OFF
  -DLIBXML2_WITH_CATALOG=OFF
  -DLIBXML2_WITH_DEBUG=OFF
  -DLIBXML2_WITH_DOCB=OFF
  -DLIBXML2_WITH_FTP=OFF
  -DLIBXML2_WITH_HTML=OFF
  -DLIBXML2_WITH_HTTP=OFF
  -DLIBXML2_WITH_ICONV=OFF
  -DLIBXML2_WITH_ICU=OFF
  -DLIBXML2_WITH_ISO8859X=OFF
  -DLIBXML2_WITH_LEGACY=OFF
  -DLIBXML2_WITH_LZMA=OFF
  -DLIBXML2_WITH_MEM_DEBUG=OFF
  -DLIBXML2_WITH_MODULES=OFF
  -DLIBXML2_WITH_OUTPUT=OFF
  -DLIBXML2_WITH_PATTERN=OFF
  -DLIBXML2_WITH_PROGRAMS=OFF
  -DLIBXML2_WITH_PUSH=OFF
  -DLIBXML2_WITH_PYTHON=OFF
  -DLIBXML2_WITH_READER=OFF
  -DLIBXML2_WITH_REGEXPS=OFF
  -DLIBXML2_WITH_RUN_DEBUG=OFF
  -DLIBXML2_WITH_SAX1=OFF
  -DLIBXML2_WITH_SCHEMAS=OFF
  -DLIBXML2_WITH_SCHEMATRON=OFF
  -DLIBXML2_WITH_TESTS=OFF
  # -DLIBXML2_WITH_THREADS=OFF
  # -DLIBXML2_WITH_THREAD_ALLOC=OFF
  # -DLIBXML2_WITH_TREE=OFF
  -DLIBXML2_WITH_VALID=OFF
  -DLIBXML2_WITH_WRITER=OFF
  -DLIBXML2_WITH_XINCLUDE=OFF
  -DLIBXML2_WITH_XPATH=OFF
  -DLIBXML2_WITH_XPTR=OFF
  -DLIBXML2_WITH_ZLIB=OFF
)
echo "CM_FLAGS=${CM_FLAGS[@]}"

cd $LIB_PATH
rm -f build/CMakeCache.txt
mkdir -p build
cd build
emmake cmake .. -DCMAKE_C_FLAGS="$CXXFLAGS" ${CM_FLAGS[@]}
# emmake make clean
emmake make install
cd $ROOT_DIR
