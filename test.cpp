#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include "winst.h"
#include "uwinst.h"
#include "basics.h"
#include "wasm_config.h"
#include "reader.h"
#include "libubh.h"
#include "convertor.h"

using std::vector;


INT32 Read_magic_number(IO_CBUFFER pc) {
  Is_True(memcmp(pc, "\0asm\1\0\0\0\0", 8) == 0,
          ("Incorrect magic number"));
  return 0;
}


INT32 Read_all_sections(UINT8 *buffer, UINT32 length, FILE_CONTEXT& ctx) {
  IO_CBUFFER pc  = buffer;
  IO_CBUFFER end = pc + length;
  if (ctx.Is_full_file()) {
    Read_magic_number(buffer);
    for(;pc < end;) {
      UINT8 section_id = Read_uint8(buffer);
      switch (section_id) {
        case 0x01: {
          pc++;
        }
      }
    }
  }
  // Treat the whole buffer as instruction buffer
  Read_insts(pc, length, ctx);
  return 0;
}


void Read_opr_from_wasm_inst(IO_CBUFFER pc, WASM_INST &inst) {
  Is_True(inst.Numopr() < 3, ("Numopr should be less than 3."));
  UINT16 inst_size = inst.Read_inst_size((IO_CBUFFER) pc);
  inst.Set_pc((UINT64) pc);
  // pc += 1;
}


INT32 Read_insts(IO_CBUFFER inst_buf, UINT32 buf_len, FILE_CONTEXT &ctx) {
  vector<WASM_INST> wasm_insts;
  IO_CBUFFER pc      = inst_buf;
  IO_CBUFFER end_buf = inst_buf + buf_len;
  for (; pc < end_buf;) {
    if (ctx.Kind() == FILE_UWASM) {
      Scan_inst(&pc);
    } else {
      UINT8 opcode = Read_uint8(pc);
      WASM_INST wasm_inst = *(OPCODE_inst(opcode));
      Read_opr_from_wasm_inst(pc, wasm_inst);
      wasm_inst.Print(pc);
      pc += wasm_inst.Read_inst_size(pc);
      wasm_insts.push_back(wasm_inst);
   }
  }
  if (ctx.Kind() == FILE_WASM) {
    Wasm_to_uwasm(wasm_insts, inst_buf, buf_len, ctx);
  }
  return 0;
}


#ifdef DEBUG

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stdout, "usage: %s <filename>\n", argv[0]);
    exit(1);
  }
  FILE_CONTEXT ctx;
  char *pgm = new char[2048]; /* source code file name */
  strcpy(pgm,argv[1]) ;
  FILE* wasmin = fopen(pgm, "r");
  if (wasmin == NULL) {
    Is_True(false, ("File %s cannot be opened", pgm))
    return 1;
  }
  FILE* listing = stdout; /* send listing to screen */
  fprintf(listing,"\nWASM file: %s\n",pgm);
  UINT8 *buffer = new UINT8[1024];
  UINT32 length = fread(buffer, 1, 1024, wasmin);
  Read_all_sections(buffer, length, ctx);
  delete[] buffer;
  fclose(wasmin);
  return 0;
}

#endif