#!/usr/bin/env bash
cd $(dirname $0)
./build.sh
cd build
./wabt_objdump -d ../tests/simple/subtract.wasm
