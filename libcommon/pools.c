#ifndef COMMON
#define COMMON
#include "com.h"
#endif
#define PAGE_SHIFT 0xc
#include "pools.h"
#define TRUE 1
#define FALSE 0
#define RIGHT 1
#define LEFT 2

static void
coalesce(struct pool_mgmt *ptr_hp, uint_t dir_coa)
{
	struct pool_mgmt *ptr_heap1;
	struct pool_mgmt *ptr_heap2;

	ptr_heap1 = ptr_hp->prev_blks;
	ptr_heap2 = ptr_hp->next_blks;
	switch (dir_coa) {
		case LEFT: 
			ptr_heap1->mem_size+=ptr_hp->mem_size+sizeof(struct pool_mgmt);
			ptr_heap1->next_blks = ptr_heap2; // adjust whole-list linked list
			ptr_heap2->prev_blks = ptr_heap1; // adjust whole-list linked list
			break;
		case RIGHT:
			/* coalesce freeing block and next block into the previous free block */
			ptr_heap1->mem_size+=ptr_heap2->mem_size+sizeof(struct pool_mgmt);
			ptr_heap1->next_blks=ptr_heap2->next_blks;
			ptr_heap2->next_blks->prev_blks=ptr_heap1;
			break;
	}
}

void
pool_free(struct pool_mgmt *pool_map, struct pool_mgmt *free_addr)
{
	struct pool_mgmt *ptr_heap;
	struct pool_mgmt *ptr_heap1;
	struct pool_mgmt *ptr_heap2;
	uaddr_t addr_tmp;
	byte *ptr_byte;
	uint_t m;
	void *ret_addr;

	for (ptr_heap=pool_map; ptr_heap->mem_addr!=(uaddr_t)free_addr; 
								ptr_heap=ptr_heap->next_blks)
		;
	if (ptr_heap->mem_addr != (uaddr_t)free_addr || ptr_heap->mem_inuse == FALSE)
		low_putstr("Error freeing memory\n");
	else {
		ptr_heap->mem_inuse=FALSE;
		ptr_heap1 = ptr_heap->prev_blks;
		ptr_heap2 = ptr_heap->next_blks;
		/* case 1: at middle of nodes */
		if (ptr_heap1 != NULL && ptr_heap1->mem_inuse==FALSE) {
			/* can we coalesce to the left? */
			if (ptr_heap1->mem_inuse==FALSE)
				coalesce(ptr_heap, LEFT);
		}
		if (ptr_heap2 != NULL && ptr_heap2->mem_inuse==FALSE) {
			/* can we coalesce to the right? */
			if (ptr_heap2->mem_inuse==FALSE) {
				/* coalesce freeing block and next block into the previous free block */
				coalesce(ptr_heap, RIGHT);
			}
		}
	}
}

/*
 * Allocate core by pages.
 */
void *
pool_alloc(struct pool_mgmt *ptr_map, uint_t alloc_size, uint_t unit_size)
{
	struct pool_mgmt *ptr_heap;
	struct pool_mgmt *ptr_heap1;
	struct pool_mgmt *ptr_heap2;
	uaddr_t addr_tmp;
	void *ret_addr;
	byte buffer[512];
	uint_t i, m;
	byte *ptr_byte;

	/* 
	 * Find a suitable block. For simplicity, algorithm is first-fit. Be free
	 * to come up with a better memory-allocation system.
	 */
	for (ptr_heap=ptr_map; (ptr_heap->mem_size<alloc_size && ptr_heap!=NULL) 
								|| ptr_heap->mem_inuse==TRUE; ptr_heap=ptr_heap->next_blks)
		;

	/* couldn't find one! */
	if (ptr_heap==NULL)
		ret_addr=NULL;
	/* we have a block that suits the request */
	else {
		ptr_heap1=ptr_heap->prev_blks; // pointer to previous node
		ptr_heap2=ptr_heap->next_blks; // pointer to next node
		ptr_heap->mem_inuse=TRUE; // memory is now in use
		ret_addr=(void *)(ptr_heap->mem_addr); // address allocated
		/* 
		 * Must we create a new control block past the allocation?
		 * This is in case the block is larger than the requesting value.
		 * XXX will allocated forever!
		 */
		if (ptr_heap->mem_size>alloc_size) {
			addr_tmp = ptr_heap->mem_addr; // allocation address
			ptr_byte=(byte *)addr_tmp;
			for (m=0; m<alloc_size*unit_size; m++)
				*ptr_byte++=0;
			addr_tmp += alloc_size*unit_size; // plus allocation size
			/* create a new control block for the remaining of space */
			ptr_heap2=(struct pool_mgmt *)addr_tmp; // point to next CB
			/* allocation address for new block */
			ptr_heap2->mem_addr=((uaddr_t)ptr_heap2)+sizeof(struct pool_mgmt);
			/* size for new block */
			ptr_heap2->mem_size=ptr_heap->mem_size-alloc_size-sizeof(struct pool_mgmt);
			ptr_heap2->next_blks = ptr_heap->next_blks; // link blocks
			ptr_heap2->prev_blks = ptr_heap; // ...
			ptr_heap2->mem_inuse=FALSE;
			ptr_heap->next_blks = ptr_heap2; // ...
			ptr_heap->mem_size=alloc_size; // allocated blocks new size
		}
	}

	return ret_addr;
}

void *
init_pool(struct pool_mgmt *beg_addr, uint_t num_pags, uint_t strc_size)
{
	struct pool_mgmt *ptr_list;
	byte buffer[512];
	uint_t i;

	/* head of the heap */
	ptr_list=beg_addr;
	/* first block of allocation */
	ptr_list->mem_addr=((uaddr_t)ptr_list)+((uaddr_t)sizeof(struct pool_mgmt));
	/* it's size in strc_size units */
	ptr_list->mem_size=(((((uaddr_t)beg_addr+(num_pags<<PAGE_SHIFT))-
										 ((uaddr_t)ptr_list))-sizeof(struct pool_mgmt))) 
										 / strc_size;
	ptr_list->next_blks = NULL;
	ptr_list->prev_blks = NULL;
	ptr_list->mem_inuse = FALSE;

	return (void *)ptr_list;
}
