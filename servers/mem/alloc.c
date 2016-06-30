#define LEFT 1
#define RIGHT 2
#define FALSE 0
#define TRUE 1
#include "vm.h"
extern struct mem_abs *ptr_heaps[6];
static usize_t heap_sizes[6];
static uint_t heaps_first[6]={0};

static void
coalesce(struct mem_abs *ptr_hp, uint_t dir_coa)
{
	struct mem_abs *ptr_heap1;
	struct mem_abs *ptr_heap2;

	ptr_heap1 = ptr_hp->prev_blks;
	ptr_heap2 = ptr_hp->next_blks;
	switch (dir_coa) {
		case LEFT: 
			ptr_heap1->mem_size+=ptr_hp->mem_size+sizeof(struct mem_abs);
			ptr_heap1->next_blks = ptr_heap2; // adjust whole-list linked list
			ptr_heap2->prev_blks = ptr_heap1; // adjust whole-list linked list
			break;
		case RIGHT:
			/* coalesce freeing block and next block into the previous free block */
			ptr_hp->mem_size+=ptr_heap2->mem_size+sizeof(struct mem_abs);
			ptr_hp->next_blks=ptr_heap2->next_blks;
			break;
	}
}

/*
 * Free core by pages.
 */
void
kfree_page(ubyte_t map_num, struct mem_abs *free_addr)
{
	struct mem_abs *ptr_heap;
	struct mem_abs *ptr_heap1;
	struct mem_abs *ptr_heap2;
	uaddr_t addr_tmp;
	byte *ptr_byte;
	uint_t m;
	void *ret_addr;

	for (ptr_heap=ptr_heaps[map_num]; ptr_heap->mem_addr!=(uaddr_t)free_addr;
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
kmalloc_page(ubyte_t map_num, uint_t alloc_size)
{
	uint_t m, i;
	struct mem_abs *ptr_heap;
	struct mem_abs *ptr_heap1;
	struct mem_abs *ptr_heap2;
	uaddr_t addr_tmp;
	void *ret_addr;
	byte *ptr_byte;

	/*
	 * Find a suitable block. For simplicity, algorithm is first-fit. Be free
	 * to come up with a better memory-allocation system.
	 */
	for (ptr_heap=ptr_heaps[map_num]; 
								(ptr_heap->mem_size<alloc_size && ptr_heap!=NULL) 
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
		ret_addr=(void *)ptr_heap->mem_addr; // address allocated
		/* 
		 * Must we create a new control block past the allocation?
		 * This is in case the block is larger than the requesting value.
		 */
		if (ptr_heap->mem_size>alloc_size) {
			addr_tmp = (uaddr_t)(ptr_heap->mem_addr); // allocation address
			ptr_byte = (byte *)addr_tmp;
			addr_tmp += alloc_size*PAGE_SIZE; // plus allocation size
			for (i=0; i<alloc_size*PAGE_SIZE; i++)
				*ptr_byte++=0;
			/* XXX hack executed once per map allocation */
			if (heaps_first[map_num] == 0) {
				heaps_first[map_num]=1;
				heap_sizes[map_num]=ptr_heap->mem_size;
			}
			/* create a new control block for the remaining of space */
			ptr_heap2=(struct mem_abs *)addr_tmp; // point to next CB
			/* allocation address for new block */
			ptr_heap2->mem_addr=((uaddr_t)ptr_heap2)+sizeof(struct mem_abs);
			/* size for new block */
			ptr_heap2->mem_size=ptr_heap->mem_size-alloc_size-
													sizeof(struct mem_abs);
			ptr_heap2->next_blks = ptr_heap->next_blks; // link blocks
			ptr_heap2->prev_blks = ptr_heap; // ...
			ptr_heap2->mem_inuse=FALSE;
			ptr_heap->next_blks = ptr_heap2; // ...
			ptr_heap->mem_size=alloc_size; // allocated blocks new size
		}
	}
	return ret_addr;
}
