#include "ubh.h"
#include "reader.h"
#include "convertor.h"

void UBH_CODE::Print(FILE_CONTEXT &ctx, FILE *outfile, FILE *file) {
  for (UWINSVEC::iterator it = Get_insts()->begin(); it != Get_insts()->end(); ++it) {
    it->Print(file);
    if (ctx.Is_emit_binary()) it->Emit_binary(outfile);
  }
}


UINT32 UBH_CODE::Get_local_num() {
  UINT32 local_num = CONST_REG_NUM + RET_REG_NUM;
  for (vector<LOCAL_TAB>::iterator it = Get_local_table()->begin(); it != Get_local_table()->end(); ++it) local_num += it->size();
  return local_num;
}


// read param and local from wasm
//
UBH_CODE::UBH_CODE(wabt::Func* func) {
  LOCAL_TAB i32_table;
  LOCAL_TAB i64_table;
  LOCAL_TAB f32_table;
  LOCAL_TAB f64_table;
  wabt::Index param_num = func->GetNumParams();
  wabt::Index i;
  for (i = 0; i < param_num; i++) {
    wabt::Type formal_param = func->GetParamType(i);
    switch (formal_param) {
      case wabt::Type::I32:
        i32_table.push_back(LOCAL_TAB_ELEM(FORMAL_PARAM, I32));
        Get_local_map()->push_back(i32_table.size() + RET_REG_NUM);
        break;
      case wabt::Type::I64:
        i64_table.push_back(LOCAL_TAB_ELEM(FORMAL_PARAM, I64));
        Get_local_map()->push_back(i64_table.size() + RET_REG_NUM);
        break;
      case wabt::Type::F32:
        f32_table.push_back(LOCAL_TAB_ELEM(FORMAL_PARAM, F32));
        Get_local_map()->push_back(f32_table.size() + RET_REG_NUM);
        break;
      case wabt::Type::F64:
        f64_table.push_back(LOCAL_TAB_ELEM(FORMAL_PARAM, F64));
        Get_local_map()->push_back(f64_table.size() + RET_REG_NUM);
        break;
      default:
        Is_True(false, ("unknown param type"));
        break;
    }
  }
  wabt::Index local_num = func->GetNumLocals();
  for (i = 0; i < local_num; i++) {
    wabt::Type local = func->GetLocalType(i);
    switch (local) {
      case wabt::Type::I32:
        i32_table.push_back(LOCAL_TAB_ELEM(LOCAL, I32));
        Get_local_map()->push_back(i32_table.size() + RET_REG_NUM);
        break;
      case wabt::Type::I64:
        i64_table.push_back(LOCAL_TAB_ELEM(LOCAL, I64));
        Get_local_map()->push_back(i64_table.size() + RET_REG_NUM);
        break;
      case wabt::Type::F32:
        f32_table.push_back(LOCAL_TAB_ELEM(LOCAL, F32));
        Get_local_map()->push_back(f32_table.size() + RET_REG_NUM);
        break;
      case wabt::Type::F64:
        f64_table.push_back(LOCAL_TAB_ELEM(LOCAL, F64));
        f64_table.push_back(LOCAL_TAB_ELEM(FORMAL_PARAM, F64));
        break;
      default:
        Is_True(false, ("unknown param type"));
        break;
    }
  }
  Get_local_table()->push_back(i32_table);
  Get_local_table()->push_back(i64_table);
  Get_local_table()->push_back(f32_table);
  Get_local_table()->push_back(f64_table);
}


REG_IDX UBH_CODE::Push_temp_reg(OPTYPE_SZ _type, UINT64 pc, WASMSTK& wasm_stack) {
  REG_IDX next = 0;
  UINT8 local_table_idx;
  switch (_type) {
    case I32:
      local_table_idx = 0;
      break;
    case I64:
      local_table_idx = 1;
      break;
    case F32:
      local_table_idx = 2;
      break;
    case F64:
      local_table_idx = 3;
      break;
    default:
      Is_True(false, ("unknown type"));
      return 0xfff;
  }
  next = Get_local_table()->at(local_table_idx).size() + CONST_REG_NUM + RET_REG_NUM;
  Get_local_table()->at(local_table_idx).push_back(LOCAL_TAB_ELEM(TEMP, _type, pc));
  wasm_stack.push(WASM_STACK_ELEM(TEMP, next));
  return next;
}