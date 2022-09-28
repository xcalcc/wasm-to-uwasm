#ifndef WASM2UWASM_CONVERTOR_H
#define WASM2UWASM_CONVERTOR_H

#include "uwinst.h"
#include "winst.h"
#include "reader.h"
#include "uwasm_bin.h"
#include "basics.h"
#include "libubh.h"
#include <vector>
#include <stack>
#include <list>
#include <unordered_map>

using namespace std;


// wasm stack track
//
class WASM_STACK_ELEM {
private:
  REG_TY      _kind;        // type: local, global, temp
  REG_IDX      _idx;       // reg id, maximum 4092
public:
  WASM_STACK_ELEM(REG_TY kind, REG_IDX idx) : _kind(kind), _idx(idx) {};
  REG_TY  Get_kind()  const   { return _kind;  }
  REG_IDX Get_idx()   const   { return _idx;   }
};

typedef vector<WASM_INST>::iterator WINSITER;
typedef std::list<UINT32>           UINTLIST;
typedef stack<WASM_STACK_ELEM>      WASMSTK;
typedef vector<WASM_INST>           WINSVEC;

enum BLK_KIND {
  BLK_NONE    = 0, // used for dummy placement at the beginning of the vec
  BLK_IF      = 1,
  BLK_BLOCK   = 2,
  BLK_ELSE    = 3,
  BLK_LOOP    = 4,
  BLK_FUNC    = 5,
};


// wasm code block
//
class WINST_BLK {
private:
  BLK_KIND  _kind;
  UINT32    _parent;
  UINTLIST  _child;
  UINT32    _start_pc;        // the pc to the 'block' or 'loop' instruction
  UINT32    _end_instr_pc;    // the pc to the 'end' instruction
  UINT32    _after_block_pc;  // the pc of first instr. after this block.
  UINT32    _if_blk_id;       // only useful for else block,
  // marking its matching if-block id.
  UINT32    _else_blk_id;     // matching else block id, only for if-block
public:
  WINST_BLK(UINT32 cur_pc, BLK_KIND kind, UINT32 parent):
    _kind(kind),
    _parent(parent),
    _child(),
    _start_pc(cur_pc),
    _end_instr_pc(0),
    _after_block_pc(0),
    _if_blk_id(0),
    _else_blk_id(0)  {}
  UINT32      Kind()             { return _kind;               }
  UINT32      Get_end_pc()       { return _end_instr_pc;       }
  UINTLIST   &Get_child()        { return _child;              }
  UINT32      Get_start_pc()     { return _start_pc;           }
  void        Set_end_pc(UINT32 end_instr_pc) { _end_instr_pc = end_instr_pc; }
  UINT32      Get_parent()       { return _parent;             }
};


typedef vector<WINST_BLK>           BLK_VEC;
typedef vector<WINST_BLK>::iterator BLK_ITER;


class LABEL_TAB {
private:
    vector<LABEL_ELEM> _labels;
public:
    LABEL_IDX Add(UINT32 pc, UINT32 sym) {
      LABEL_IDX cur = _labels.size();
      _labels.push_back(LABEL_ELEM(pc, sym));
      return cur;
    };
    LABEL_ELEM &Get(LABEL_IDX idx) {
      Is_True(_labels.size() > idx, ("labels table does not contain idx = %d, out of range.", idx));
      return _labels[idx];
    };

    UINT32 Size() { return _labels.size(); }

    void Print(FILE *file = stdout) {
      fprintf(file, "======== Printing the label table =========\n");
      fprintf(file, " Label count = %lu \n", _labels.size());
      for (UINT32 i = 0; i < _labels.size(); i++) {
        LABEL_ELEM &elem = _labels[i];
        fprintf(file, " // Label ID : 0x%08x \t", i);
        elem.Print(file);
      }
      fprintf(file, "======== End of the label table =========\n");
    }
};

U_MODULE Wasm_to_uwasm(WINSVEC &instr, IO_CBUFFER inst_buf, UINT32 len, FILE_CONTEXT &ctx);


class CONVERT_OPTIONS {
  const char *_output_name;
  bool       _conv_reading;
public:
  void Set_conv_in_read(bool v) {
    _conv_reading = v;
  }

  bool Is_conv_in_read() {
    return _conv_reading;
  }

  void Set_output_name(const char *n) {
    _output_name = n;
  }

  const char *Get_output_name() {
    return _output_name;
  }

  CONVERT_OPTIONS() {
    _output_name  = "a.uwasm";
    _conv_reading = true;
  }
};

CONVERT_OPTIONS &Get_conv_opt();

UINT32 Create_new_block(stack<UINT32> &work, BLK_VEC &blk_vec, UINT32 pc, BLK_KIND kind);

WINST_BLK * Get_blk_in_hier(UINT32 blk_id, BLK_VEC &blk_hierarchy);

UINT32 Finish_old_block(stack<UINT32> &stack, BLK_VEC &blks, UINT32 pc);

#endif //WASM2UWASM_CONVERTOR_H
