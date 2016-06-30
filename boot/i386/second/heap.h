#ifndef COMMON
#define COMMON
#include "term.h"

struct sc_hp {
	uaddr_t mem_addr;
	uaddr_t mem_size; // contigious blocks in 32-byte blocks
	byte mem_inuse;
	byte mem_last;
	/* whole-list linked-list */
	struct sc_hp *next_blks;
	struct sc_hp *prev_blks;
};
#endif
#define HEAP_START 0x100000
#define HEAP_END 0x1f0000
#define ALLOC_SIZE 64
