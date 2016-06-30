#include "defs/vm.h"

/* global variables */
extern struct mem_map_indpt *ptr_list_indpt;
extern struct kmon_servs_maps kms_maps[7];

/* private variables */
struct mem_abs *ptr_heaps[6] = { NULL };

/*
 * Initialize heaps.
 */
void
heap_init()
{
	/* address variables */
	uaddr_t heap_cpy; // control to traverse the heaps
	struct mem_abs *ptr_tmp; // temporal pointer to traverse heaps
	struct mem_abs *ptr_tmp1; // temporal pointer to traverse heaps
	struct mem_map_indpt *ptr_lst_tmp; // temporal pointer to traverse map list

	uint_t addr_tmp;

	/* control variables */
	uint_t i, m, n, o;

	low_putstr("\n");
#ifdef VERBOSE
	low_putstr("Kernel, user-level servers and user-level applications heaps\n");
	low_putstr("======================================================\n");
#endif
	for (i=0; i<6; i++) {
		if (i<5) {
			heap_cpy = kms_maps[i].base_addr+kms_maps[i].len_bytes;
			/* head of heap alligned to page boundary */
			ptr_heaps[i] = (struct mem_abs *)(((heap_cpy+4095)&(-4096)));
			/* allocation address XXX is not page-aligned! */
			ptr_heaps[i]->mem_addr=((uaddr_t)ptr_heaps[i])+
														 ((uaddr_t)sizeof(struct mem_abs));
			/* size of allocation */
			ptr_heaps[i]->mem_size=((kms_maps[i+1].base_addr-((uaddr_t)ptr_heaps[i]))
														 -sizeof(struct mem_abs))/PAGE_SIZE;
			ptr_heaps[i]->mem_inuse = FALSE;
		}
		else {
			heap_cpy = kms_maps[i].base_addr+kms_maps[i].len_bytes;
			/* head of heap alligned to page boundary */
			ptr_heaps[i] = (struct mem_abs *)(((heap_cpy+4095)&(-4096)));
			/* allocation address */
			ptr_heaps[i]->mem_addr=((uaddr_t)ptr_heaps[i])+
														 ((uaddr_t)sizeof(struct mem_abs));
			/* size of allocation XXX quite innacurate; smaller than actual size */
			//ptr_heaps[i]->mem_size=((((ptr_lst_tmp->base_addr[0]+
			//											 ptr_lst_tmp->addr_len[0])-((4<<20)*4))
			//											 -sizeof(struct mem_abs))/PAGE_SIZE);
		}
		ptr_heaps[i]->next_blks=NULL;
		ptr_heaps[i]->prev_blks=NULL;
	}
	
#ifdef VERBOSE
	low_putstr("======================================================\n");
#endif
}
