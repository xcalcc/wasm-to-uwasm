#!/usr/bin/env bash
cd $(dirname $0)
echo "Workdir : $(pwd)"
mkdir build
cd build
cmake ..
make wabt_objdump