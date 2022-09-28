#include <cstdio>
#include "basics.h"
#include "winst.h"
#include "reader.h"

#define WASM_OP(tr, st1, st2, st3, im1, im2, cls, numopr, ret_size, g, h, i, j) {i, g, im1, im2, cls, numopr},
#define ___ VOID
#define NIL VOID

WASM_INST WASM_META_INFO [] = {
#include "winst_opcode.h"
};


WASM_INST *OPCODE_inst(UINT8 opcode) {
  const int INST_TABLE_SIZE = sizeof(WASM_META_INFO) / sizeof(WASM_INST);
  for (UINT32 i = 0; i < INST_TABLE_SIZE; i++) {
    if (WASM_META_INFO[i].Encode() == opcode) {
      return &WASM_META_INFO[i];
    }
  }
  Is_True(false, ("Instruction not in the table : %d, 0x%02x", (INT32) opcode, (INT32) opcode));
  return nullptr;
}


UINT8 WASM_INST::Print_opr (FILE *file, OPTYPE_SZ opr_size, IO_CBUFFER pc) {
  switch(opr_size) {
    case I32:
    case F32: {
      UINT32 opr1     = Read_leb128(pc);
      UINT8  leb_size = Read_opr_size(pc);
      fprintf(file, " 0x%08x", opr1);
      return leb_size;
    }
    case I64:
    case F64: {
      UINT64 opr1     = Read_leb128(pc);
      UINT8  leb_size = Read_opr_size(pc);
      fprintf(file, " 0x%016llx", opr1);
      return leb_size;
    }
    case VOID: return 0;
    default:
      fprintf(file, " unknown type opr");
      return opr_size;
  }
}

UINT64 WASM_INST::Read_opr (UINT8 opr_size, IO_CBUFFER pc) {
  switch(opr_size) {
    case 4: {
      UINT32 opr1     = Read_leb128(pc);
      return opr1;
    }
    case 8: {
      UINT64 opr1     = Read_leb128(pc);
      return opr1;
    }
    case 0: return 0;
    default:
      return 0;
  }
}


INT32 WASM_INST::Print(IO_CBUFFER pc, FILE *file) {
  Is_True(Numopr() < 3, ("Numopr should be less than 3."));

  fprintf(file, "\t0x%018llx\t%02x", (UINT64) pc, Read_uint8(pc));
  UINT16 inst_size = Read_inst_size(pc);
  pc += 1;
  for (UINT32 i = 0 ; i < 9 || i < inst_size - 1; i++) {
    if (i < inst_size - 1) {
      fprintf(file, " %02x", pc[i]);
    } else {
      fprintf(file, "%-3s", "");
    }
  }
  int n = 0;
  fprintf(file, "\t// %s", _name);
  if (Numopr() > 0) {
    n += Print_opr(file, OpSz1(), pc);
    pc += n;
  }
  if (Numopr() > 1) {
    fprintf(file, ",");
    n += Print_opr(file, OpSz2(), pc);
  }
  fprintf(file, "\n");
  return n;
}


// emitting binary form of WASM to file.
//
INT32 WASM_INST::Emit_binary(IO_CBUFFER pc, FILE *file) {
  Is_True(Numopr() < 3, ("Numopr should be less than 3."));
  pc += 1;
  for (UINT32 i = 0 ; i < OpSz1() + OpSz2(); i++) {
    fprintf(file, "%c", pc[i]);
  }
  int n = 0;
  if (Numopr() > 0) {
    n += Emit_opr(file, OpSz1(), pc);
    pc += n;
  }
  if (Numopr() > 1) {
    n += Emit_opr(file, OpSz2(), pc);
  }
  return n;
}


// emitting operand
// 
UINT8 WASM_INST::Emit_opr (FILE *file, UINT8 opr_size, IO_CBUFFER pc) {
  UINT8 opr_real_bytes = 0;
  switch(opr_size) {
    case 4: {
      UINT32 opr1 = Read_leb128(pc);
      opr_real_bytes = Read_opr_size(pc);
      fwrite(&opr1, 1, opr_real_bytes, file);
      return 4;
    }
    case 8: {
      UINT64 opr1 = Read_leb128(pc);
      opr_real_bytes = Read_opr_size(pc);
      fwrite(&opr1, 1, opr_real_bytes, file);
      return 8;
    }
    case 0: return 0;
    default:
      return opr_size;
  }
}

UINT16 WASM_INST::Read_inst_size(IO_CBUFFER pc) {
  pc += 1;
  UINT32 n = 0;
  if (Numopr() > 0) {
    n += Read_opr_size(pc);
    pc += n;
  }
  if (Numopr() > 1) {
    n += Read_opr_size(pc);
  }
  return n + 1;
}

INT32 WASM_INST::Print(IO_CBUFFER pc, UINT32 ins_cnt, FILE *file) {
  fprintf(file, "\t0x%06x\t", ins_cnt);
  return Print(pc, file);
}


INT32 WASM_INST::Print(FILE *file) {
  return Print((IO_CBUFFER) this->Pc(), file);
}

INT32 WASM_INST::Print(UINT32 ins_cnt, FILE *file) {
  fprintf(file, "\t0x%06x\t", ins_cnt);
  return Print(file);
}
