/*
 * Debugging routines.
 */
#ifndef COMMON
#define COMMON
#include "string.h"

static char *dump_format[] = { "cr0 = 0x", "cr3 = 0x", "cr4 = 0x", "eax = 0x", "ebx = 0x", "ecx = 0x", "edx = 0x", "esi 0x", "edi = 0x", "ebp = 0x", "esp = 0x", "cs = 0x", "ds = 0x", "es = 0x", "ss = 0x", "gs = 0x", "fs = 0x",  };
static char *excp_dump_format[] = { "cs = 0x", "eip = 0x", "errcode = 0x\n" };
static char *excp_vectors[] = { "Division Exception", "Debug Exception", "NMI Exception", "Breakpoint Exception", "Overflow Exception", "Bound Exception", "Invalid Opcode Exception", "Device Exception", "Double Fault Exception", "Coprocessor Exception", "Invalid TSS Exception", "Segment Not Present Exception", "Stack Fault Exception", "General Protection Exception", "Page Exception" };
static char *misc_str = "=============================================\n";


/*
 * Dumping registers.
 */
#define FALSE 0
#define REG_NUM_DUMP 17
#define EXCP_NUM_DUMP 3
#endif
