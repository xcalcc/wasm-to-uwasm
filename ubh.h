

#ifndef _UBH_H
#define _UBH_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <stack>
#include "basics.h"
#include "uwinst.h"
#include "uwasm_bin.h"
#include "reader.h"

using namespace std;

#define UBHMAGIC "uASM"
#define UC_MAG0  'u'
#define   UIDX_MAG0 0
#define UC_MAG1  'A'
#define   UIDX_MAG1 1
#define UC_MAG2  'S'
#define   UIDX_MAG2 2
#define UC_MAG3  'M'
#define   UIDX_MAG3 3

// we assume the format is a mixture of 32 and 64 bits mixture of quantities
typedef uint32_t ubh_uint;
typedef int32_t  ubh_int;
typedef uint64_t ubh_ulong;   // assume long is 64 internally
typedef int64_t  ubh_long;
typedef uint16_t ubh_ushort;  // only used in binary header description
typedef uint8_t  ubh_uchar;   // only used in binary header description

typedef uint16_t ubh_ofs;     // offset is always 32 bit signed
typedef ubh_ofs  ubh_idx16;   // should never be used directly
                              // shift properly before use, max size is 2**16  

typedef uint32_t ubh_addr;    // assume 32 bit address space always

typedef uint16_t ubh_idx;     // indices for various section, always 16 bits


#define UBH_MAGIC (4)         // magic at starttt of uWASM binary file
#define UBH_OTHER (28)        // other pertinent info

class UBH_MHDR {
private:
  unsigned char uMagic[UBH_MAGIC];
  unsigned char uOther[UBH_OTHER];  // this include validation info etc
  ubh_ushort    shentsz;            // section hdr table entry size
  ubh_ushort    shnum;              // section hdr table entry count
public:
  const unsigned char*  Umagic(void)   const { return uMagic;  }
  const unsigned char*  Uother(void)   const { return uOther;  }
  ubh_ushort            Shentsz(void)  const { return shentsz; }
  ubh_ushort            Shnum(void)    const { return shnum;   }

  void Set_shentsz(ubh_ushort shentsz)       { UBH_MHDR::shentsz = shentsz; }
  void setShnum(ubh_ushort shnum)            { UBH_MHDR::shnum = shnum;     }
}; // uWasm binary module header


typedef enum {
  SEC_CUST      = 0,
  SEC_TYPE      = 1,
  SEC_IMPORT    = 2,
  SEC_FUNC      = 3,
  SEC_TAB       = 4,
  SEC_MEM       = 5,     // MemSec with Min and Max
  SEC_GLBL      = 6,     // I32 globals
  SEC_EXPORT    = 7,
  SEC_START     = 8,
  SEC_ELEM      = 9,
  SEC_CODE      = 10,
  SEC_DATA      = 11,
  SEC_GLBLI64   = 12,
  SEC_GLBLF32   = 13,
  SEC_GLBLF64   = 14,
  SEC_MEM_MIN   = 15,     // MemSec with Min only
  SEC_STR_Mod   = 16,     // Module string
  SEC_STR_Name  = 17,     // Name string
  SEC_INIT_BYTE = 18,     // initialization bytes table
} SEC_ID;

// Section header
class UBH_SHDR {
private:
  ubh_uchar   sh_id;
  ubh_uint    sh_sz : 24;  // assume 4-byte aligned => max is 2**28
                           // must be access through an access function
  ubh_uint    sh_idx;      // index to table of the section
public:
  ubh_uchar ShId()  const        { return sh_id;   }
  ubh_uint  ShSz()  const        { return sh_sz;   }
  ubh_uint  ShIdx() const        { return sh_idx;  }
  void Set_shId(ubh_uchar shId)  { sh_id = shId;   }
  void Set_shSz(ubh_uint shSz)   { sh_sz = shSz;   }
  void Set_shIdx(ubh_uint shIdx) { sh_idx = shIdx; }
};


// Functype (parameter and results table can follow right after this struct
class UBH_FUNCTYPE {
private:
  ubh_uchar   _tag = 0x60;       // 0x60
  ubh_uint    _parm_sidx : 12;   // short index to parm table
  ubh_uint    _res_sidx  : 12;   // short index to results table
public:
  ubh_uchar Tag()      const                 { return _tag;       }
  ubh_uint  ParmSidx() const                 { return _parm_sidx; }
  ubh_uint  ResSidx()  const                 { return _res_sidx;  }
  void      Set_parm_sidx(ubh_uint parmSidx) {
    Is_True(parmSidx >= (1 << 12), ("parmSidx out of range"));
    _parm_sidx = parmSidx;
  }
  void      Set_res_sidx(ubh_uint resSidx)   {
    Is_True(resSidx >= (1 << 12), ("parmSidx out of range"));
    _res_sidx = resSidx;
  }
};


// Import 
class UBH_IMPORT {
private:
  ubh_uchar   _tag = 0x2;        // 0x2
  ubh_uchar   _desctag;          // func: 0, table: 1, mem:2, global: 3
  ubh_uint    _modstrIdx;        // begin of module string table 
  ubh_uint    _namestrIdx;       // begin of name string table
  // ??? desc ???
public:
  UBH_IMPORT() {}
  UBH_IMPORT(ubh_uchar desctag) : _desctag(desctag) {
    Is_True(desctag > 3 || desctag < 0, ("unknown desctag"));
  }

  ubh_uchar Tag()        const             { return _tag;              }
  ubh_uchar Desctag()    const             { return _desctag;          }
  ubh_uint  ModstrIdx()  const             { return _modstrIdx;        }
  ubh_uint  NamestrIdx() const             { return _namestrIdx;       }
  void Set_modstrIdx (ubh_uint modstrIdx)  { _modstrIdx = modstrIdx;   }
  void Set_namestrIdx(ubh_uint namestrIdx) { _namestrIdx = namestrIdx; }
};

// Table section

// Element section

// Limit min+max
class ubh_maxLimit {
private:
  ubh_uint    _minlimit;
  ubh_uint    _maxlimit;
public:
  ubh_uint  Minlimit() const                { return _minlimit;     }
  ubh_uint  Maxlimit() const                { return _maxlimit;     }
  void      Set_minlimit(ubh_uint minlimit) { _minlimit = minlimit; }
  void      Set_maxlimit(ubh_uint maxlimit) { _maxlimit = maxlimit; }
};

// Limit min  (must be padded to end at 64 bit boundary)
class ubh_minLimit {
private:
  ubh_uint   _minlimit;
public:
  ubh_uint Minlimit() const                { return _minlimit;     }
  void     Set_minlimit(ubh_uint minlimit) { _minlimit = minlimit; }
};


enum {
  VALTY_I32 = 0x7F,   // i32
  VALTY_I64 = 0x7E,   // i64
  VALTY_F32 = 0x7D,   // f32
  VALTY_F64 = 0x7C,   // f64
} VAL_TYPE;

// Global/local const 32 (must be padded to end at 64 bit boundary)
class ubh_constVal32 {
private:
  ubh_uint _val;
public:
  ubh_uint Val(void)  const { return _val; }
};

// Global/local const 64 bits
class ubh_constVal64 {
private:
  ubh_ulong _val;
public:
  ubh_ulong Val(void)  const  { return _val; }
  void Set_val(ubh_ulong val) { _val = val;  }
};

template <typename T>
class ubh_constVal {
private:
    T _val;
public:
    ubh_constVal(T val) : _val(val) {};
    T Val(void)  const { return _val; }
};

// Global/local variable 32 (must be padded to end at 64 bit boundary)
class ubh_Val32 {
private:
  ubh_uint  _val;   // this could be an instruction to initialize ??
public:
  ubh_uint Val(void)  const  { return _val; }
  void Set_val(ubh_uint val) { _val = val; }
};

template <typename T>
class ubh_Val {
private:
  T _val;
public:
  T Val(void)  const  { return _val; }
  void Set_val(T val) { _val = val; }
};

// Global/local variable 64
class ubh_Val64 {
private:
  ubh_ulong _val;   // this could be an instruction to initialize ??
public:
  ubh_ulong Val(void)  const { return _val; }
};
  
  
// Export
class UBH_EXPORT {
private:
  ubh_uint   _tag : 2;     // func: 0, table: 1, mem:2, global: 3
  ubh_uint   _idx : 30;    // offset to string in string table
public:
  ubh_uint Tag() const           { return _tag; }
  ubh_uint Idx() const           { return _idx; }
  void     Set_tag(ubh_uint tag) {
    Is_True(tag < 0 || tag > 3, ("tag out of range"));
    _tag = tag;
  }
  void     Set_idx(ubh_uint idx) {
    Is_True(idx >= (1 << 30), ("idx out of range"));
    _idx = idx;
  }
};


// Start not needed


#define BLOCK_INSTR_BEGIN 1
#define CONST_REG_NUM     2 // reg0 = 0, reg1 = 1
#define RET_REG_NUM       2 // reg use for return


class LABEL_ELEM {
public:
    UINT32 _pc;
    UINT32 _symbol_idx;
    LABEL_ELEM(){
      _pc = 0;
      _symbol_idx = 0;
    }
    LABEL_ELEM(UINT32 pc, UINT32 sym){
      _pc = pc;
      _symbol_idx = sym;
    }
    void Print(FILE *file = stdout) {
      fprintf(file, " block-id = 0x%08x, pc = 0x%08x\n", _symbol_idx, _pc);
    }
};


class LOCAL_TAB_ELEM {
private:
    REG_TY      _kind;
    OPTYPE_SZ   _val_type;
    LOCAL_IDX   _home_local_idx;  // only for locals
    // debug info
    UINT64       _push_stmt;      // which line(wasm inst offset) did push
public:
    LOCAL_TAB_ELEM(REG_TY kind, OPTYPE_SZ val_type, UINT64 push_stmt=0) :
        _kind(kind), _val_type(val_type), _push_stmt(push_stmt) {};
    REG_TY    Kind()                 const        { return _kind;                }
    OPTYPE_SZ ValType()              const        { return _val_type;            }
    LOCAL_IDX Home_local_idx()       const        { return _home_local_idx;      }
    UINT64    Push_stmt()            const        { return _push_stmt;           }
    void      Push_stmt(UINT64 push_stmt)         { _push_stmt = push_stmt;      }
    void      Set_local_idx(LOCAL_IDX local_idx)  { _home_local_idx = local_idx; }
};


typedef vector<UWASM_INST>          UWINSVEC;
typedef vector<LOCAL_TAB_ELEM>      LOCAL_TAB;

class WASM_STACK_ELEM; // implement in convertor.h
typedef stack<WASM_STACK_ELEM>      WASMSTK;

// Code
class UBH_CODE {
private:
  ubh_uint          _size;
  ubh_uint          _local_idx;  // ?
  ubh_uint          _inst_idx;   // ?
  UWINSVEC          _uwasm_instructions;
  vector<LOCAL_TAB> _local_table;         // reg elem stack
  vector<REG_IDX>   _local_map;           // map local idx(in wasm) to reg idx
public:
  UBH_CODE() {};
  UBH_CODE(wabt::Func* func);

  ubh_uint           Size()            const                 { return _size;                }
  ubh_uint           LocalIdx()        const                 { return _local_idx;           }
  ubh_uint           InstIdx()         const                 { return _inst_idx;            }
  UWINSVEC          *Get_insts()                             { return &_uwasm_instructions; }
  vector<LOCAL_TAB> *Get_local_table()                       { return &_local_table;        }
  vector<REG_IDX>   *Get_local_map()                         { return &_local_map;          }

  void Set_size(ubh_uint size)                               { _size = size;                }
  void Set_local_idx(ubh_uint local_idx)                     { _local_idx = local_idx;      }
  void Set_inst_idx(ubh_uint inst_idx)                       { _inst_idx = inst_idx;        }

  void Set_insts(const UWINSVEC &insts)                      { _uwasm_instructions = insts; }
  void Set_local_table(const vector<LOCAL_TAB> &local_table) { _local_table = local_table;  }
  void Set_local_map(const vector<REG_IDX> &local_map)       { _local_map = local_map;      }

  void Print(FILE_CONTEXT &ctx, FILE *outfile=stdout, FILE *file=stdout);
  UINT32 Get_local_num();
  REG_IDX Push_temp_reg(OPTYPE_SZ _type, UINT64 pc, WASMSTK& wasm_stack);
};


// Data
class UBH_DATA {
private:
  ubh_uchar  _glbl_idx = 0; // must be 0 now
  ubh_ushort _init_sz;      // size of initializaton (limited to 2**16 now)
  ubh_uint   _inst_idx;     // offset of memory through calculation
  ubh_uint   _init_idx;
public:
  ubh_uchar  GlblIdx() const                { return _glbl_idx; }
  ubh_ushort InitSz()  const                { return _init_sz;  }
  ubh_uint   InstIdx() const                { return _inst_idx; }
  ubh_uint   InitIdx() const                { return _init_idx; }
  void       Set_initSz (ubh_ushort initSz) { _init_sz = initSz;   }
  void       Set_instIdx(ubh_uint instIdx ) { _inst_idx = instIdx; }
  void       Set_initIdx(ubh_uint initIdx ) { _init_idx = initIdx; }
};


#endif // _UBH_H
