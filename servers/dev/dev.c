#include "dev.h"
#include "../../libcommon/pools.h"

static ubyte_t num_inits=0;

static void
pool_devs_init(struct pool_mgmt *ptr_pool, uint_t num_pags, ushort_t num_ent, 
							 ushort_t size)
{
	uint_t m;
	struct pool_mgmt **ptr_map;

	ptr_map = (void *)&dev_mem;
	ptr_map += num_ent;
	/* get start of free dev structures */
	*ptr_map=(struct pool_mgmt *)init_pool((void *)ptr_pool, 
														 num_pags, size);
	dev_mem.granularities[num_ent] = (uint_t)size;
}

int
etm_io(int mail_num)
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
	ptr_in=(struct com_fmt *)receive(IO_ENT, mail_num, pool_in_out[IO_IN]);
	ptr_out = pool_in_out[IO_OUT]; // point to outbox
	ptr_out += ptr_in->ent; // add offset to outbox entry
	if ((ptr_in->payld.flags & (ACK | RESPONSE)) == (ACK|RESPONSE))
	{
//#ifdef VERBOSE
		kprintf("IO Server -> received reply at inbox entry %d\n",&mail_num);
//#endif
		switch (ptr_in->payld.cmd) {
			case NONE:
				break;
			case ALLOC:
				*(ptr_out->payld.ptr_smwh) = ptr_in->payld.ptr_smwh;
				pool_devs_init((struct pool_mgmt *)(*(ptr_out->payld.ptr_smwh)),
											ptr_out->payld.number, ptr_out->ent, ptr_out->unit_size);
				num_inits++;
				if (num_inits == NUM_INIT_ALLOCS) {
					low_cpuid(); // get cpu information
					//kprintf("\nLooking for PnP BIOS...");
					//kprintf("(a hell of a headache interface!)\n");
					//kprintf("============================");
					//kprintf("==========================\n");
					//pnp_bios();
					kprintf("\nLooking for SMBIOS...\n");
					kprintf("============================");
					kprintf("==========================\n");
					smbios();
					init_devs(); // configure devices
				}
				break;

		}
		invalidate(ptr_out); // invalidate request entry
		invalidate(ptr_in); // invalidate received-message entry
	}
	else {
//#ifdef VERBOSE
		kprintf("IO Server -> received request at %x\n", &mail_num);
		kprintf("%x\n", ptr_in);
//#endif
		ptr_out=pool_in_out[IO_OUT], i=0;
		if (ptr_out->src_port != 0) {
			for (;(ptr_out+1)->src_port!=0; ptr_out++, i++)
				;
			ptr_out++;
		}
		switch (ptr_in->payld.cmd) {
		}
//#ifdef VERBOSE
		kprintf("IO Server -> replied at %x\n", &ptr_out);
		kprintf("%x\n", ptr_out);
//#endif
		invalidate((byte *)ptr_in);
	}
}
