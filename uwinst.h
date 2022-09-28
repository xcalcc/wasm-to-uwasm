#ifndef WASM2UWASM_UWINST_H
#define WASM2UWASM_UWINST_H

#include <cstdio>
#include <string>
#include "basics.h"

#define MN_SZ         8    // total mnemonics 2**MN_SZ
#define OFS_SSZ      12    // small offset range (2**OFS_SSZ << 5) assume 32 bit alignment
#define REGSET_SZ    12    // register set size 2**REGSET_SZ
#define INST_LSZ     64    // instruction (large) size
#define INST_SSZ     32    // instruction (small) size

typedef enum {
    TY_AM32,
    TY_AM32_R,
    TY_AM32_OFS,
    TY_AM32_R_OFS,
    TY_AM32_R_R,
    TY_AM32_IMM_OFS,
    TY_AM64_R_R_R,
    TY_AM64_R_OFS,
    TY_AM64_R_R_OFS,
} TY_AM;

#define B_SIZE 1
#define REG_SIZE 10
#define MN_SIZE 7
#define MN2_SIZE 8
#define SOFS_SIZE 20

// we assume address mode as below - load.i32 from local memory using small offset
// mnemonic    reg, sofs   // e.g. ldl  rd5, 0x0FFF   ; loadilocal32s dest_reg, (0xfff<<5)
//                         // bits   8   12    12  = 32
// reverse for little endian

// inst without operand
typedef struct Am32 {
    UINT32 b32  : B_SIZE;     // small 32 bit inst if 0
    UINT32 mn   : MN_SIZE;    // one bit reserved for 64 bit instructions
    UINT32 dum  : (32 - B_SIZE - MN_SIZE);   // no fill
} AM32;


typedef struct Am32_r {
    UINT32 b32  : B_SIZE;     // small 32 bit inst if 0
    UINT32 mn   : MN_SIZE;    // one bit reserved for 64 bit instructions
    UINT32 rd   : REG_SIZE;   // dest reg
    UINT32 dum  : (32 - B_SIZE - MN_SIZE - REG_SIZE);  // no fill
} AM32_R;


typedef struct Am32_ofs {
    UINT32 b32  : B_SIZE;     // small 32 bit inst if 0
    UINT32 mn   : MN_SIZE;    // one bit reserved for 64 bit instructions
    UINT32 lofs  : (32 - B_SIZE - MN_SIZE);
} AM32_OFS;


typedef struct Am32_r_ofs {
    UINT32 b32  : B_SIZE;     // small 32 bit inst if 0
    UINT32 mn   : MN_SIZE;    // one bit reserved for 64 bit instructions
    UINT32 rd   : REG_SIZE;   // dest reg
    UINT32 sofs : 12;
} AM32_R_OFS;      // address mode  register, small offset


typedef struct Am32_r_r {
    UINT32 b32  : B_SIZE;     // small 32 bit inst if 0
    UINT32 mn   : MN_SIZE;    // one bit reserved for 64 bit instructions
    UINT32 rd   : REG_SIZE;   // dest reg
    UINT32 rs   : REG_SIZE;   // source reg
} AM32_R_R;


typedef struct Am32_imm_ofs {
    UINT32 b32  : B_SIZE;     // small 32 bit inst if 0
    UINT32 mn   : MN_SIZE;    // one bit reserved for 64 bit instructions
    UINT32 imms : 12;
    UINT32 sofs : 12;
} AM32_IMM_OFS;


typedef struct Am64_r_r_r {
    UINT32 b32  : B_SIZE;     // must be 1
    UINT32 mn   : MN_SIZE;    // first half of opcode
    UINT32 mn2  : MN2_SIZE;   // second half of opcode
    UINT32 rd   : REG_SIZE;   // dest reg
    UINT32 dum1 : (32 - B_SIZE - MN_SIZE - MN2_SIZE - REG_SIZE);    // no fill
    // second 32 bit word
    UINT32 dum2 : (32 - REG_SIZE - REG_SIZE);                      // no fill
    UINT32 rs   : REG_SIZE;   // src1 reg
    UINT32 rt   : REG_SIZE;   // src2 reg
} AM64_R_R_R;


typedef struct Am64_r_ofs {
    UINT32 b32  : B_SIZE;     // must be 1
    UINT32 mn   : MN_SIZE;    // first half of opcode
    UINT32 mn2  : MN2_SIZE;   // second half of opcode
    UINT32 rd   : REG_SIZE;   // dest/src reg
    UINT32 dum1 : (32 - B_SIZE - MN_SIZE - MN2_SIZE - REG_SIZE); // no fill
    // second 32 bit word
    UINT32 lofs;        // full 32 bit offset
} AM64_R_OFS;


typedef struct Am64_r_r_ofs {
    UINT32 b32  : B_SIZE;     // must be 1
    UINT32 mn   : MN_SIZE;    // first half of opcode
    UINT32 mn2  : MN2_SIZE;   // second half of opcode
    UINT32 rd   : REG_SIZE;   // dest reg
    UINT32 dum1 : (32 - B_SIZE - MN_SIZE - MN2_SIZE - REG_SIZE);    // no fill
    // second 32 bit word
    UINT32 rs   : REG_SIZE;   // src reg
    UINT32 sofs : SOFS_SIZE;  // 20 bit offset
} AM64_R_R_OFS;


typedef UINT16 OPC_OFST;

// address mode, union of all kind of am, used in uwasm inst
// an inst can have only one kind of am
//
typedef struct am {
  union {
      AM32          am32;
      AM32_R        am32_r;
      AM32_OFS      am32_ofs;
      AM32_R_OFS    am32_r_ofs;
      AM32_R_R      am32_r_r;
      AM32_IMM_OFS  am32_imm_ofs;
      AM64_R_OFS    am64_r_ofs;
      AM64_R_R_OFS  am64_r_r_ofs;
      AM64_R_R_R    am64_r_r_r;
  };
} AM;

// uWASM instruction table
//
class UWASM_INST {
  friend UWASM_INST Scan_inst(IO_CBUFFER *pc, FILE* file);
private:
  const char   *_name;          // name of the instruction
  OPC_OFST      _opc_ofst;      // _offset to the implementation routine in the VM.
  UINT16        _uwasm_op;      // the according wasm op code.
  UINT16        _wasm_op;       // the wasm opcode for 1 to 1 translation
  AM            _am;            // arrangement of address mode
  TY_AM         _ty_am;         // type
  UINT32        _pc;            // PC for display
public:
  const char   *Name()                  const       { return _name;     }
  OPC_OFST      Opc_ofst()              const       { return _opc_ofst; }
  UINT16        Encode()                const       { return _uwasm_op; }
  void          Set_ofst(OPC_OFST ofst)             { _opc_ofst = ofst; }
  TY_AM         Ty_am()                 const       { return _ty_am;    }
  AM*           Am()                                { return &_am;      }
  UINT16        Wasm_op()               const       { return _wasm_op;  }
  void          Set_wasm_op(UINT16 w)               { _wasm_op = w;     }
  void          Set_pc (UINT32 pc)                  { _pc = pc;         };
  UINT16        Rd();
  UINT16        Rs();
  UINT16        Rt();
  UINT32        Ofs();
  UINT32        Imm();
  void          Set_rd(UINT16 r);
  void          Set_rs(UINT16 r);
  void          Set_rt(UINT16 r);
  void          Set_ofs(UINT32 ofs);
  void          Set_imm(UINT32 imm);
  void          Set_am(INT64 rd=-1, INT64 rs=-1,
                       INT64 rt=-1, INT64 ofs=-1, INT64 imm=-1);
  UWASM_INST(const char *name, UINT16 wasm_op, UINT16 uwasm_op, TY_AM ty_am);
  void Print(FILE* file = stdout);
  INT32 Emit_binary(FILE *file);
  void Rand_operand();

private:
  UWASM_INST() {} // only to be used by friend function.
};


// UWASM instruction info
//
extern UWASM_INST UWASM_META_INFO[];


UWASM_INST *UOPCODE_inst(UINT16 opcode);
UWASM_INST *UOPCODE_from_wasm(UINT16 opcode);
UWASM_INST Scan_inst(IO_CBUFFER *pc, FILE* file=stdout);


#endif //WASM2UWASM_UWINST_H
