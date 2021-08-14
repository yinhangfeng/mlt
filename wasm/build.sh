#!/bin/bash

set -eo pipefail

SCRIPT_ROOT=$(dirname $0)/build-scripts

run() {
  for name in $@; do
    $SCRIPT_ROOT/$name.sh
  done
}

run-all() {
  SCRIPTS=(
    # install dependencies
    # install-deps
    # build-ffmpeg
    # build-libxml2
    build-mlt
  )
  run ${SCRIPTS[@]}
}

main() {
  # verify Emscripten version
  emcc -v
  if [[ "$@" == "" ]]; then
    run-all
  else
    run "$@"
  fi
}

main "$@"
