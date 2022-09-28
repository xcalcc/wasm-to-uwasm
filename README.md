# Getting started

Clone this repo and run
```
./build.sh
cd build
./wabt_objdump -d ../tests/simple/subtract.wasm
```

And you shall see the dump in the terminal.

## Caveat per-se

The register number generated for uWASM are just some hand-picken numbers, they only demonstrate that there should be a register index of such size.

## CI/CD 

The project will be automatically built and tested against
some basic examples on CI/CD.

See details [here](gitlab-address/wasm-to-uwasm/pipelines)


## Bugs

Please file bugs to [here](gitlab-address/wasm-to-uwasm/-/issues)

