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

rm -rf wasm/packages/dist
mkdir -p wasm/packages/dist
mv out/out/melt.js wasm/packages/mlt/dist/melt.js
mv out/out/melt.wasm wasm/packages/mlt/dist/melt.wasm
mv out/out/melt.worker.js wasm/packages/mlt/dist/melt.worker.js
