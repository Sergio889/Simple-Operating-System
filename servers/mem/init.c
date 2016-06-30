#include "vm.h"
struct mem_abs *ptr_heaps[6] = { NULL };
extern int etm_mem(int);

void *
init(uaddr_t *heaps_addr)
{
	uint_t i;
	struct com_fmt *ptr_out;
	void *ptr_mails;
	int *temp_zero;
	byte buffer[512];

	for (i=0; i<5; i++)
		ptr_heaps[i] = (struct mem_abs *)heaps_addr[i];

	/* allocate pages for communication channels */
	ptr_mails = (void *)kmalloc_page(KERN_ENT, 4*2);
	if (ptr_mails==NULL) {
		low_putstr("Failed allocating");
		while (1)
			;
	}
//#ifdef VERBOSE
	low_putstr("Memory Server -> Communication channels mapped to address ");
	kprintf("%x\n", &ptr_mails);
	low_putstr("Memory Server -> VM server up and running...\n");
//#endif

	/* save mail boxes for memory server */
	pool_in_out[MEM_IN] = (struct com_fmt *)ptr_mails;
	pool_in_out[MEM_OUT] = (struct com_fmt *)(ptr_mails+4096);
	ptr_out = pool_in_out[MEM_OUT];
	ptr_out->ent = 0;
	ptr_out->payld.cmd = NEW_ENTRY;
	ptr_out->src_port = 100;
	ptr_out->payld.ptr_smwh = (void *)etm_mem;
//#ifdef VERBOSE
	low_putstr("Memory Server -> new request in outbox mail\n");
//#endif

	return ptr_mails;
}
