#include <cstdio>
#include <string>
#include <cmath>
#include "uwinst.h"
#include "reader.h"
#include "wasm_config.h"

#define UWASM_OP(name, wasm_op, uwasm_op, ty_am, _enum) {name, wasm_op, uwasm_op, ty_am},
#define ___ NO_TYPE
#define NIL NO_TYPE

#define GET_MEM_VAL(val, n) (*(UINT8 *)(((char *) val) + n))

// UWASM instruction info
//
UWASM_INST UWASM_META_INFO[] = {
#include "uwinst_opcode.h"
};


UWASM_INST *UOPCODE_inst(UINT16 opcode) {
  const int INST_TABLE_SIZE = sizeof(UWASM_META_INFO) / sizeof(UWASM_INST);
  for (UINT32 i = 0; i < INST_TABLE_SIZE; i++) {
    if (UWASM_META_INFO[i].Encode() == opcode) {
      return &UWASM_META_INFO[i];
    }
  }
  Is_True(false, ("Instruction not in the table : dec: %d,  hex: 0x%02x",
                  (INT32) opcode, (INT32) opcode));
  return nullptr;
}

UWASM_INST *UOPCODE_from_wasm(UINT16 opcode) {
  const int INST_TABLE_SIZE = sizeof(UWASM_META_INFO) / sizeof(UWASM_INST);
  for (UINT32 i = 0; i < INST_TABLE_SIZE; i++) {
    if (UWASM_META_INFO[i].Wasm_op() == opcode) {
      return &UWASM_META_INFO[i];
    }
  }
  Is_True(false, ("Instruction not in the table : dec: %d,  hex: 0x%02x",
    (INT32) opcode, (INT32) opcode));
  return nullptr;
}


UWASM_INST::UWASM_INST(const char *name,     UINT16 wasm_op,
                       UINT16 uwasm_op,      TY_AM ty_am) :
                       _name(name),          _wasm_op(wasm_op),
                       _uwasm_op(uwasm_op),   _ty_am(ty_am)  {
  _opc_ofst = 0x0000;
  switch (ty_am) {
    case TY_AM32_R:
    case TY_AM32_R_R:
    case TY_AM32_R_OFS:
    case TY_AM32: {
//      Is_True((uwasm_op & 0x80) == 0, ("Incorrect non-zero starting bit."));
      Am()->am32_r.b32 = 0;
      Am()->am32_r.mn = uwasm_op;
      break;
    }
    default:
      Am()->am64_r_r_ofs.b32  = 1;
      Am()->am64_r_r_ofs.mn   = (uwasm_op >> 8u) & 0x7fu;
      Am()->am64_r_r_ofs.mn2  = uwasm_op & 0xffu;
  }
}


void UWASM_INST::Print(FILE* file) {
  fprintf(file, "%-22s $$ 0x%08x ", "", _pc);
  switch (Ty_am()) {
    case TY_AM32_R:
    case TY_AM32_R_R:
    case TY_AM32_R_OFS:
    case TY_AM32: {
      UINT32 *val = (UINT32 *)&(Am()->am32_r);
      fprintf(file, "%02x %02x %02x %02x %-12s",
              GET_MEM_VAL(val, 0),
              GET_MEM_VAL(val, 1),
              GET_MEM_VAL(val, 2),
              GET_MEM_VAL(val, 3), "");
      break;
    }
    default: {
      UINT64 *val = (UINT64 *) &(Am()->am64_r_r_ofs);
      fprintf(file, "%02x %02x %02x %02x %02x %02x %02x %02x",
              GET_MEM_VAL(val, 0),
              GET_MEM_VAL(val, 1),
              GET_MEM_VAL(val, 2),
              GET_MEM_VAL(val, 3),
              GET_MEM_VAL(val, 4),
              GET_MEM_VAL(val, 5),
              GET_MEM_VAL(val, 6),
              GET_MEM_VAL(val, 7));
      break;
    }
  }
  fprintf(file, "\t // %-16s", Name());
  switch (Ty_am()) {
    case TY_AM32:
      break;
    case TY_AM32_R:
      fprintf(file, " R%03x", Rd());
      break;
    case TY_AM32_R_OFS:
      fprintf(file, " R%03x (0x%03x)", Rd(), Ofs());
      break;
    case TY_AM32_R_R:
      fprintf(file, " R%03x R%03x", Rd(), Rs());
      break;
    case TY_AM64_R_R_R:
      fprintf(file, " R%03x R%03x R%03x", Rd(), Rs(), Rt());
      break;
    case TY_AM64_R_OFS:
      fprintf(file, " R%03x 0x%08x", Rd(), Ofs());
      break;
    case TY_AM64_R_R_OFS:
      fprintf(file, " R%03x R%03x (0x%05x)", Rd(), Rs(), Ofs());
      break;
    default:
      fprintf(file, "unknown am type");
  }
  fprintf(file, "\n");
}


INT32 UWASM_INST::Emit_binary(FILE* file) {
  // fprintf(file, "%c.......", (UINT16) this->Encode());
  switch (Ty_am()) {
    case TY_AM32_R:
    case TY_AM32_R_R:
    case TY_AM32_R_OFS:
    case TY_AM32: {
      fwrite(&(Am()->am32_r), 1, 4, file);
      break;
    }
    default: {
      fwrite(&(Am()->am64_r_r_ofs), 1, 8, file);
      break;
    }
  }
  return 8;
}

UWASM_INST Scan_inst(IO_CBUFFER *pc, FILE* file) {
  UWASM_INST uwasm_inst;
  AM32 *am_base = (AM32 *) *pc;
  if (am_base->b32 == 0) { // first bit 0
    // no instruction yet
    UINT16 opcode = am_base->mn;
    uwasm_inst = *(UOPCODE_inst(opcode));
    *((UINT32 *) uwasm_inst.Am()) = *(UINT32 *) am_base;
    *pc += 4;
  } else { // first bit 1
    AM64_R_R_R *am64_r_r_r = (AM64_R_R_R *) *pc;
    UINT16 opcode = (1 << 15) | (am64_r_r_r->mn << 14) | (am64_r_r_r->mn2);
    uwasm_inst = *(UOPCODE_inst(opcode));
    *((UINT64 *) uwasm_inst.Am()) = *(UINT64 *) am_base;
    *pc += 8;
  }
  if (Tracing(TP_PRINT)) {
    uwasm_inst.Print();
  }
  return uwasm_inst;
}

void UWASM_INST::Rand_operand() {
  switch (Ty_am()) {
    case TY_AM64_R_R_R: {
      Am()->am64_r_r_r.rd = 0x200;
      Am()->am64_r_r_r.rs = 0x001;
      Am()->am64_r_r_r.rt = 0x0F0;
      break;
    }
    case TY_AM64_R_OFS: {
      Am()->am64_r_ofs.rd   = 0x200;
      Am()->am64_r_ofs.lofs = 0xff010203;
      break;
    }
    case TY_AM32_R: {
        Am()->am32_r.rd   = 0x200;
    }
    default: {
      // Nothing to do
    }
  }
}

void UWASM_INST::Set_rd(UINT16 r) {
  switch (Ty_am()) {
    case TY_AM32_R:
    case TY_AM32_R_R:
    case TY_AM32_R_OFS:
      Am()->am32_r.rd = r;
      break;
    case TY_AM64_R_R_R:
    case TY_AM64_R_OFS:
    case TY_AM64_R_R_OFS:
      Am()->am64_r_r_r.rd = r;
      break;
    default:
      break;
  }
}

void UWASM_INST::Set_rs(UINT16 r) {
  switch (Ty_am()) {
    case TY_AM32_R_R:
      Am()->am32_r_r.rs = r;
      break;
    case TY_AM64_R_R_R:
      Am()->am64_r_r_r.rs = r;
      break;
    case TY_AM64_R_R_OFS:
      Am()->am64_r_r_ofs.rs = r;
      break;
    default:
      break;
  }
}

void UWASM_INST::Set_rt(UINT16 r) {
  switch (Ty_am()) {
    case TY_AM64_R_R_R:
      Am()->am64_r_r_r.rt = r;
      break;
    default:
      break;
  }
}

UINT16 UWASM_INST::Rd() {
  switch (Ty_am()) {
    case TY_AM32_R:
    case TY_AM32_R_R:
    case TY_AM32_R_OFS:
      return Am()->am32_r.rd;
    case TY_AM64_R_R_R:
    case TY_AM64_R_OFS:
    case TY_AM64_R_R_OFS:
      return Am()->am64_r_r_r.rd;
    default:
      Is_True(false, ("type has no rd"));
      return 4096;
  }
}

UINT16 UWASM_INST::Rs() {
  switch (Ty_am()) {
    case TY_AM32_R_R:
      return Am()->am32_r_r.rs;
    case TY_AM64_R_R_R:
      return Am()->am64_r_r_r.rs;
    case TY_AM64_R_R_OFS:
      return Am()->am64_r_r_ofs.rs;
    default:
      Is_True(false, ("type has no rs"));
      return 4096;
  }
}

UINT16 UWASM_INST::Rt() {
  switch (Ty_am()) {
    case TY_AM64_R_R_R:
      return Am()->am64_r_r_r.rt;
    default:
      Is_True(false, ("type has no rt"));
      return 4096;
  }
  return 0;
}

UINT32 UWASM_INST::Ofs() {
  switch (Ty_am()) {
    case TY_AM32_OFS:
      return Am()->am32_ofs.lofs;
    case TY_AM32_R_OFS:
      return Am()->am32_r_ofs.sofs;
    case TY_AM32_IMM_OFS:
      return Am()->am32_imm_ofs.sofs;
    case TY_AM64_R_OFS:
      return Am()->am64_r_ofs.lofs;
    case TY_AM64_R_R_OFS:
      return Am()->am64_r_r_ofs.sofs;
    default:
      Is_True(false, ("type has no ofs"));
      return 4096;
  }
}

void UWASM_INST::Set_ofs(UINT32 ofs) {
  switch (Ty_am()) {
    case TY_AM32_OFS:
      Am()->am32_ofs.lofs = ofs;
    case TY_AM32_R_OFS:
      Am()->am32_r_ofs.sofs = ofs;
      break;
    case TY_AM64_R_OFS:
      Am()->am64_r_ofs.lofs = ofs;
      break;
    case TY_AM64_R_R_OFS:
      Am()->am64_r_r_ofs.sofs = ofs;
      break;
    case TY_AM32_IMM_OFS:
      Am()->am32_imm_ofs.sofs = ofs;
    default:
      break;
  }
}

UINT32 UWASM_INST::Imm() {
  switch (Ty_am()) {
    case TY_AM32_IMM_OFS:
      return Am()->am32_imm_ofs.imms;
    default:
      Is_True(false, ("type has no imm"));
      return 4096;
  }
}


void UWASM_INST::Set_imm(UINT32 imm) {
  switch (Ty_am()) {
    case TY_AM32_IMM_OFS:
      Am()->am32_imm_ofs.imms = imm;
    default:
      Is_True(false, ("type has no imm"));
  }
}

void UWASM_INST::Set_am(INT64 rd, INT64 rs, INT64 rt, INT64 ofs, INT64 imm) {
  if (rd != -1)  Set_rd(rd);
  if (rs != -1)  Set_rs(rs);
  if (rt != -1)  Set_rt(rt);
  if (ofs != -1) Set_ofs(ofs);
  if (imm != -1) Set_imm(imm);
}
