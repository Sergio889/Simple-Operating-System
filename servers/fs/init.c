#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#include "fs.h"
extern int etm_fs(int);
extern struct pool_mgmt *ptr_pool;
extern struct pool_mgmt *ptr_free;

void *
init(uaddr_t *mail_addr)
{
	void *ptr_addr;
	struct com_fmt *ptr_tmp;
	uint_t i;

	pool_in_out[FS_IN] = (struct com_fmt *)mail_addr;
	pool_in_out[FS_OUT] = (struct com_fmt *)(mail_addr+(4096/sizeof(uaddr_t *)));
	/* write first message to outbox */
	ptr_tmp=pool_in_out[FS_OUT];
	ptr_tmp->unit_size = sizeof(struct fs);
	ptr_tmp->src_port = 100; // temporal for 'entry is used'
	ptr_tmp->payld.cmd = ALLOC; // command
	ptr_tmp->payld.flags = PAGS4K; // request in 4K page units
	ptr_tmp->payld.number = 10; // XXX arbitrary
	ptr_tmp->payld.ptr_smwh = (void *)&ptr_pool;
	ptr_tmp->ent = 0; // XXX arbitrary
	ptr_addr = etm_fs;
//#ifdef VERBOSE
	low_putstr("FS Server -> new request at inbox mail 0x00000000\n");
	low_putstr("FS Server -> new entry point at address ");
	kprintf("%x\n", &ptr_addr, buffer);
	low_putstr("FS Server -> File-system code up and running...\n");
//#endif

	return etm_fs;
}
