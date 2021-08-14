mlt.wasm
================

## Setup

```
git submodule update --init --recursive
```

## Build

1. Use docker (easy way)

Install latest docker and run `build-with-docker.sh`.

```
$ bash wasm/build-with-docker.sh
```

2. Install emsdk (unstable way)

Setup the emsdk from [HERE](https://emscripten.org/docs/getting_started/downloads.html) and run `build.sh`.

```
$ bash wasm/build.sh
```

If nothing goes wrong, you can find JavaScript files in `wasm/packages/mlt/dist`.
