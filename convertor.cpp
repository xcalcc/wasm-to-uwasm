#include "convertor.h"
#include "winst.h"
#include "uwinst.h"
#include "basics.h"
#include "reader.h"
#include "op_enum.h"
#include "uwasm_bin.h"
#include "executor.h"
#include "libubh.h"
#include <iterator>

using namespace std;

CONVERT_OPTIONS &Get_conv_opt() {
  static CONVERT_OPTIONS conv_options;
  return conv_options;
}

void Print_winst_vec(WINSVEC *ins, FILE * file=stdout) {
  fprintf(file, "===== Printing wasm instruction vector =====\n");
  UINT32 ins_cnt = BLOCK_INSTR_BEGIN;
  for (UINT32 i = 0 ; i < ins->size(); i++, ins_cnt++) {
    WASM_INST &insp = (*ins)[i];
    insp.Print((IO_BUFFER) insp.Pc(), ins_cnt, file);
  }
  fprintf(file, "===== End of wasm instruction vector =====\n");
}


LABEL_IDX Get_label_index_for_branch(UINT32 ins_cnt, UINT32 current,
                                     BLK_VEC &blk_vec, UINT32 br_cnt,
                                     LABEL_TAB &labels) {
  if (current == LABEL_INVALID || current == LABEL_NOT_NEEDED) {
    return current;
  }
  // Loop over this level of blocks
  WINST_BLK *blk = Get_blk_in_hier(current, blk_vec); // function
  Is_True(blk->Get_start_pc() > 0 && blk->Get_end_pc() > 0, ("block = %d is not complete.", current));
  if (ins_cnt < blk->Get_start_pc() || ins_cnt > blk->Get_end_pc()) {
    // No viable targets in this block or its child.
    return LABEL_INVALID;
  }
  if (ins_cnt == blk->Get_start_pc() && blk->Kind() != BLK_FUNC) {
    // In case of 'if' or 'else' instruction's start
    if (blk->Kind() == BLK_IF) {
      // In case of an 'if' instr, we should jump to else block
      // Add a label to jump to the else branch.
      LABEL_IDX label = labels.Add(blk->Get_end_pc() + 1, current);
      Is_Trace(Tracing(TP_BLK_GET),
               (TFile, "Adding jump for if instr, target = %d, "
                       " label_id = %d\n",
                 blk->Get_end_pc() + 1, label));
      return label;
    } else if (blk->Kind() == BLK_LOOP) {
      Is_Trace(Tracing(TP_BLK_GET), (TFile, "Loop not needing a branch\n"));
      return LABEL_NOT_NEEDED;
    } else {
      // In case of an 'else' instr
      Is_True(blk->Kind() == BLK_ELSE,
              ("Not an else block, blk = %d", blk->Kind()));
      Is_Trace(Tracing(TP_BLK_GET),
               (TFile, "Adding jump for else instr, target = %d\n",
                 blk->Get_end_pc() + 1));
      LABEL_IDX label = labels.Add(blk->Get_end_pc() + 1, current);
      return label;
    }
  }
  if (ins_cnt == blk->Get_end_pc()) {
    // In case of 'if' or 'else' instruction's ending
    if (blk->Kind() == BLK_IF || blk->Kind() == BLK_ELSE || blk->Kind() == BLK_BLOCK ||
        blk->Kind() == BLK_FUNC || blk->Kind() == BLK_LOOP) {
      Is_Trace(Tracing(TP_BLK_GET),
               (TFile, "No jump needed for end of block for "
                       "if/block/else/func/loop.\n"));
      // For 'if', we do it at the beginning of the else branch.
      return LABEL_NOT_NEEDED;
      return LABEL_NOT_NEEDED;
    } else {
      Is_True(false, ("Probably a end-of-function or something else."));
    }
  }
  Is_True (ins_cnt > blk->Get_start_pc() && ins_cnt < blk->Get_end_pc() ||
           blk->Get_start_pc() == ins_cnt && blk->Kind() == BLK_FUNC,
           ("Condition implied not met."));

  UINTLIST                &children = blk->Get_child();
  for (UINTLIST::iterator it        = children.begin();
       it != children.end(); it++) {
    UINT32    child_blk       = *it;
    LABEL_IDX child_found_idx = LABEL_INVALID;
    child_found_idx = Get_label_index_for_branch(ins_cnt, child_blk, blk_vec,
                                                 br_cnt, labels);
    if (child_found_idx != LABEL_INVALID) {
      return child_found_idx;
    }
  }

  WINST_BLK *cur_blk = blk;
  if (br_cnt > 0) {
    // Goto the parental or other block levels.
    for (UINT32 i = 0; i < br_cnt; i++) {
      UINT32 cur_parent = cur_blk->Get_parent();
      Is_True(cur_parent != 0, ("Incorrect branch cnt found = %d", br_cnt));
      cur_blk = Get_blk_in_hier(cur_parent, blk_vec);
    }
  }

  // No child applicable, this means a branch out of current block/loop
  if (cur_blk->Kind() == BLK_LOOP) {
    // Jump to begin / continue;
    Is_Trace(Tracing(TP_BLK_GET),
             (TFile, "For loops we need a jump to begin, pc = %d \n",
               cur_blk->Get_start_pc() + 1));
    LABEL_IDX label = labels.Add(cur_blk->Get_start_pc() + 1, current);
    return label;
  }
  // Else, this is jumping out of a block / if / else.
  LABEL_IDX label = labels.Add(cur_blk->Get_end_pc() + 1, current);
  Is_Trace(Tracing(TP_BLK_GET),
           (TFile, "Generating label = %d, target = %d, for "
                   "break out of current block = %d\n", label,
                    cur_blk->Get_end_pc() + 1, current));
  return label;
}

// build block hierarchy by loop against loop and blocks
//
BLK_VEC *Build_block_hierarchy(WINSVEC &ins, LABEL_TAB &labels) {
  BLK_VEC *blk = new BLK_VEC();
  BLK_VEC &blocks = *(blk);
  stack<UINT32> work;
  UINT32 ins_cnt = BLOCK_INSTR_BEGIN;
  if (Tracing(TP_BLK_WINS)) {
    Print_winst_vec(&ins, TFile);
  }
  // Push a placeholder, to let all block id > 0.
  blocks.push_back(WINST_BLK(0, BLK_NONE, 0));
  labels.Add(0, 0); // placeholder as well.
  // Push a function block, to let all the next level blocks belong to it.
  Create_new_block(work, blocks, ins_cnt, BLK_FUNC);
  Is_Trace(Tracing(TP_BLK_BUILD),
           (TFile, "====== Starting to build block hierarchy ======\n"));
  for (WINSITER it = ins.begin(); it != ins.end(); it++) {
    WASM_INST &inst = *it;
    if (inst.Encode() == If) {
      UINT32 blk_id = Create_new_block(work, blocks, ins_cnt, BLK_IF);
      Is_Trace(Tracing(TP_BLK_BUILD),
               (TFile, "Found a if id = %d, pc = 0x%08x\n", blk_id, ins_cnt));
    } else if (inst.Encode() == Block) {
      // push
      UINT32 blk_id = Create_new_block(work, blocks, ins_cnt, BLK_BLOCK);
      Is_Trace(Tracing(TP_BLK_BUILD),
               (TFile, "Found a block id = 0x%08x, pc = 0x%08x\n", blk_id, ins_cnt));
    } else if (inst.Encode() == Loop) {
      // push
      UINT32 blk_id = Create_new_block(work, blocks, ins_cnt, BLK_LOOP);
      Is_Trace(Tracing(TP_BLK_BUILD),
               (TFile, "Found a loop id = 0x%08x, pc = 0x%08x\n", blk_id, ins_cnt));
    } else if (inst.Encode() == Else) {
      // Pop_wasm_stk the if block
      Is_True(!work.empty(), ("Work list cannot be empty."));
      UINT32 old_block_id = Finish_old_block(work, blocks, ins_cnt);
      UINT32 else_id = Create_new_block(work, blocks, ins_cnt, BLK_ELSE);
      Is_True(Get_blk_in_hier(old_block_id, blocks)->Kind() == BLK_IF,
              ("For else-block, the top of stack should be"
               " a if-block, but it is %d ",
                Get_blk_in_hier(old_block_id, blocks)->Kind()));
      Is_Trace(Tracing(TP_BLK_BUILD),
               (TFile, "Found a else = 0x%08x, if_block = 0x%08x, pc = 0x%08x\n",
                else_id, old_block_id, ins_cnt));
    } else if (inst.Encode() == End) {
      // pop and set the values.
      if (work.empty()) {
        Is_Trace(Tracing(TP_BLK_BUILD), (TFile, "End processed with emtpy work stack, error ?\n"));
        continue;
      }
      UINT32 blk_id = Finish_old_block(work, blocks, ins_cnt);
      Is_Trace(Tracing(TP_BLK_BUILD),
               (TFile, "End of a block-level = %d, top of stack = 0x%08x, pc = 0x%08x\n",
                Get_blk_in_hier(blk_id, blocks)->Kind(), blk_id, ins_cnt));
    }
    ins_cnt ++;
  }
  return blk;
}


UINT32 Finish_old_block(stack<UINT32> &work, BLK_VEC &blks, UINT32 pc) {
  Is_True(!work.empty(), ("The work list should not be empty."));
  UINT32 tos_blk = work.top(); // top of stack block.
  WINST_BLK *blk = Get_blk_in_hier(tos_blk, blks);
  Is_True(tos_blk != 0 && Get_blk_in_hier(tos_blk, blks)->Kind() != BLK_NONE,
          ("Top of stack block should be valid, yet block %d is not.", tos_blk));
  work.pop();
  blk->Set_end_pc(pc);
  return tos_blk;
}


UINT32 Create_new_block(stack<UINT32> &work, BLK_VEC &blk_vec, UINT32 ins_cnt,
                        BLK_KIND kind) {
  // Find the previous sibling in this K-tree.
  UINT32 parent       = 0;
  UINT32 blk_id = blk_vec.size();
  if (!work.empty()){
    // Find parent.
    parent = work.top();
    // Find prev_sibling
    UINTLIST &siblings = Get_blk_in_hier(work.top(), blk_vec)->Get_child();
    siblings.push_back(blk_id);
  }
  blk_vec.push_back(WINST_BLK(ins_cnt,  kind, parent));
  work.push(blk_id);
  return blk_id;
}


WINST_BLK * Get_blk_in_hier(UINT32 blk_id, BLK_VEC &blk_hierarchy) {
  Is_True(blk_hierarchy.size() > blk_id && blk_id > 0,
          ("Block id = %d is out of range (max = %d)",
            blk_id, blk_hierarchy.size()));
  return &(blk_hierarchy[blk_id]);
}


WASM_STACK_ELEM Pop_wasm_stk(stack<WASM_STACK_ELEM>& wasm_stack, bool de_pt= false, UINT8 *next= nullptr) {
  WASM_STACK_ELEM elem = wasm_stack.top();
  wasm_stack.pop();
  if (de_pt && elem.Get_kind() == TEMP) *next -= 1;
  return elem;
}


U_MODULE Wasm_to_uwasm(WINSVEC &instr, IO_CBUFFER inst_buf, UINT32 len, FILE_CONTEXT &ctx) {
  FILE                  *outfile = nullptr;
  U_MODULE               u_module;
  UBH_CODE               u_code(ctx.Get_func());
  WASMSTK                wasm_stack; // wasm stack we trace
  LABEL_TAB              label_tab;
  BLK_VEC               *vec = Build_block_hierarchy(instr, label_tab);
  UINT32                 ins_cnt = BLOCK_INSTR_BEGIN;
  Is_Trace(Tracing(TP_STACK), (TFile, "====== Stack-based pass to translate to uWASM ======\n"));
  for (WINSITER wasm_inst = instr.begin(); wasm_inst != instr.end(); wasm_inst++) {
    switch (wasm_inst->Encode()) {
      case Block:
      case Loop:
      case Br:
      case Else:
      case End: {
        // Unconditional jump
        UINT32 jump_cnt = 0;
        if (wasm_inst->Encode() == Br) {
          jump_cnt = Read_leb128((IO_BUFFER) wasm_inst->Pc() + 1);
        }
        LABEL_IDX target = Get_label_index_for_branch(ins_cnt, BLOCK_FUNCTION,
                                                      *vec,
                                                      jump_cnt, label_tab);
        if (target != LABEL_INVALID && target != LABEL_NOT_NEEDED) {
          UWASM_INST uwasm_inst =  *(UOPCODE_inst(UJMP));
          uwasm_inst.Set_pc(ins_cnt);
          uwasm_inst.Set_am(target);
          u_code.Get_insts()->push_back(uwasm_inst);
        }
        break;
      }
      case If:
      case BrIf:
      case BrTable:
      case BrOnExn: {
        // Expand of stack infos.
        Is_True(wasm_stack.size() > 0, ("No value on stack, maybe we got something wrong..."));
        WASM_STACK_ELEM rd = Pop_wasm_stk(wasm_stack);
        // Conditional jump target calculation.
        UINT32 br_cnt = 0;
        if (wasm_inst->Encode() == BrIf) {
          br_cnt = Read_leb128((IO_BUFFER) wasm_inst->Pc() + 1);
        }
        LABEL_IDX target = Get_label_index_for_branch(ins_cnt, BLOCK_FUNCTION,
                                                      *vec, br_cnt, label_tab);
        if (target != LABEL_INVALID && target != LABEL_NOT_NEEDED) {
          UWASM_INST uwasm_inst =  *(UOPCODE_inst(UJNZ));
          uwasm_inst.Set_pc(ins_cnt);
          uwasm_inst.Set_am(rd.Get_idx(), -1, -1, target);
          u_code.Get_insts()->push_back(uwasm_inst);
        }
        break;
      }
      case Drop: {
        Pop_wasm_stk(wasm_stack);
        break;
      }
      case Select:
      {
        // Instruction skipped.
        break;
      }
      case LocalGet: {
        IO_BUFFER pc = (IO_BUFFER)wasm_inst->Pc() + 1;
        WASM_STACK_ELEM r = WASM_STACK_ELEM(LOCAL, u_code.Get_local_map()->at(Read_leb128(pc)));
        wasm_stack.push(r);
        break;
      }
      case LocalSet:
      case LocalTee: {
        IO_BUFFER pc = (IO_BUFFER)wasm_inst->Pc() + 1;
        UINT32 local_idx = Read_leb128(pc);
        REG_IDX rd_idx = u_code.Get_local_map()->at(local_idx);
        WASM_STACK_ELEM rs = wasm_inst->Encode() == LocalSet ? Pop_wasm_stk(wasm_stack) : wasm_stack.top();
        UWASM_INST uwasm_inst = *(UOPCODE_inst(UMOV));
        uwasm_inst.Set_pc(ins_cnt);
        uwasm_inst.Set_am(rd_idx, rs.Get_idx());
        u_code.Get_insts()->push_back(uwasm_inst);
        break;
      }
      case I32Load: {
        UWASM_INST uwasm_inst = *(UOPCODE_inst(wasm_inst->Encode()));
        uwasm_inst.Set_pc(ins_cnt);
        WASM_STACK_ELEM rs = Pop_wasm_stk(wasm_stack);
        IO_BUFFER pc = (IO_BUFFER)wasm_inst->Pc() + 1 + Read_opr_size((IO_BUFFER)wasm_inst->Pc()+1);
        UINT32 rd = u_code.Push_temp_reg(I32, (UINT64)pc, wasm_stack);
        uwasm_inst.Set_am(rd, rs.Get_idx(), -1, Read_leb128(pc));
        u_code.Get_insts()->push_back(uwasm_inst);
        break;
      }
      case I32Store: {
        UWASM_INST uwasm_inst = *(UOPCODE_inst(wasm_inst->Encode()));
        uwasm_inst.Set_pc(ins_cnt);
        WASM_STACK_ELEM rd = Pop_wasm_stk(wasm_stack);
        WASM_STACK_ELEM rs = Pop_wasm_stk(wasm_stack);
        IO_BUFFER pc = (IO_BUFFER)wasm_inst->Pc() + 1 + Read_opr_size((IO_BUFFER)wasm_inst->Pc()+1);
        uwasm_inst.Set_am(rd.Get_idx(), rs.Get_idx(), Read_leb128(pc));
        u_code.Get_insts()->push_back(uwasm_inst);
        break;
      }
      case I32Const: {
        UWASM_INST uwasm_inst = *(UOPCODE_inst(wasm_inst->Encode()));
        uwasm_inst.Set_pc(ins_cnt);
        UINT32 rd = u_code.Push_temp_reg(I32, wasm_inst->Pc(), wasm_stack);
        uwasm_inst.Set_am(rd, -1, -1, Read_leb128((IO_BUFFER) wasm_inst->Pc() + 1));
        u_code.Get_insts()->push_back(uwasm_inst);
        break;
      }
      case I32Add:
      case I32Sub:
      case I32Mul:
      case I32DivS:
      case I32DivU:
      case I32LtS:
      case I32Ne: {
        UWASM_INST uwasm_inst = *(UOPCODE_inst(wasm_inst->Encode()));
        uwasm_inst.Set_pc(ins_cnt);
        Is_True(wasm_stack.size() >= 2, ("not enough reg\n"));
        WASM_STACK_ELEM rs = Pop_wasm_stk(wasm_stack);
        WASM_STACK_ELEM rt = Pop_wasm_stk(wasm_stack);
        UINT32 rd = u_code.Push_temp_reg(I32, wasm_inst->Pc(), wasm_stack);
        uwasm_inst.Set_am(rd, rs.Get_idx(), rt.Get_idx());
        u_code.Get_insts()->push_back(uwasm_inst);
        break;
      }
      case Call: {
        // for i in (j, j+n): mov wasm_stack.top to reg[i]
        break;
      }
      case Return: {
        WASM_STACK_ELEM rd = Pop_wasm_stk(wasm_stack);
        UWASM_INST uwasm_inst = *(UOPCODE_inst(URETURN));
        uwasm_inst.Set_pc(ins_cnt);
        uwasm_inst.Set_am(rd.Get_idx());
        u_code.Get_insts()->push_back(uwasm_inst);
      }
      default: {
        // Translate to UWASM, print the uWASM in binary form
        UWASM_INST uwasm_inst = *(UOPCODE_from_wasm(wasm_inst->Encode()));
        IO_BUFFER  pc         = (IO_BUFFER) wasm_inst->Pc();
        uwasm_inst.Set_pc(ins_cnt);
        uwasm_inst.Rand_operand();
        u_code.Get_insts()->push_back(uwasm_inst);
        break;
      }
    }
    ins_cnt ++;
  }
  UINT32 param_begin = u_code.Get_local_num(); // start reg idx of param
  /* TODO: change reg idx in CALL
  for each CALL:
   j = param_begin
   */
  if (ctx.Is_emit_binary()) outfile = fopen("output.inst", "w+");
  u_code.Print(ctx, outfile);
  Is_Trace(Tracing(TP_STACK), (TFile, "====== Finish translate to uWASM ======\n"));
  // Print LABEL TABLE
  label_tab.Print();
  // Try to execute the result;
  UWASM_FRAME *frame = new UWASM_FRAME();
  Execute_function_body(frame, u_code.Get_insts());
  delete frame;
  u_module.Set_code_sec(u_code);
  return u_module;
}
