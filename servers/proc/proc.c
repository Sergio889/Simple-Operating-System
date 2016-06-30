#include "proc.h"
#include "../../libcommon/pools.h"

struct pool_mgmt *ptr_pool;
struct pool_mgmt *ptr_free;

static void
pool_procs_init(struct pool_mgmt *ptr_pool, uint_t num_pags, ushort_t num_ent, 
							 ushort_t size)
{
	uint_t m;
	struct pool_mgmt **ptr_map;

	ptr_map = (void *)&proc_mem;
	/* get start of free dev structures */
	*ptr_map=(struct pool_mgmt *)init_pool((void *)ptr_pool, num_pags, size);
	proc_mem.granularities[num_ent] = (uint_t)size;
}

int
etm_proc(int msg_ent)
{
	struct com_fmt *ptr_in;
	struct com_fmt *ptr_out;
	uint_t i, m;
	uint_t in_entry;

	/* 
	 * First pass to general receive routine.
	 * The if will be taken if the message is an ACK.
	 * Else the message is a request.
	 */
	ptr_in=(struct com_fmt *)receive(PROC_ENT, msg_ent, pool_in_out[PROC_IN]);
	ptr_out = pool_in_out[PROC_OUT]; // point to outbox
	ptr_out += ptr_in->ent; // add offset to outbox entry
	if ((ptr_in->payld.flags & (ACK | RESPONSE)) == (ACK|RESPONSE))
	{
//#ifdef VERBOSE
		kprintf("Process Server -> received reply at inbox entry %d\n",&msg_ent,
						buffer);
//#endif
		switch (ptr_in->payld.cmd) {
			case NONE:
				break;
			case ALLOC:
				*(ptr_out->payld.ptr_smwh) = ptr_in->payld.ptr_smwh;
				pool_procs_init((struct pool_mgmt *)(*(ptr_out->payld.ptr_smwh)),
											 ptr_out->payld.number, ptr_out->ent, ptr_out->unit_size);
				break;

		}
		invalidate(ptr_out); // invalidate request entry
		invalidate(ptr_in); // invalidate received-message entry
	}
	else {
//#ifdef VERBOSE
		kprintf("Process Server -> received request \n%x\n", ptr_in, buffer);
//#endif
		while (1)
			;
		ptr_out=pool_in_out[PROC_OUT], i=0;
		if (ptr_out->src_port != 0) {
			for (;(ptr_out+1)->src_port!=0; ptr_out++, i++)
				;
			ptr_out++;
		}
		switch (ptr_in->payld.cmd) {
		}
		ptr_in->src_port = 0; // invalidate entry
	}
}
