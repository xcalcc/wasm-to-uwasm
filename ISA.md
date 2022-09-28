uWASM ISA Definition

## Registers

Each register is labelled with a 12-biit index.
Thus having a limit of totally at most 4096 registers.

We will use R0 - R4095 to represent them.

#### Dedicated registers

There are three read-only registers that has a dedicated value. 
- R0 = (i32) 0
- R1 = (i32) 1
- R4095 = (i32) -1

#### Other registers' use

We are now considering to put the R2-R4094 into three instruction classes.  
- The locals 
- The temporaries
- The globals

Their layout is still under review.  

## Data Types

There are four data types holded in registers and used in instructions currently, they are:
- i32
- i64
- u32
- u64 

## Instructions

The instructions are either 32-bit long or 64-bit long.
The 0-bit (little-endian) in the first byte determines whether this should be a 64-bit instruction or 32-bit instruction.  

We have the following kind of instruction patterns

- Adress-mode-32,  which contains a 7-bit opcode
- Adress-mode-32_R, which contains a 7-bit opcode, and a 12-bit register number
- Adress-mode-32_R_OFS, which contains a 7-bit opcode, and a 12-bit register number, and 12-bit offset/immediate number
- Adress-mode-32_R_R, which contains a 7-bit opcode, and a 12-bit register number, and 12-bit register number
- Adress-mode-64_R_R_R, which contains a 15-bit opcode, and three 12-bit register numbers.
- Adress-mode-64_R_OFL, which contains a 15-bit opcode, and a 12-bit register number, and a 32-bit offset/immediate number 
- Adress-mode-64_R_R_OFS, which contains a 15-bit opcode, and two 12-bit register numbers, and a 24-bit offset/immediate number

We devide the instructions into following groups:

### Numeric Instructions

Numeric(Arithmetic instructions are often based on registers)
They are mostly: 
- AM32_R_R
- AM64_R_R_R
- AM32_R
- AM64_R_OFS

Examples:

- i32.add AM64_R_R_R
- i32.mul AM64_R_R_R
- i32.eqz AM32_R_R
- i32.ne  AM64_R_R_R
- i32.const AM64_R_OFS
- i64.const AM64_R_OFS
- f32.const AM64_R_OFS
- f64.const AM64_R_OFS


### Memory instructions

There are three major kind of memory instructions

#### Load and stores

This include
- i32.load_long AM64_R_OFS  
- i64.load_long AM64_R_OFS  
- f32.load_long AM64_R_OFS
- f64.load_long AM64_R_OFS  


- i32.load_short AM64_R_R_OFS  
- i64.load_short AM64_R_R_OFS  
- f32.load_short AM64_R_R_OFS
- f64.load_short AM64_R_R_OFS  


- i32.store_long AM64_R_OFS    
- i64.store_long AM64_R_OFS  
- f32.store_long AM64_R_OFS    
- f64.store_long AM64_R_OFS      

#### Memory utilities
- memory.size   AM32_R
- memory.grow   AM32_R


#### Control Instructions

Labels aree stored in a table comes with the binary.

Controls are determined by labels, jumps, conditional jumps, and function-calls.

- jeq, jne, jlt, jle, jgt, jge    AM32_R_OFS  
- jez, jnz, jlz, jlez, jgz, jgez  AM32_R_OFS, 12-bit offset  

- call_short AM32_OFS,  this has a function index of 20-bit. 
- call_long  AM64_R_OFS,  this has a function index of 32-bit. 
- indirect.call AM32_R,  this uses a function index stored in a 12-bit indexed register


## VM Execution

### Calling

The call instruction should be like
```asm
call $call_ofst 
```

Where the local table should contains something like the following:

|  Register Index | Get_kind | Content |
|  ----  | ----  | ---- |
|  2  | Formal Param | (set by VM)  |
|  ... | Formal Param | (set by VM)  |
|  i > 3 | Local/Temp | (local or temporaries usable by code)  |
|  i + 1 | Local/Temp | (local or temporaries usable by code)  |
|  ...  | Local/Temp | (local or temporaries usable by code)  |
|  j  | Param area | (will be used as actual param in calls) |
|  j + 1  | Param area | (will be used as actual param in calls) |
|  ...  | Param area | (will be used as actual param in calls) |

Calling of each function is described in the following section.


### Internal actions done during calling

1. Finding the function pointer under the offset
2. Reserve the a block for locals for the callee function in the register stack
3. Store the register base pointer (RBP) somewhere (probably on register stack)
4. Update the register base pointer (RBP)
5. Copy the register param to the callee local block
6. Execute the next function
7. When that function returns, reset the register base pointer (RBP)
8. Continuing executing the caller function  


### Dealing with recursions

If the VM follows the above workings, the recursion should be resolved just fine.
The function execution of a call graph
```
call graph:
A -> B---> C
     |---> A  (recursion)
     \---> D 
```

The stack before A calling B

|  Function in stack |
| ---- |
|  A |

The stack before B calling C

|  Function in stack |
| ---- |
|  A |
|  B |

The stack while C is executing

|  Function in stack |
| ---- |
|  A |
|  B |
|  C |


The stack before B calling A

|  Function in stack |
| ---- |
|  A |
|  B |

The stack after B called A

|  Function in stack |
| ---- |
|  A |
|  B |
|  A |


The stack after A returns to B

|  Function in stack |
| ---- |
|  A |
|  B |

The stack after B called D

|  Function in stack |
| ---- |
|  A |
|  B |
|  D |


## Appendix

a current instruction table. 

```c
//     name                       wasm_op, uwasm_op ty_am
    {  "nop",                     0x00,   0x00,   TY_AM32,        }, // nop
    {  "return",                  0x0f,   0x0f,   TY_AM32_R,      }, // ?
    {  "i32.load_long",           0x28,   0x28,   TY_AM64_R_OFS   },
    {  "i64.load_long",           0x29,   0x29,   TY_AM64_R_OFS   },
    {  "f32.load_long",           0x2a,   0x2a,   TY_AM64_R_OFS   },
    {  "f64.load_long",           0x2b,   0x2b,   TY_AM64_R_OFS   },
    {  "i32.load8_s",             0x2c,   0x2c,   TY_AM64_R_OFS   },
    {  "i32.load8_u",             0x2d,   0x2d,   TY_AM64_R_OFS   },
    {  "i32.load16_s",            0x2e,   0x2e,   TY_AM64_R_OFS   },
    {  "i32.load16_u",            0x2f,   0x2f,   TY_AM64_R_OFS   },
    {  "i64.load8_s",             0x30,   0x30,   TY_AM64_R_OFS   },
    {  "i64.load8_u",             0x31,   0x31,   TY_AM64_R_OFS   },
    {  "i64.load16_s",            0x32,   0x32,   TY_AM64_R_OFS   },
    {  "i64.load16_u",            0x33,   0x33,   TY_AM64_R_OFS   },
    {  "i64.load32_s",            0x34,   0x34,   TY_AM64_R_OFS   },
    {  "i64.load32_u",            0x35,   0x35,   TY_AM64_R_OFS   },
    {  "i32.store_long",          0x36,   0x36,   TY_AM64_R_OFS },
    {  "i64.store_long",          0x37,   0x37,   TY_AM64_R_OFS   },
    {  "f32.store_long",          0x38,   0x38,   TY_AM64_R_OFS   },
    {  "f64.store_long",          0x39,   0x39,   TY_AM64_R_OFS   },
    {  "i32.store8",              0x3a,   0x3a,   TY_AM64_R_OFS   },
    {  "i32.store16",             0x3b,   0x3b,   TY_AM64_R_OFS   },
    {  "i64.store8",              0x3c,   0x3c,   TY_AM64_R_OFS   },
    {  "i64.store16",             0x3d,   0x3d,   TY_AM64_R_OFS   },
    {  "i64.store32",             0x3e,   0x3e,   TY_AM64_R_OFS   },
    {  "memory.size",             0x3f,   0x3f,   TY_AM32_R       },
    {  "memory.grow",             0x40,   0x40,   TY_AM32_R       },
    {  "i32.const",               0x41,   0x41,   TY_AM64_R_OFS   },
    {  "i64.const",               0x42,   0x42,   TY_AM64_R_OFS   },
    {  "f32.const",               0x43,   0x43,   TY_AM64_R_OFS   },
    {  "f64.const",               0x44,   0x44,   TY_AM64_R_OFS   },
    {  "i32.eqz",                 0x45,   0x45,   TY_AM64_R_R_R   },
    {  "i32.eq",                  0x46,   0x46,   TY_AM64_R_R_R   },
    {  "i32.ne",                  0x47,   0x47,   TY_AM64_R_R_R   },
    {  "i32.lt_s",                0x48,   0x48,   TY_AM64_R_R_R   },
    {  "i32.lt_u",                0x49,   0x49,   TY_AM64_R_R_R   },
    {  "i32.gt_s",                0x4a,   0x4a,   TY_AM64_R_R_R   },
    {  "i32.gt_u",                0x4b,   0x4b,   TY_AM64_R_R_R   },
    {  "i32.le_s",                0x4c,   0x4c,   TY_AM64_R_R_R   },
    {  "i32.le_u",                0x4d,   0x4d,   TY_AM64_R_R_R   },
    {  "i32.ge_s",                0x4e,   0x4e,   TY_AM64_R_R_R   },
    {  "i32.ge_u",                0x4f,   0x4f,   TY_AM64_R_R_R   },
    {  "i64.eqz",                 0x50,   0x50,   TY_AM32_R_R   },
    {  "i64.eq",                  0x51,   0x51,   TY_AM64_R_R_R   },
    {  "i64.ne",                  0x52,   0x52,   TY_AM64_R_R_R   },
    {  "i64.lt_s",                0x53,   0x53,   TY_AM64_R_R_R   },
    {  "i64.lt_u",                0x54,   0x54,   TY_AM64_R_R_R   },
    {  "i64.gt_s",                0x55,   0x55,   TY_AM64_R_R_R   },
    {  "i64.gt_u",                0x56,   0x56,   TY_AM64_R_R_R   },
    {  "i64.le_s",                0x57,   0x57,   TY_AM64_R_R_R   },
    {  "i64.le_u",                0x58,   0x58,   TY_AM64_R_R_R   },
    {  "i64.ge_s",                0x59,   0x59,   TY_AM64_R_R_R   },
    {  "i64.ge_u",                0x5a,   0x5a,   TY_AM64_R_R_R   },
    {  "f32.eq",                  0x5b,   0x5b,   TY_AM64_R_R_R   },
    {  "f32.ne",                  0x5c,   0x5c,   TY_AM64_R_R_R   },
    {  "f32.lt",                  0x5d,   0x5d,   TY_AM64_R_R_R   },
    {  "f32.gt",                  0x5e,   0x5e,   TY_AM64_R_R_R   },
    {  "f32.le",                  0x5f,   0x5f,   TY_AM64_R_R_R   },
    {  "f32.ge",                  0x60,   0x60,   TY_AM64_R_R_R   },
    {  "f64.eq",                  0x61,   0x61,   TY_AM64_R_R_R   },
    {  "f64.ne",                  0x62,   0x62,   TY_AM64_R_R_R   },
    {  "f64.lt",                  0x63,   0x63,   TY_AM64_R_R_R   },
    {  "f64.gt",                  0x64,   0x64,   TY_AM64_R_R_R   },
    {  "f64.le",                  0x65,   0x65,   TY_AM64_R_R_R   },
    {  "f64.ge",                  0x66,   0x66,   TY_AM64_R_R_R   },
    {  "i32.clz",                 0x67,   0x67,   TY_AM32_R_R   },
    {  "i32.ctz",                 0x68,   0x68,   TY_AM32_R_R   },
    {  "i32.popcnt",              0x69,   0x69,   TY_AM64_R_OFS   },
    {  "i32.add",                 0x6a,   0x6a,   TY_AM64_R_R_R   },
    {  "i32.sub",                 0x6b,   0x6b,   TY_AM64_R_R_R   },
    {  "i32.mul",                 0x6c,   0x6c,   TY_AM64_R_R_R   },
    {  "i32.div_s",               0x6d,   0x6d,   TY_AM64_R_R_R   },
    {  "i32.div_u",               0x6e,   0x6e,   TY_AM64_R_R_R   },
    {  "i32.rem_s",               0x6f,   0x6f,   TY_AM64_R_R_R   },
    {  "i32.rem_u",               0x70,   0x70,   TY_AM64_R_R_R   },
    {  "i32.and",                 0x71,   0x71,   TY_AM64_R_R_R   },
    {  "i32.or",                  0x72,   0x72,   TY_AM64_R_R_R   },
    {  "i32.xor",                 0x73,   0x73,   TY_AM64_R_R_R   },
    {  "i32.shl",                 0x74,   0x74,   TY_AM64_R_R_R   },
    {  "i32.shr_s",               0x75,   0x75,   TY_AM64_R_R_R   },
    {  "i32.shr_u",               0x76,   0x76,   TY_AM64_R_R_R   },
    {  "i32.rotl",                0x77,   0x77,   TY_AM64_R_R_R   },
    {  "i32.rotr",                0x78,   0x78,   TY_AM64_R_R_R   },
    {  "i64.clz",                 0x79,   0x79,   TY_AM64_R_R_R   },
    {  "i64.ctz",                 0x7a,   0x7a,   TY_AM64_R_R_R   },
    {  "i64.popcnt",              0x7b,   0x7b,   TY_AM64_R_R_R   },
    {  "i64.add",                 0x7c,   0x7c,   TY_AM64_R_R_R   },
    {  "i64.sub",                 0x7d,   0x7d,   TY_AM64_R_R_R   },
    {  "i64.mul",                 0x7e,   0x7e,   TY_AM64_R_R_R   },
    {  "i64.div_s",               0x7f,   0x7f,   TY_AM64_R_R_R   },
    {  "i64.div_u",               0x80,   0x80,   TY_AM64_R_R_R   },
    {  "i64.rem_s",               0x81,   0x81,   TY_AM64_R_R_R   },
    {  "i64.rem_u",               0x82,   0x82,   TY_AM64_R_R_R   },
    {  "i64.and",                 0x83,   0x83,   TY_AM64_R_R_R   },
    {  "i64.or",                  0x84,   0x84,   TY_AM64_R_R_R   },
    {  "i64.xor",                 0x85,   0x85,   TY_AM64_R_R_R   },
    {  "i64.shl",                 0x86,   0x86,   TY_AM64_R_R_R   },
    {  "i64.shr_s",               0x87,   0x87,   TY_AM64_R_R_R   },
    {  "i64.shr_u",               0x88,   0x88,   TY_AM64_R_R_R   },
    {  "i64.rotl",                0x89,   0x89,   TY_AM64_R_R_R   },
    {  "i64.rotr",                0x8a,   0x8a,   TY_AM64_R_R_R   },
    {  "f32.abs",                 0x8b,   0x8b,   TY_AM64_R_R_R   },
    {  "f32.neg",                 0x8c,   0x8c,   TY_AM64_R_R_R   },
    {  "f32.ceil",                0x8d,   0x8d,   TY_AM64_R_R_R   },
    {  "f32.floor",               0x8e,   0x8e,   TY_AM64_R_R_R   },
    {  "f32.trunc",               0x8f,   0x8f,   TY_AM64_R_R_R   },
    {  "f32.nearest",             0x90,   0x90,   TY_AM64_R_R_R   },
    {  "f32.sqrt",                0x91,   0x91,   TY_AM64_R_R_R   },
    {  "f32.add",                 0x92,   0x92,   TY_AM64_R_R_R   },
    {  "f32.sub",                 0x93,   0x93,   TY_AM64_R_R_R   },
    {  "f32.mul",                 0x94,   0x94,   TY_AM64_R_R_R   },
    {  "f32.div",                 0x95,   0x95,   TY_AM64_R_R_R   },
    {  "f32.min",                 0x96,   0x96,   TY_AM64_R_R_R   },
    {  "f32.max",                 0x97,   0x97,   TY_AM64_R_R_R   },
    {  "f32.copysign",            0x98,   0x98,   TY_AM64_R_R_R   },
    {  "f64.abs",                 0x99,   0x99,   TY_AM64_R_R_R   },
    {  "f64.neg",                 0x9a,   0x9a,   TY_AM64_R_R_R   },
    {  "f64.ceil",                0x9b,   0x9b,   TY_AM64_R_R_R   },
    {  "f64.floor",               0x9c,   0x9c,   TY_AM64_R_R_R   },
    {  "f64.trunc",               0x9d,   0x9d,   TY_AM64_R_R_R   },
    {  "f64.nearest",             0x9e,   0x9e,   TY_AM64_R_R_R   },
    {  "f64.sqrt",                0x9f,   0x9f,   TY_AM64_R_R_R   },
    {  "f64.add",                 0xa0,   0xa0,   TY_AM64_R_R_R   },
    {  "f64.sub",                 0xa1,   0xa1,   TY_AM64_R_R_R   },
    {  "f64.mul",                 0xa2,   0xa2,   TY_AM64_R_R_R   },
    {  "f64.div",                 0xa3,   0xa3,   TY_AM64_R_R_R   },
    {  "f64.min",                 0xa4,   0xa4,   TY_AM64_R_R_R   },
    {  "f64.max",                 0xa5,   0xa5,   TY_AM64_R_R_R   },
    {  "f64.copysign",            0xa6,   0xa6,   TY_AM64_R_R_R   },
    {  "i32.wrap_i64",            0xa7,   0xa7,   TY_AM64_R_R_R   },
    {  "i32.trunc_f32_s",         0xa8,   0xa8,   TY_AM64_R_R_R   },
    {  "i32.trunc_f32_u",         0xa9,   0xa9,   TY_AM64_R_R_R   },
    {  "i32.trunc_f64_s",         0xaa,   0xaa,   TY_AM64_R_R_R   },
    {  "i32.trunc_f64_u",         0xab,   0xab,   TY_AM64_R_R_R   },
    {  "i64.extend_i32_s",        0xac,   0xac,   TY_AM64_R_R_R   },
    {  "i64.extend_i32_u",        0xad,   0xad,   TY_AM64_R_R_R   },
    {  "i64.trunc_f32_s",         0xae,   0xae,   TY_AM64_R_R_R   },
    {  "i64.trunc_f32_u",         0xaf,   0xaf,   TY_AM64_R_R_R   },
    {  "i64.trunc_f64_s",         0xb0,   0xb0,   TY_AM64_R_R_R   },
    {  "i64.trunc_f64_u",         0xb1,   0xb1,   TY_AM64_R_R_R   },
    {  "f32.convert_i32_s",       0xb2,   0xb2,   TY_AM64_R_R_R   },
    {  "f32.convert_i32_u",       0xb3,   0xb3,   TY_AM64_R_R_R   },
    {  "f32.convert_i64_s",       0xb4,   0xb4,   TY_AM64_R_R_R   },
    {  "f32.convert_i64_u",       0xb5,   0xb5,   TY_AM64_R_R_R   },
    {  "f32.demote_f64",          0xb6,   0xb6,   TY_AM64_R_R_R   },
    {  "f64.convert_i32_s",       0xb7,   0xb7,   TY_AM64_R_R_R   },
    {  "f64.convert_i32_u",       0xb8,   0xb8,   TY_AM64_R_R_R   },
    {  "f64.convert_i64_s",       0xb9,   0xb9,   TY_AM64_R_R_R   },
    {  "f64.convert_i64_u",       0xba,   0xba,   TY_AM64_R_R_R   },
    {  "f64.promote_f32",         0xbb,   0xbb,   TY_AM64_R_R_R   },
    {  "i32.reinterpret_f32",     0xbc,   0xbc,   TY_AM64_R_R_R   },
    {  "i64.reinterpret_f64",     0xbd,   0xbd,   TY_AM64_R_R_R   },
    {  "f32.reinterpret_i32",     0xbe,   0xbe,   TY_AM64_R_R_R   },
    {  "f64.reinterpret_i64",     0xbf,   0xbf,   TY_AM64_R_R_R   },
    {  "i32.extend8_s",           0xC0,   0xC0,   TY_AM64_R_R_R   },
    {  "i32.extend16_s",          0xC1,   0xC1,   TY_AM64_R_R_R   },
    {  "i64.extend8_s",           0xC2,   0xC2,   TY_AM64_R_R_R   },
    {  "i64.extend16_s",          0xC3,   0xC3,   TY_AM64_R_R_R   },
    {  "i64.extend32_s",          0xC4,   0xC4,   TY_AM64_R_R_R   },
    {  "i32.trunc_sat_f32_s",     0x00,   0x00,   TY_AM32_R_R     },
    {  "i32.trunc_sat_f32_u",     0x01,   0x01,   TY_AM32_R_R     },
    {  "i32.trunc_sat_f64_s",     0x02,   0x02,   TY_AM32_R_R     },
    {  "i32.trunc_sat_f64_u",     0x03,   0x03,   TY_AM32_R_R     },
    {  "i64.trunc_sat_f32_s",     0x04,   0x04,   TY_AM32_R_R     },
    {  "i64.trunc_sat_f32_u",     0x05,   0x05,   TY_AM32_R_R     },
    {  "i64.trunc_sat_f64_s",     0x06,   0x06,   TY_AM32_R_R     },
    {  "i64.trunc_sat_f64_u",     0x07,   0x07,   TY_AM32_R_R     },
 ```
