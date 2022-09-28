#ifndef WASM2UWASM_READER_H
#define WASM2UWASM_READER_H

#include "basics.h"
#include "uwinst.h"
#include "winst.h"
#include "ir.h"
#include "uwasm_bin.h"

typedef enum {
  FILE_WASM  = 1,
  FILE_UWASM = 2,
} WASM_FILE_KIND;

class FILE_CONTEXT {
private:
  WASM_FILE_KIND    _kind;             // wasm or uwasm
  BOOL              _emit_binary;      // whether to emit binary dump
  BOOL              _full_file;        // whether the file contains all sections
  wabt::Module     *_module;           // wabt's module
  FUNC_IDX          _func_ctx;         // current function index
public:
  WASM_FILE_KIND     Kind()                const   { return _kind;                                       }
  BOOL               Is_emit_binary()      const   { return _emit_binary;                                }
  BOOL               Is_full_file()        const   { return _full_file;                                  }
  wabt::Module      *Get_module()          const   { return _module;                                     }
  wabt::Func        *Get_func()            const   { return Get_module()->GetFunc(wabt::Var(_func_ctx)); }
  wabt::Func        *Next_func()                   { _func_ctx++; return Get_func();                     }

  void      Set_kind (WASM_FILE_KIND kind)         {  _kind = kind;                                      }
  void      Set_emit_binary (BOOL opt)             {  _emit_binary = opt;                                }
  void      Set_full_file (BOOL opt)               {  _full_file = opt;                                  }
  void      Set_module(wabt::Module &module)       {  _module = &module;                                 }
  void      Set_func_ctx(FUNC_IDX func_ctx)        {  _func_ctx = func_ctx;                              }

  FILE_CONTEXT() {
    _emit_binary = true;
    _full_file   = false;
    _kind        = FILE_UWASM;
  }
};

UINT8 Read_uint8(IO_CBUFFER pc);

UINT64 Read_leb128(IO_CBUFFER pc);
UINT8  Read_opr_size(IO_CBUFFER pc);

template <typename T>
T Read_uint(IO_CBUFFER pc, UINT32 _sz=0) {
  UINT32 _res_sz = sizeof(T) * 8;
  if (_sz == 0) _sz = _res_sz;
  T res = *(T *)pc;
  res &= (~((T)0x0) >> (_res_sz - _sz));
  return res;
}

INT32 Read_insts(IO_CBUFFER inst_buf, UINT32 buf_len, FILE_CONTEXT &ctx);
INT32 Read_all_sections(IO_CBUFFER module_buf, UINT32 length, FILE_CONTEXT &ctx);
INT32 Read_magic_number(IO_CBUFFER buffer, UINT32 &pc, UINT32 length);
UINT32 Read_available(IO_CBUFFER buffer, UINT32 &pc, UINT32 length);
void Read_opr_from_wasm_inst(IO_CBUFFER pc, WASM_INST &inst);


#endif //WASM2UWASM_READER_H
