//
// Created by xc5 on 2020/7/11.
//

#ifndef WASM2UWASM_BASICS_H
#define WASM2UWASM_BASICS_H

typedef unsigned int       UINT32;
typedef int                INT32;
typedef long long int      INT64;
typedef short              INT16;
typedef char               INT8;
typedef unsigned long long UINT64;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;
typedef char               INT8;
typedef UINT8 *            IO_BUFFER;
typedef const UINT8 *      IO_CBUFFER;
typedef bool               BOOL;

#define Is_Trace(cond, msg) if ((cond)) { fprintf msg; }
#define TFile stdout
#define AssertDev(Condition, Params) \
        (Condition ? (void) 1 \
        :(Quit_with_tracing ( __FILE__, __LINE__, __func__),	\
        Assertion_Failure_Print Params) );

#define DBAR "===============================================\n"

/* Only in non-release mode */
# define AssertThat AssertDev
# define Is_True AssertDev

#define INLINE inline

// This is vulnerable to changes
typedef enum {
  TP_PARSE   = 1,
  TP_BLK     = 2,
  TP_BLK_GET   = 3,
  TP_BLK_BUILD = 4,
  TP_BLK_WINS  = 5,
  TP_STACK   = 6,
  TP_PRINT   = 7,
  TP_BIN_READ = 10,
  TP_EXEC    = 20, // 20-30 reserved
  TP_END     = 63,
} TRACE_KIND;

extern UINT32 Set_tracing_opt(TRACE_KIND kind, UINT32 level);
extern bool Tracing(TRACE_KIND);
extern void Quit_with_tracing(const char *, UINT32, const char *); // Quiting
extern void Assertion_Failure_Print ( const char *fmt, ... ); // Printf-like function
extern void Comp_Failure_Print ( const char *fmt, ... ); // Compilation Failure message printing


template <typename T>
T Endian_swap(T num) {
  T res = 0;
  T mask = 0xff;
  int i;
  int _sz = sizeof(T)-1;
  for (i = _sz; i >= 1; i -= 2)
    res |= ((num & (mask << (_sz - i) * 4)) << (i * 8));
  for (i = 1; i <= _sz; i += 2)
    res |= ((num & (mask << (i + _sz) * 4)) >> (i * 8));
  return res;
}

typedef enum {
  I32 = -0x01,      // 0x7f
  I64 = -0x02,      // 0x7e
  F32 = -0x03,      // 0x7d
  F64 = -0x04,      // 0x7c
  V128 = -0x05,     // 0x7b
  I8 = -0x06,       // 0x7a  : packed-type only, used in gc and as v128 lane
  I16 = -0x07,      // 0x79  : packed-type only, used in gc and as v128 lane
  Funcref = -0x10,  // 0x70
  Anyref = -0x11,   // 0x6f
  Nullref = -0x12,  // 0x6e
  Exnref = -0x18,   // 0x68
  Func = -0x20,     // 0x60
  Struct = -0x21,   // 0x5f
  Array = -0x22,    // 0x5e
  VOID = -0x40,     // 0x40
  ___ = VOID,       // Convenient for the opcode table in opcode.h

  Any = 0,          // Not actually specified, but useful for type-checking
  Hostref = 2,      // Not actually specified, but used in testing and type-checking
  I8U = 4,   // Not actually specified, but used internally with load/store
  I16U = 6,  // Not actually specified, but used internally with load/store
  I32U = 7,  // Not actually specified, but used internally with load/store
} OPTYPE_SZ;

typedef enum {
    LOCAL,
    GLOBAL,
    TEMP,
    FORMAL_PARAM,
    CALL_PARAM,
} REG_TY;

#define Comp_Failure(msg ...) { Quit_with_tracing ( __FILE__, __LINE__, __func__ ); Comp_Failure_Print(msg); }

#endif //WASM2UWASM_BASICS_H
