#include "dbg_disasm.h"

struct xop_map xop_grp1[] = {
	"add", 0x00, 0, 0, 0,
	"or", 0x01, 0, 0, 0,
	"adc", 0x02, 0, 0, 0,
	"sbb", 0x03, 0, 0, 0,
	"and", 0x04, 0, 0, 0,
	"sub", 0x05, 0, 0, 0,
	"xor", 0x06, 0, 0, 0,
	"cmp", 0x07, 0, 0, 0
};

struct xop_map xop_grp2[] = {
	"rol", 0x00, 0, 0, 0,
	"ror", 0x01, 0, 0, 0,
	"rcl", 0x02, 0, 0, 0,
	"rcr", 0x03, 0, 0, 0,
	"shl", 0x04, 0, 0, 0,
	"shr", 0x05, 0, 0, 0,
	"sar", 0x07, 0, 0, 0
};

struct xop_map xop_grp3[] = {
	"test", 0x00, 0, 0, 0,
	"not", 0x02, 0, 0, 0,
	"neg", 0x03, 0, 0, 0,
	"mul", 0x04, 0, 0, 0,
	"imul", 0x05, 0, 0, 0,
	"div", 0x06, 0, 0, 0,
	"idiv", 0x07, 0, 0, 0
};

struct xop_map xop_grp4[] = {
	"inc", 0x00, 0, 0, 0,
	"dec", 0x02, 0, 0, 0
};

struct xop_map xop_grp11[] = {
	"mov", 0x00, 0, 0, 0
};

struct xop_map xop_grp1A[] = {
	"pop", 0x00, 0, 0, 0
};

struct xop_map xop_grp5[] = {
	"inc", 0x00, 0, 0, 0,
	"dec", 0x01, 0, 0, 0,
	"calln", 0x02, 0, 0, 0,
	"callf", 0x03, 0, 0, 0,
	"jmpn", 0x04, 0, 0, 0,
	"jmpf", 0x05, 0, 0, 0,
	"push", 0x06, 0, 0, 0
};

struct op_map op_list_1[] = {
	0x00000000, "addb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000001, "add", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x00000002, "addb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x00000003, "add", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x00000004, "add", WITH_MODRM, WITH_MODRM|WITH_REG, 0, // XXX
	0x00000005, "add", 0, WITH_rAX, 0, 
	0x00000006, "push", 0, WITH_ES, 0, 
	0x00000007, "pop", 0, WITH_ES, 0, 
	0x00000008, "orb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000009, "or", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x0000000a, "orb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x0000000b, "or", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x0000000c, "or", WITH_MODRM, WITH_MODRM|WITH_REG, 0,  // XXX
	0x0000000d, "or", 0, WITH_rAX, 0,
	0x0000000e, "push", 0, WITH_CS, 0, 
	0x0000000f, NULL, 0, 0, 0, // 2-byte escape
	0x00000010, "adcb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000011, "adc", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x00000012, "adcb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x00000013, "adc", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x00000014, "adc", WITH_MODRM, WITH_MODRM|WITH_REG, 0, // XXX
	0x00000015, "adc", 0, WITH_rAX, 0, 
	0x00000016, "push", 0, WITH_SS, 0, 
	0x00000017, "pop", 0, WITH_SS, 0, 
	0x00000018, "sbb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000019, "sbb", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x0000001a, "sbb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x0000001b, "sbb", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x0000001c, "sbb", WITH_MODRM, WITH_MODRM|WITH_REG, 0,  // XXX
	0x0000001d, "sbb", 0, WITH_rAX, 0,
	0x0000001e, "push", 0, WITH_DS, 0, 
	0x0000001f, "pop", 0, WITH_DS, 0, 
	0x00000020, "andb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0,
	0x00000021, "and", WITH_MODRM, WITH_MODRM|WITH_REG, 0,
	0x00000022, "andb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0,
	0x00000023, "and", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x00000024, "and", WITH_MODRM, WITH_MODRM|WITH_REG, 0, // XXX
	0x00000025, "and", 0, WITH_rAX, 0,
	0x00000026, NULL, 0, 0, 0, // SEG=ES Prefix
	0x00000027, "daa", 0, 0, 0, 
	0x00000028, "subb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000029, "sub", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x0000002a, "subb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x0000002b, "sub", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x0000002c, "sub", WITH_MODRM, WITH_MODRM|WITH_REG, 0,  // XXX
	0x0000002d, "sub", 0, WITH_rAX, 0,
	0x0000002e, NULL, 0, 0, 0, // SEG=CS Prefix
	0x0000002f, "das", 0, 0, 0, 
	0x00000030, "xorb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000031, "xor", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x00000032, "xorb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x00000033, "xor", WITH_MODRM|WITH_REG, WITH_MODRM, 0, 
	0x00000034, "xor", WITH_MODRM, WITH_MODRM|WITH_REG, 0, // XXX
	0x00000035, "xor", 0, WITH_rAX, 0,
	0x00000036, NULL, 0, 0, 0, // SEG=SS Prefix
	0x00000037, "daa", 0, 0, 0, 
	0x00000038, "cmpb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000039, "cmp", WITH_MODRM, WITH_MODRM|WITH_REG, 0,
	0x0000003a, "cmpb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0, 
	0x0000003b, "cmp", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x0000003c, "cmp", WITH_MODRM, WITH_MODRM|WITH_REG, 0, // XXX
	0x0000003d, "cmp", 0, WITH_rAX, 0,
	0x0000003e, NULL, 0, 0, 0, // SEG=DS Prefix
	0x0000003f, "aas", 0, 0, 0,
	0x00000040, "inc", 0, WITH_eAX, 0, 
	0x00000041, "inc", 0, WITH_eCX, 0, 
	0x00000042, "inc", 0, WITH_eDX, 0, 
	0x00000043, "inc", 0, WITH_eBX, 0,
	0x00000044, "inc", 0, WITH_eSP, 0,
	0x00000045, "inc", 0, WITH_eBP, 0,
	0x00000046, "inc", 0, WITH_eSI, 0,
	0x00000047, "inc", 0, WITH_eDI, 0,
	0x00000048, "dec", 0, WITH_eAX, 0, 
	0x00000049, "dec", 0, WITH_eCX, 0,
	0x0000004a, "dec", 0, WITH_eDX, 0,
	0x0000004b, "dec", 0, WITH_eBX, 0,
	0x0000004c, "dec", 0, WITH_eSP, 0,
	0x0000004d, "dec", 0, WITH_eBP, 0,
	0x0000004e, "dec", 0, WITH_eSI, 0,
	0x0000004f, "dec", 0, WITH_eDI, 0,
	0x00000050, "push", 0, WITH_rAX, 0, 
	0x00000051, "push", 0, WITH_rCX, 0,
	0x00000052, "push", 0, WITH_rDX, 0,
	0x00000053, "push", 0, WITH_rBX, 0,
	0x00000054, "push", 0, WITH_rSP, 0,
	0x00000055, "push", 0, WITH_rBP, 0,
	0x00000056, "push", 0, WITH_rSI, 0, 
	0x00000057, "push", 0, WITH_rDI, 0, 
	0x00000058, "pop", 0, WITH_rAX, 0, 
	0x00000059, "pop", 0, WITH_rCX, 0,
	0x0000005a, "pop", 0, WITH_rDX, 0,
	0x0000005b, "pop", 0, WITH_rBX, 0,
	0x0000005c, "pop", 0, WITH_rSP, 0,
	0x0000005d, "pop", 0, WITH_rBP, 0,
	0x0000005e, "pop", 0, WITH_rSI, 0,
	0x0000005f, "pop", 0, WITH_rDI, 0, 
	0x00000060, "pusha", 0, 0, 0,
	0x00000061, "popa", 0, 0, 0,
	0x00000062, "bound", WITH_MODRM|WITH_REG, WITH_MODRM|WITH_MEM, 0,
	0x00000063, NULL, NULL, NULL, 0, // undefined
	0x00000064, NULL, 0, 0, 0, // SEG=FS Prefix
	0x00000065, NULL, 0, 0, 0, // SEG=GS Prefix
	0x00000066, NULL, 0, 0, 0, // Operand Size Prefix
	0x00000067, NULL, 0, 0, 0, // Address Size Prefix
	0x00000068, "push", WITH_IMM, 0, 0, 
	0x00000069, "imul", WITH_MODRM|WITH_REG, WITH_MODRM, WITH_IMM, 
	0x0000006a, "push", WITH_IMM|BYTE, 0, 0, 
	0x0000006b, "imul", WITH_MODRM|WITH_REG, WITH_MODRM, WITH_IMM|BYTE, 
	0x0000006c, "ins", WITH_ES|WITH_rDI|BYTE, WITH_DX, 0, 
	0x0000006d, "ins", WITH_ES|WITH_rDI, WITH_DX, 0, 
	0x0000006e, "outs", WITH_DX, WITH_ES|WITH_rDI|BYTE, 0, 
	0x0000006f, "outs", WITH_DX, WITH_ES|WITH_rDI, 0, 
	0x00000070, "jmp", 0, 0, 0,  // XXX
	0x00000071, "jmp", 0, 0, 0,  // XXX
	0x00000072, "jb", 0, WITH_DIS, 0,
	0x00000073, "jnb", 0, WITH_DIS, 0,
	0x00000074, "je", 0, WITH_DIS, 0,
	0x00000075, "jne", 0, WITH_DIS, 0,
	0x00000076, "jbe", 0, WITH_DIS, 0,
	0x00000077, "ja", 0, WITH_DIS, 0,
	0x00000078, "js", 0, 0, 0,
	0x00000079, "jns", 0, 0, 0,
	0x0000007a, "jp", 0, 0, 0,
	0x0000007b, "jnp", 0, 0, 0,
	0x0000007c, "jl", 0, 0, 0,
	0x0000007d, "jnl", 0, 0, 0,
	0x0000007e, "jle", 0, 0, 0,
	0x0000007f, "jnle", 0, 0, 0,
	0x00000080, NULL, WITH_MODRM|BYTE, WITH_IMM|BYTE, 0, // defined by extension
	0x00000081, NULL, WITH_MODRM, WITH_IMM, 0, // defined by extension
	0x00000082, NULL, WITH_MODRM|BYTE, WITH_IMM|BYTE, 0, // defined by extension
	0x00000083, NULL, WITH_MODRM, WITH_IMM|BYTE, 0, // defined by extension
	0x00000084, "testb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000085, "test", WITH_MODRM, WITH_MODRM|WITH_REG, 0,
	0x00000086, "xchgb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0,
	0x00000087, "xchg", WITH_MODRM, WITH_MODRM|WITH_REG, 0,
	0x00000088, "movb", WITH_MODRM|BYTE, WITH_MODRM|WITH_REG|BYTE, 0, 
	0x00000089, "mov", WITH_MODRM, WITH_MODRM|WITH_REG, 0, 
	0x0000008a, "movb", WITH_MODRM|WITH_REG|BYTE, WITH_MODRM|BYTE, 0,
	0x0000008b, "mov", WITH_MODRM|WITH_REG, WITH_MODRM, 0,
	0x0000008c, "mov", WITH_MODRM, WITH_MODRM|WITH_REG|WITH_SEG|WORD, 0,
	0x0000008d, "lea", WITH_MODRM|WITH_REG, WITH_MODRM|WITH_MEM, 0,
	0x0000008e, "mov", WITH_MODRM|WITH_REG|WITH_SEG|WORD, WITH_MODRM, 0,
	0x0000008f, NULL, 0, WITH_MODRM, 0, // defined by extension opcode
	0x00000090, "nop", 0, 0, 0,
	0x00000091, "xchg", WITH_rAX, WITH_rCX, 0,
	0x00000092, "xchg", WITH_rAX, WITH_rDX, 0, 
	0x00000093, "xchg", WITH_rAX, WITH_rBX, 0,
	0x00000094, "xchg", WITH_rAX, WITH_rSP, 0,
	0x00000095, "xchg", WITH_rAX, WITH_rBP, 0,
	0x00000096, "xchg", WITH_rAX, WITH_rSI, 0,
	0x00000097, "xchg", WITH_rAX, WITH_rDI, 0,
	0x00000098, "cbw", 0, 0, 0,
	0x00000099, "cwd", 0, 0, 0,
	0x0000009a, "callf", 0, WITH_DIRA, 0,
	0x0000009b, "fwait", 0, 0, 0,
	0x0000009c, "pushf", 0, 0, 0,
	0x0000009d, "popf", 0, 0, 0,
	0x0000009e, "sahf", 0, 0, 0,
	0x0000009f, "lahf", 0, 0, 0,
	0x000000a0, "mov", WITH_AL, WITH_IMM|BYTE, 0,
	0x000000a1, "mov", WITH_rAX, WITH_IMM, 0,  // XXX
	0x000000a2, "mov", WITH_IMM|BYTE, WITH_AL, 0, 
	0x000000a3, "mov", WITH_IMM, WITH_rAX, 0, // XXX
	0x000000a4, "movsb", WITH_STR|WITH_DS|WITH_rSI|BYTE, 
											 WITH_STR|WITH_ES|WITH_rDI|BYTE, 0,
	0x000000a5, "movs", WITH_STR|WITH_DS|WITH_rSI, WITH_STR|WITH_ES|WITH_rDI, 0,
	0x000000a6, "cmpsb", WITH_STR|WITH_DS|WITH_rSI|BYTE, 
											 WITH_STR|WITH_ES|WITH_rDI|BYTE, 0, 
	0x000000a7, "cmps", WITH_STR|WITH_DS|WITH_rSI, WITH_STR|WITH_ES|WITH_rDI, 0, 
	0x000000a8, "test", WITH_AL, WITH_IMM|BYTE, 0, 
	0x000000a9, "test", WITH_rAX, WITH_IMM, 0, 
	0x000000aa, "stosb", WITH_ES|WITH_rDI|BYTE, WITH_AL, 0, 
	0x000000ab, "stos", WITH_ES|WITH_rDI, WITH_rAX, 0, 
	0x000000ac, "lodsb", WITH_AL, WITH_DS|WITH_rSI|BYTE, 0, 
	0x000000ad, "lods", WITH_rAX, WITH_DS|WITH_rSI, 0, 
	0x000000ae, "scasb", WITH_AL, WITH_ES|WITH_rDI|BYTE, 0, 
	0x000000af, "scas", WITH_rAX, WITH_ES|WITH_rDI, 0, 
	0x000000b0, "movb", WITH_AL, WITH_IMM|BYTE, 0, 
	0x000000b1, "movb", WITH_BH, WITH_IMM|BYTE, 0, 
	0x000000b2, "movb", WITH_CL, WITH_IMM|BYTE, 0,
	0x000000b3, "movb", WITH_DL, WITH_IMM|BYTE, 0,
	0x000000b4, "movb", WITH_BL, WITH_IMM|BYTE, 0,
	0x000000b5, "movb", WITH_AH, WITH_IMM|BYTE, 0,
	0x000000b6, "movb", WITH_CH, WITH_IMM|BYTE, 0,
	0x000000b7, "movb", WITH_DH, WITH_IMM|BYTE, 0,
	0x000000b8, "mov", WITH_rAX, WITH_IMM, 0, 
	0x000000b9, "mov", WITH_rCX, WITH_IMM, 0, 
	0x000000ba, "mov", WITH_rDX, WITH_IMM, 0,
	0x000000bb, "mov", WITH_rBX, WITH_IMM, 0,
	0x000000bc, "mov", WITH_rSP, WITH_IMM, 0,
	0x000000bd, "mov", WITH_rBP, WITH_IMM, 0,
	0x000000be, "mov", WITH_rSI, WITH_IMM, 0,
	0x000000bf, "mov", WITH_rDI, WITH_IMM, 0,
	0x000000c0, NULL, WITH_MODRM|BYTE, WITH_IMM|BYTE, 0, // defined by extension
	0x000000c1, NULL, WITH_MODRM, WITH_IMM|BYTE, 0, // defined by extension 
	0x000000c2, NULL, NULL, NULL, 0, // undefined
	0x000000c3, NULL, NULL, NULL, 0, // undefined
	0x000000c4, NULL, NULL, NULL, 0, // undefined
	0x000000c5, NULL, NULL, NULL, 0, // undefined
	0x000000c6, NULL, WITH_MODRM|BYTE, WITH_IMM|BYTE, 0, // defined by extension
	0x000000c7, NULL, WITH_MODRM, WITH_IMM, 0, // defined by extension
	0x000000c8, "enter", WITH_IMM|WORD, WITH_IMM|BYTE, 0,
	0x000000c9, "leave", 0, 0, 0,
	0x000000ca, "retf", 0, WITH_IMM|WORD, 0,
	0x000000cb, "retf", 0, 0, 0,
	0x000000cc, "int 3", 0, 0, 0,
	0x000000cd, "int", 0, WITH_IMM|BYTE, 0,
	0x000000ce, "into", 0, 0, 0,
	0x000000cf, "iret", 0, 0, 0,
	0x000000d0, NULL, WITH_MODRM|BYTE, NUMBER_1, 0, // defined by extension
	0x000000d1, NULL, WITH_MODRM, NUMBER_1, 0, // defined by extension
	0x000000d2, NULL, WITH_MODRM|BYTE, WITH_CL, 0, // defined by extension 
	0x000000d3, NULL, WITH_MODRM, WITH_CL, 0, // defined by extension 
	0x000000d4, NULL, NULL, NULL, 0, // undefined
	0x000000d5, NULL, NULL, NULL, 0, // undefined
	0x000000d6, NULL, NULL, NULL, 0, // undefined
	0x000000d7, NULL, NULL, NULL, 0, // undefined
	0x000000d8, NULL, NULL, NULL, 0, // undefined
	0x000000d9, NULL, NULL, NULL, 0, // undefined
	0x000000da, NULL, NULL, NULL, 0, // undefined
	0x000000db, NULL, NULL, NULL, 0, // undefined
	0x000000dc, NULL, NULL, NULL, 0, // undefined
	0x000000dd, NULL, NULL, NULL, 0, // undefined
	0x000000de, NULL, NULL, NULL, 0, // undefined
	0x000000df, NULL, NULL, NULL, 0, // undefined
	0x000000e0, "loopne", 0, WITH_EIP|BYTE, 0, 
	0x000000e1, "loope", 0, WITH_EIP|BYTE, 0, 
	0x000000e2, "loop", 0, WITH_EIP|BYTE, 0, 
	0x000000e3, NULL, NULL, NULL, 0, // undefined
	0x000000e4, "inb", WITH_AL, WITH_IMM|BYTE, 0,
	0x000000e5, "in", WITH_eAX, WITH_IMM|BYTE, 0,
	0x000000e6, "outb", WITH_IMM|BYTE, WITH_AL, 0, 
	0x000000e7, "out", WITH_IMM|BYTE, WITH_eAX, 0,
	0x000000e8, "call", 0, WITH_EIP, 0,
	0x000000e9, "jmp", 0, WITH_EIP, 0,
	0x000000ea, "jmp", 0, WITH_DIRA, 0,
	0x000000eb, "jmp", 0, WITH_EIP|BYTE, 0,
	0x000000ec, "in", WITH_AL, WITH_DX, 0,
	0x000000ed, "in", WITH_eAX, WITH_DX, 0,
	0x000000ee, "out", WITH_DX, WITH_AL, 0,
	0x000000ef, "out", WITH_DX, WITH_eAX, 0,
	0x000000f0, NULL, 0, 0, 0, // LOCK Prefix
	0x000000f1, NULL, NULL, NULL, 0, // undefined
	0x000000f2, NULL, 0, 0, 0, // REPNE Prefix
	0x000000f3, NULL, 0, 0, 0, // REPE Prefix
	0x000000f4, "hlt", 0, 0, 0,
	0x000000f5, NULL, NULL, NULL, 0, // undefined
	0x000000f6, NULL, 0, WITH_MODRM|BYTE, 0, // defined by extension 
	0x000000f7, NULL, 0, WITH_MODRM, 0, // defined by extension 
	0x000000f8, "clc", 0, 0, 0,
	0x000000f9, "stc", 0, 0, 0,
	0x000000fa, "cli", 0, 0, 0,
	0x000000fb, "sti", 0, 0, 0,
	0x000000fc, "cld", 0, 0, 0,
	0x000000fd, "std", 0, 0, 0,
	0x000000fe, NULL, 0, WITH_MODRM|BYTE, 0, // defined by extension
	0x000000ff, NULL, 0, WITH_MODRM, 0, // defined by extension 
	-1, NULL, -1, -1, -1,
	0x00000000, NULL, 0, 0, 0, // defined by extension
	0x00000001, NULL, 0, 0, 0, // defined by extension
	0x00000002, "lar", WITH_MODRM|WITH_REG, WITH_MODRM|WORD, 0, 
	0x00000003, "lsl", WITH_MODRM|WITH_REG, WITH_MODRM|WORD, 0, 
	0x00000005, "syscall", 0, 0, 0, 
	0x00000006, "clts", 0, 0, 0, 
	0x00000007, "sysret", 0, 0, 0, 
	0x00000008, "invd", 0, 0, 0, 
	0x00000009, "wbinvd", 0, 0, 0, 
	0x000000b6, "movzx", WITH_MODRM|WITH_REG, WITH_MODRM|BYTE, 0,
	0x000000b7, "movzx", WITH_MODRM|WITH_REG, WITH_MODRM|BYTE, 0,
	-1, NULL, -1, -1, -1
};


struct op_map *op_indx_1[] = { &op_list_1[0], &op_list_1[16], &op_list_1[32], 
															 &op_list_1[48], &op_list_1[64], &op_list_1[80], 
															 &op_list_1[96], &op_list_1[112], &op_list_1[127],
															 &op_list_1[144], &op_list_1[160], 
															 &op_list_1[176], &op_list_1[192], 
															 &op_list_1[203], &op_list_1[207], 
															 &op_list_1[224], &op_list_1[239] };
