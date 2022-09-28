//
// Created by xc5 on 2020/7/28.
//

#ifndef WASM2UWASM_EXECUTOR_H
#define WASM2UWASM_EXECUTOR_H

#include <cstdio>
#include "basics.h"
#include "executor.h"
#include "uwinst.h"
#include "op_enum.h"

enum REG_TY_KIND {
  RTI32,
  RTI64,
  RTF32,
  RTF64,
};

enum DEDICATED_REG {
  REGISTER_zero     = 0,
  REGISTER_one      = 1,
  REGISTER_return   = 2,
  REGISTER_return_2 = 3,
  REGISTER_max      = (1 << 10) - 1,
};

#define MEMORY_INITIAL_SIZE (64 * 1024)

class UWASM_MEMORY{
private:
  UINT8      *_data;
  UINT32      _size;
  UINT32      _real_size;
public:
  UWASM_MEMORY() {
    _size      = MEMORY_INITIAL_SIZE;
    _real_size = _size;
    _data      = new UINT8[_size];
  }
  const UINT8 *Get_data() const {
    return _data;
  }
  void Print(FILE * file = stderr);
};


class UWASM_FRAME {
private:
  UINT32 _i32bp = 0;
  UINT32 _i64bp = 0;
  UINT32 _f32bp = 0;
  UINT32 _f64bp = 0;
  UINT32 _i32sz = 0;
  UINT32 _i64sz = 0;
  UINT32 _f32sz = 0;
  UINT32 _f64sz = 0;
  UINT32 _i32regs[4096];
  UINT64 _i64regs[4096];
  float  _f32regs[4096];
  double _f64regs[4096];
  UWASM_MEMORY *_memory;
public:

  UWASM_FRAME() {
    _memory = new UWASM_MEMORY();
  }

  void Print(FILE *file = stdout);

  UINT32 Get_i32_bp() const {
    return _i32bp;
  }

  void Set_i32_bp(UINT32 i32Bp) {
    _i32bp = i32Bp;
  }

  UINT32 Get_i64_bp() const {
    return _i64bp;
  }

  void Set_i64_bp(UINT32 i64Bp) {
    _i64bp = i64Bp;
  }

  UINT32 Get_f32_bp() const {
    return _f32bp;
  }

  void Set_f32_bp(UINT32 f32Bp) {
    _f32bp = f32Bp;
  }

  UINT32 Get_f64_bp() const {
    return _f64bp;
  }

  void Set_f64_bp(UINT32 f64Bp) {
    _f64bp = f64Bp;
  }

  UINT32 Get_i32_sz() const {
    return _i32sz;
  }

  void Set_i32_sz(UINT32 i32Sz) {
    _i32sz = i32Sz;
  }

  UINT32 Get_i64_sz() const {
    return _i64sz;
  }

  void Set_i64_sz(UINT32 i64Sz) {
    _i64sz = i64Sz;
  }

  UINT32 Get_f32_sz() const {
    return _f32sz;
  }

  void Set_f32_sz(UINT32 f32Sz) {
    _f32sz = f32Sz;
  }

  UINT32 Get_f64_sz() const {
    return _f64sz;
  }

  void Set_f64_sz(UINT32 f64Sz) {
    _f64sz = f64Sz;
  }

  const UINT32 *Get_i32_regs() const {
    return _i32regs;
  }

  const UINT64 *Get_i64_regs() const {
    return _i64regs;
  }

  const float *Get_f32_regs() const {
    return _f32regs;
  }

  const double *Get_f64_regs() const {
    return _f64regs;
  }

  template<REG_TY_KIND KIND>
  UINT32 Get_reg(UINT16 reg_id);
  template<REG_TY_KIND KIND, typename VALT>
  void Set_reg(UINT16 reg_id, VALT val);
  UWASM_MEMORY *Get_memory() const { return _memory; }
};


using std::vector;
using INSTVEC = vector<UWASM_INST>;

INT32 Execute_function_body(UWASM_FRAME *frame, INSTVEC *vec);

template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_load(UWASM_FRAME *frame, UWASM_INST *it);

template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_ret(UWASM_FRAME *frame, UWASM_INST *it);

template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_add(UWASM_FRAME *frame, UWASM_INST *it);

#endif //WASM2UWASM_EXECUTOR_H
