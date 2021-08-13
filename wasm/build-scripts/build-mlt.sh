#!/bin/bash
#
# Common variables for all scripts

set -euo pipefail

mkdir -p out/wasm
cd out/wasm
rm -f CMakeCache.txt
emcmake cmake ../../wasm
emmake make clean
emmake make
cd ../..
