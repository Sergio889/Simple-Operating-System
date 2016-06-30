#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#define INIT_MAPS 0x1400000
#define PDIR_BASE 0x1400000
#define PTAB_BASE 0x1401000

void *
mmap_ident(uaddr_t map_base, uint_t map_len)
{
	uaddr_t *map_pd, *map_pt, map_cpy;
	uint_t i;

	map_pd = (uaddr_t *)(PDIR_BASE+(((map_base>>22)&0x3ff)*4));
	map_pt = (uaddr_t *)(PTAB_BASE+(((map_base>>12)&0x3ff)+(4*1024)));

	*map_pd = ((uaddr_t)map_pt)|0xb;

	for (i=0, map_cpy=(uaddr_t)map_base; i<map_len; i++)
		*map_pt = map_cpy|0xb, map_cpy+=4096;

	return (void *)map_base;
}
