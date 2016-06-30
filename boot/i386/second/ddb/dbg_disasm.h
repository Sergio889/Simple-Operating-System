#ifndef DBG
#define DBG
#include "../term.h"
#include "../heap.h"
#include "dbg.h"
#define WITH_MODRM 0x01 // MOD/RM follows opcode
#define WITH_REG 0x02 // REG in MOD/RM selects a register as operand
#define WITH_SIB 0x04
#define WITH_DIS 0x08
#define WITH_IMM 0x10
#define BYTE 0x20
#define WITH_rAX 0x40
#define WITH_rBX 0x80
#define WITH_rCX 0x100
#define WITH_rDX 0x200
#define WITH_rDI 0x400
#define WITH_rSI 0x800
#define WITH_rBP 0x1000
#define WITH_rSP 0x2000
#define WITH_eAX 0x40
#define WITH_eBX 0x80
#define WITH_eCX 0x100
#define WITH_eDX 0x200
#define WITH_eDI 0x400
#define WITH_eSI 0x800
#define WITH_eBP 0x1000
#define WITH_eSP 0x2000
#define WITH_EIP 0x4000
#define WITH_MEM 0x8000
#define WITH_SS 0x10000
#define WITH_ES 0x20000
#define WITH_DS 0x40000
#define WITH_STR 0x80000
#define WITH_BH 0x100000
#define WITH_DH 0x200000
#define WITH_CH 0x400000
#define WITH_AH 0x800000
#define WITH_BL 0x1000000
#define WITH_DL 0x2000000
#define WITH_CL 0x4000000
#define WITH_AL 0x8000000
#define WITH_PSEUDO 0x8000000
#define WITH_MODREG 0x8000000
#define WITH_CS 0x10
#define WITH_DX 0x10
#define WITH_SEG 0x10
#define WITH_DIRA 0x10
#define WORD 0x10
#define NUMBER_1 0x10

struct op_map {
	word op_code;
	char *op_str;
	word op_dst; // how to determine first operand
	word op_src; // how to determine second operand
	word op_oth; // some opcodes require 3 operands
};

struct xop_map {
	char *op_str;
	byte op_ext;
	word op_dst; // how to determine first operand
	word op_src; // how to determine second operand
	word op_oth; // some opcodes require 3 operands
};
#endif
