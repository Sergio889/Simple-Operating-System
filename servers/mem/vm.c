#include "vm.h"
extern struct mem_abs *ptr_heaps[6];
static usize_t heap_sizes[6];
static uint_t heaps_first[6]={0};
#define LEFT 1
#define RIGHT 2
#define FALSE 0
#define TRUE 1

int
etm_mem(int msg_ent)
{
	struct com_fmt *ptr_in;
	struct com_fmt *ptr_out;
	uint_t i, m;
	uint_t entry;
	byte buffer[512];

	/* 
	 * First pass to general receive routine.
	 * The if will be taken if the message is an ACK.
	 * Else the message is a request.
	 */
	ptr_in=(struct com_fmt *)receive(MEM_ENT, msg_ent, pool_in_out[MEM_IN]);
	ptr_out = pool_in_out[MEM_OUT]; // point to outbox
	ptr_out += ptr_in->ent; // add offset to outbox entry
	if ((ptr_in->payld.flags & (ACK | RESPONSE)) == (ACK | RESPONSE))
	{
//#ifdef VERBOSE
		kprintf("Memory Server -> received reply at inbox entry %x\n", &msg_ent, 
						buffer);
//#endif
		invalidate(ptr_in);
		invalidate(ptr_out);
	}
	else {
//#ifdef VERBOSE
		kprintf("Memory Server -> received request \n%x\n", ptr_in, buffer);
//#endif
		ptr_out=pool_in_out[MEM_OUT], i=0;
		if (ptr_out->src_port != 0) {
			for (;(ptr_out)->src_port!=0; ptr_out++, i++)
				;
		}
		switch (ptr_in->payld.cmd) {
			case ALLOC:
				ptr_out->payld.cmd = NONE;
				ptr_out->ent = i;
				if ((ptr_in->payld.flags & PROC_POOL) == PROC_POOL)
					entry = PROC_ENT;
				else if ((ptr_in->payld.flags & FS_POOL) == FS_POOL)
					entry = FS_ENT;
				else if ((ptr_in->payld.flags & IO_POOL) == IO_POOL)
					entry = IO_ENT;
				ptr_out->payld.ptr_smwh = kmalloc_page(entry+1, ptr_in->payld.number);
				ptr_out->src_port = 100;
				break;
		}
//#ifdef VERBOSE
		kprintf("Memory Server -> replied at %x\n", &ptr_out, buffer);
		kprintf("\n%x\n", ptr_out, buffer);
//#endif
		invalidate((byte *)ptr_in);
	}
	return i;
}
