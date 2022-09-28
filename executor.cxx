//
// Created by xc5 on 2020/7/28.
//
#include <vector>
#include "executor.h"
#include "uwinst.h"


template<>
UINT32 UWASM_FRAME::Get_reg<RTI32>(UINT16 reg_id) {
  return _i32regs[reg_id];
}

template<>
UINT32 UWASM_FRAME::Get_reg<RTI64>(UINT16 reg_id) {
  return _i64regs[reg_id];
}

template<>
void UWASM_FRAME::Set_reg<RTI32, INT32>(UINT16 reg_id, INT32 val) {
  _i32regs[reg_id] = val;
}


template<>
void UWASM_FRAME::Set_reg<RTI64, INT64>(UINT16 reg_id, INT64 val) {
  _i64regs[reg_id] = val;
}


template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_add(UWASM_FRAME *frame, UWASM_INST *it) {
  frame->Set_reg<K, REG_TYPE>(it->Rd(),
                              frame->Get_reg<K>(it->Rs()) +
                              frame->Get_reg<K>(it->Rt()));
  return 0;
}


template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_const(UWASM_FRAME *frame, UWASM_INST *it) {
  frame->Set_reg<K, REG_TYPE>(it->Rd(),
                              it->Ofs());
  return 0;
}


template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_ret(UWASM_FRAME *frame, UWASM_INST *it) {
  frame->Set_reg<K, REG_TYPE>(REGISTER_return,
                              frame->Get_reg<K>(it->Rd()));
  return 0;
}


template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_load(UWASM_FRAME *frame, UWASM_INST *it) {
  UINT32 base = frame->Get_reg<K>(it->Rs());
  UINT32 memr = *(UINT32 *) &(frame->Get_memory()->Get_data()[base + it->Ofs()]);
  frame->Set_reg<K, REG_TYPE>(it->Rd(), memr);
  return 0;
}

template<REG_TY_KIND K, typename REG_TYPE>
INT32 Execute_store(UWASM_FRAME *frame, UWASM_INST *it) {
  UINT32 base = frame->Get_reg<K>(it->Rs());
  UINT32 *memr = (UINT32 *) &(frame->Get_memory()->Get_data()[base + it->Ofs()]);
  *memr = frame->Get_reg<K>(it->Rd());
  return 0;
}



INT32 Execute_function_body(UWASM_FRAME *frame, INSTVEC *vec) {
  // Assuming the frame is properly set up already.
  Is_Trace(Tracing(TP_EXEC), (TFile, "%sStarting to execute the instructions\n%s",
                              DBAR, DBAR));
  UINT32 ins_executed = 0;
  if (Tracing(TP_EXEC)) {
    frame->Print(TFile);
  }
  for (INSTVEC::iterator it = vec->begin(); it != vec->end(); it++) {
    switch (it->Encode()) {
      case UI32ADD: {
        Execute_add<RTI32, INT32>(frame, &(*it));
        break;
      }
      case UI64ADD: {
        Execute_add<RTI64, INT64>(frame, &(*it));
        break;
      }
      case URETURN: {
        // return
        Execute_ret<RTI32, INT32>(frame, &(*it));
        break;
      }
      case UI32LOAD: {
        Execute_load<RTI32, INT32>(frame, &(*it));
        break;
      }
      case UI32STORE: {
        Execute_store<RTI32, INT32>(frame, &(*it));
        break;
      }
      case UI32CONST: {
        Execute_const<RTI32, INT32>(frame, &(*it));
        break;
      }
      default: {
        AssertThat(false, ("Opcode not implemented 0x%04x", it->Encode()));
      }
    }
    ins_executed ++;
  }
  if (Tracing(TP_EXEC)) {
    Is_Trace(Tracing(TP_EXEC),
             (TFile, "VM executed %d instructions\n", ins_executed));
    frame->Print(TFile);
  }
  return 0;
}

void UWASM_FRAME::Print(FILE *file) {
  // Print the contents in the frame.
  fprintf(file, "------ CONTENTS in FRAME --------\n"
                " Total frame size: i32 = %u, i64 = %u, f32 = %u, f64 = %u \n"
                "[i32 first ten register] : \n",
                _i32sz, _i64sz, _f32sz, _f64sz);
  for (UINT32 i = 0; i < 20; i++) {
    fprintf(file, "R%03x = 0x%08x\n", i, Get_reg<RTI32>(i));
  }
  _memory->Print(file);
  fprintf(file, "%s", "\n----------- END FRAME -----------\n");
}

void UWASM_MEMORY::Print(FILE *file) {
  UINT32 total = 256;
  UINT32 line_width = 32;
  fprintf(file, "%s Printing Memory Data %u(0x%08x) bytes \n%s", DBAR, total, total, DBAR);
  for (UINT32 base = 0; base < total; base += line_width) {
    fprintf(file, "0x%08x : ", base);
    for (UINT32 colm = 0; colm < line_width; colm ++) {
      fprintf(file, "%02x ", Get_data()[base + colm]);
    }
    fprintf(file, "\n");
  }
}
