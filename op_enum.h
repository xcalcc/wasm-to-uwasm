//
// Created by xc5 on 2020/7/22.
//

#ifndef WASM2UWASM_OP_ENUM_H
#define WASM2UWASM_OP_ENUM_H

#define WASM_OP(a,b,c,d,r1,r2,e,numopr,ret_size, encode_8bit, enum_name,i,j) enum_name = encode_8bit,
#define UWASM_OP(name, wasm_op, encode_8bit, ty_am, enum_name) enum_name = encode_8bit,
#define ___ NO_TYPE
#define NIL NO_TYPE

enum WINST_OPCODE {
  #include "winst_opcode.h"
};

enum UWINST_OPCODE {
#include "uwinst_opcode.h"
};


#endif //WASM2UWASM_OP_ENUM_H
