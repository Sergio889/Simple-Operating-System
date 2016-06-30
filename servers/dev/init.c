#include "dev.h"
extern int etm_io(int);

void *
init(uaddr_t *mail_addr)
{
	/* address variables */
	void *addr_ptr;
	struct com_fmt *ptr_req1; // request device structures
	struct com_fmt *ptr_req2; // request device mappings
	struct com_fmt *ptr_req3; // request device mappings

	/* control variables */
	uint_t i;

	/* save mail boxes */
	pool_in_out[IO_IN] = (struct com_fmt *)mail_addr; // in mail
	pool_in_out[IO_OUT] = (struct com_fmt *)(mail_addr+(4096/sizeof(uaddr_t *)));

	/* 
	 * Write first message to outbox.
	 * Request memory for memory arena 1.
	 * Used for allocating device structures.
	 * Granularity 32-bytes.
	 */
	ptr_req1=pool_in_out[IO_OUT]; // beginning of outbox mail
	ptr_req1->src_port = 100; // temporal for 'entry is used'
	ptr_req1->unit_size = sizeof(struct dev);//(ushort_t )sizeof(struct dev);
	ptr_req1->payld.cmd = ALLOC; // command
	ptr_req1->payld.flags = PAGS4K; // request in 4K page units
	ptr_req1->payld.number = 10; // number of units to request
	ptr_req1->payld.ptr_smwh = (void *)(&dev_mem.ptr_pool_devs);
	ptr_req1->ent = 0; // first entry
//#ifdef VERBOSE
	kprintf("IO Server -> new request at outbox mail 0\n");
//#endif
	/* 
	 * Write second message to outbox.
	 * Request memory for memory arena 2.
	 * Used for allocating device mapping at memory address space.
	 * Granularity 32-bytes.
	 */
	ptr_req2=ptr_req1+1; // next outbox mail entry
	ptr_req2->src_port = 100; // temporal for 'entry is used'
	ptr_req2->unit_size = 32; // temporal for 'entry is used'
	ptr_req2->payld.cmd = ALLOC; // command
	ptr_req2->payld.flags = PAGS4K; // request in 4K page units
	ptr_req2->payld.number = 100; // number of units to request
	ptr_req2->payld.ptr_smwh = (void *)(&dev_mem.ptr_pool_maps);
	ptr_req2->ent = 1; // second entry
//#ifdef VERBOSE
	kprintf("IO Server -> new request at outbox mail 1\n");
//#endif
	/* 
	 * Write third message to outbox.
	 * Request memory for memory arena 3.
	 * Various uses.
	 * Granularity 64-bytes.
	 */
	ptr_req3=ptr_req2+1; // next outbox mail entry
	ptr_req3->src_port = 100; // temporal for 'entry is used'
	ptr_req3->unit_size = 64; // temporal for 'entry is used'
	ptr_req3->payld.cmd = ALLOC; // command
	ptr_req3->payld.flags = PAGS4K; // request in 4K page units
	ptr_req3->payld.number = 10; // number of units to request
	ptr_req3->payld.ptr_smwh = (void *)(&dev_mem.ptr_pool_others);
	ptr_req3->ent = 2; // third entry
	/* be verbose */
//#ifdef VERBOSE
	kprintf("IO Server -> new request at outbox mail 2\n");
	kprintf("IO Server -> new entry point at ");
//#endif
	addr_ptr = etm_io; // return value
//#ifdef VERBOSE
	/* be verbose */
	kprintf("%x\nIO Server -> up and running...\n", &addr_ptr);
//#endif

	/* return new entry point */
	return etm_io;
}

/*
 * For probing and initializing devices, we assume PCI as the local bus.
 */
void
init_devs()
{
	pci_conf();
}
