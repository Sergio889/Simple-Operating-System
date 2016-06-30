#ifndef COMMON
#define COMMON
#include "../../../libcommon/com.h"
#endif
#define NMEMINTOPS 10
#define PAGS_4K 0x1
#define PAGS_2M 0x2
#define POOL 0x1
#define HEAP 0x2

/*
 * This template models all the activity that can happen in the
 * system.
 * It handles the mechanisms for managing the physical memory
 * One instance per memory unit (page, or whatever).
 */
struct mem_abs {
	uaddr_t mem_addr;
	uaddr_t mem_size; // contigious blocks in 32-byte blocks
	byte mem_inuse;
	byte mem_last;
	/* whole-list linked-list */
	struct mem_abs *next_blks;
	struct mem_abs *prev_blks;
};
