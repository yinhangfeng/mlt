#!/bin/bash
#
# Common variables for all scripts

set -euo pipefail
echo
echo "###################### build-mlt ######################"
echo

mkdir -p out
cd out
rm -f CMakeCache.txt
emcmake cmake ../wasm
emmake make clean
emmake make

cd ..

rm -rf wasm/packages/mlt/dist
cp -r out/out/wasm wasm/packages/mlt/dist
