#include "../../libcommon/com.h"
#include "../../libcommon/elf32.h"
#include "../../libcommon/SOSfs.h"
#include "../../libcommon/bios.h"
#define NEXCPS 20 // Intel exceptions

byte IDT[512]; // space for the Interrupt Descriptor Table

struct idt_desc {
	ushort_t idt_off1;
	ushort_t idt_seg;
	ushort_t idt_ctrl;
	ushort_t idt_off2;
} idt_desc;
