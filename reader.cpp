#include "reader.h"
#include "basics.h"

UINT64 Read_leb128(IO_CBUFFER pc) {
  UINT64 result = 0;
  UINT32 offset = 0; // 7, 15, 23, 31, 39, 47, 55, 63,
  for (UINT32 current = *(UINT8 *)pc; (current >> 7) != 0; pc++, current = *(UINT8 *)pc) {
    result += (current & 0x7f) << offset;
    offset += 7;
  }
  result += ((UINT32) *(UINT8 *)pc & 0x7f) << offset;
  return result;
}


UINT8 Read_opr_size(IO_CBUFFER pc) {
  UINT8 result = 1;
  for (UINT32 current = *(UINT8 *)pc; ((current >> 7) & 0x01) != 0; pc++, current = *(UINT8 *)pc) {
    result ++;
  }
  return result;
}

UINT8  Read_uint8(IO_CBUFFER pc) {
  UINT8 res = *pc;
  return res;
}
