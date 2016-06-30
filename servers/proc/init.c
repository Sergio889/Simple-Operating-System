#include "proc.h"
extern int etm_proc(int);
extern struct proc *ptr_pool;

void *
init(uaddr_t *mail_addr)
{
	void *addr_ptr;
	struct com_fmt *ptr_tmp;
	uint_t i;

	/* save mail boxes */
	pool_in_out[PROC_IN] = (struct com_fmt *)mail_addr; // in mail
	pool_in_out[PROC_OUT]=(struct com_fmt *)(mail_addr+(4096/sizeof(uaddr_t *)));

	/* write first message to outbox */
	ptr_tmp=pool_in_out[PROC_OUT];
	ptr_tmp->unit_size = sizeof(struct proc);
	ptr_tmp->src_port = 100; // temporal for 'entry is used'
	ptr_tmp->payld.cmd = ALLOC; // command
	ptr_tmp->payld.flags = PAGS4K; // request in 4K page units
	ptr_tmp->payld.number = 10; // XXX arbitrary
	ptr_tmp->payld.ptr_smwh = (void *)&ptr_pool;
	ptr_tmp->ent = 0; // XXX arbitrary
//#ifdef VERBOSE
	low_putstr("Process Server -> new request at outbox mail 0x00000000\n");
	low_putstr("Process Server -> new entry point at ");
//#endif
	addr_ptr = etm_proc;
//#ifdef VERBOSE
	for (i=0; i<512; i++)
		buffer[i] = 0;
	kprintf("%x\n", &addr_ptr, buffer);
	low_putstr("Process Server -> Processes? threads? up and running...\n");
//#endif

	/* return new entry point */
	return etm_proc;
}
