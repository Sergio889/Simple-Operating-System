#include "heap.h"
static struct sc_hp *ptr_list=-1;

static void
coalesce(struct sc_hp *ptr_hp, uint_t dir_coa)
{
	struct sc_hp *ptr_heap1;
	struct sc_hp *ptr_heap2;

	ptr_heap1 = ptr_hp->prev_blks;
	ptr_heap2 = ptr_hp->next_blks;
	switch (dir_coa) {
		case LEFT: 
			ptr_heap1->mem_size+=ptr_hp->mem_size+sizeof(struct sc_hp);
			ptr_heap1->next_blks = ptr_heap2; // adjust whole-list linked list
			if (ptr_heap2!=NULL)
				ptr_heap2->prev_blks = ptr_heap1; // adjust whole-list linked list
			break;
		case RIGHT:
			/* coalesce freeing block and next block into the previous free block */
			ptr_hp->next_blks=ptr_heap2->next_blks;
			ptr_hp->mem_size+=ptr_heap2->mem_size+sizeof(struct sc_hp);
			if (ptr_heap2->next_blks!=NULL)
				ptr_heap2->next_blks->prev_blks=ptr_hp;
			break;
	}
}

void
heap_alloc(uint_t alloc_size, uint_t *addr_alloc)
{
	struct sc_hp *ptr_heap;
	struct sc_hp *ptr_heap1;
	struct sc_hp *ptr_heap2;
	uaddr_t addr_tmp;
	void *ret_addr;
	byte buffer[512];
	uint_t i, m;
	byte *ptr_byte;

	/* 
	 * Find a suitable block. For simplicity, algorithm is first-fit. Be free
	 * to come up with a better memory-allocation system.
	 */
	for (ptr_heap=ptr_list; (ptr_heap->mem_size<alloc_size && ptr_heap!=NULL) 
								|| ptr_heap->mem_inuse==TRUE; ptr_heap=ptr_heap->next_blks)
		;
	/* couldn't find one! */
	if (ptr_heap==NULL)
		*addr_alloc=NULL;
	/* we have a block that suits the request */
	else {
		ptr_heap1=ptr_heap->prev_blks; // pointer to previous node
		ptr_heap2=ptr_heap->next_blks; // pointer to next node
		ptr_heap->mem_inuse=TRUE; // memory is now in use
		*addr_alloc=ptr_heap->mem_addr; // address allocated
		/* 
		 * Must we create a new control block past the allocation?
		 * This is in case the block is larger than the requesting value.
		 */
		if (ptr_heap->mem_size>alloc_size) {
			addr_tmp = ptr_heap->mem_addr; // allocation address
			ptr_byte=(byte *)addr_tmp;
			for (m=0; m<alloc_size*ALLOC_SIZE; m++)
				*ptr_byte++=0;
			addr_tmp += alloc_size*ALLOC_SIZE; // plus allocation size
			/* create a new control block for the remaining of space */
			ptr_heap2=(struct sc_hp *)addr_tmp; // point to next CB
			/* allocation address for new block */
			ptr_heap2->mem_addr=((uaddr_t)ptr_heap2)+sizeof(struct sc_hp);
			/* size for new block */
			ptr_heap2->mem_size=ptr_heap->mem_size-alloc_size-sizeof(struct sc_hp);
			ptr_heap2->next_blks = ptr_heap->next_blks; // link blocks
			ptr_heap2->prev_blks = ptr_heap; // ...
			ptr_heap2->mem_inuse=FALSE;
			ptr_heap->next_blks = ptr_heap2; // ...
			ptr_heap->mem_size=alloc_size; // allocated blocks new size
		}
	}
}

void
heap_free(void *free_addr)
{
	struct sc_hp *ptr_heap;
	struct sc_hp *ptr_heap1;
	struct sc_hp *ptr_heap2;
	uaddr_t addr_tmp;
	byte *ptr_byte;
	uint_t m;
	void *ret_addr;

	for (ptr_heap=ptr_list; (void *)ptr_heap->mem_addr!=free_addr &&
													ptr_heap->next_blks!=NULL; 
													ptr_heap=ptr_heap->next_blks)
		;
	if (ptr_heap->mem_addr != (uaddr_t)free_addr || ptr_heap->mem_inuse == FALSE)
	{
		for (m=0; m<512; m++)
			buffer[m]=0;
		low_putstr("\n");
		memdump32((int *)&free_addr, buffer, 10, 4, 1);
		low_putstr("Error freeing memory Second:\n ");
		low_putstr(buffer);
	}
	else {
		ptr_heap->mem_inuse=FALSE;
		ptr_heap1 = ptr_heap->prev_blks;
		ptr_heap2 = ptr_heap->next_blks;
		/* case 1: at middle of nodes */
		if (ptr_heap1 != NULL) {
			if (ptr_heap1->mem_inuse==FALSE)
			/* can we coalesce to the left? */
				coalesce(ptr_heap, LEFT);
		}
		if (ptr_heap2 != NULL && ptr_heap2->mem_inuse==FALSE) {
				/* coalesce freeing block and next block into the previous free block */
				coalesce(ptr_heap, RIGHT);
		}
	}
}

/*
 * Initialize the heap of second.
 */
void
heap_init()
{
	void *test_tmp;
	void *test_tmp1;
	void *test_tmp2;
	void *test_tmp3;
	byte buffer[512];
	uint_t i;
	/* head of the heap */
	ptr_list=(struct sc_hp *)HEAP_START;
	/* first block of allocation */
	ptr_list->mem_addr=((uaddr_t)ptr_list)+((uaddr_t)sizeof(struct sc_hp));
	/* it's size in ALLOC_SIZE units */
	ptr_list->mem_size=(((((uaddr_t)HEAP_END) - ((uaddr_t)ptr_list)) - 
											sizeof(struct sc_hp))) / ALLOC_SIZE;
	ptr_list->next_blks = NULL;
	ptr_list->prev_blks = NULL;
	ptr_list->mem_inuse = FALSE;
}
