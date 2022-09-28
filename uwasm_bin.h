//
// Created by xc5 on 2020/7/25.
//

#ifndef WASM2UWASM_UWASM_BIN_H
#define WASM2UWASM_UWASM_BIN_H

#include "basics.h"

typedef  UINT32             LABEL_IDX;
typedef  UINT32             LOCAL_IDX;
typedef  UINT32             REG_IDX;
typedef  UINT32             FUNC_IDX;

#define LABEL_INVALID       (0)
#define LABEL_NOT_NEEDED    (0xFFFFFFFF)
#define BLOCK_FUNCTION      1

#endif //WASM2UWASM_UWASM_BIN_H
