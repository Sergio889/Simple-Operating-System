#ifndef COMMON
#define COMMON
#include "com.h"
#endif

struct pool_mgmt {
	uaddr_t mem_addr;
	uaddr_t mem_size; // contigious blocks in 32-byte blocks
	/* whole-list linked-list */
	struct pool_mgmt *next_blks;
	struct pool_mgmt *prev_blks;
	byte mem_inuse;
	byte mem_last;
};
