#include "defs/proc.h"
#include "defs/vm.h"
#include "templates/PROC/proc_abs.h"

/* external variables */
extern struct mem_abs *ptr_heaps[6];
extern struct mem_abs *ptr_fpags[6];
extern struct mem_abs *ptr_upags[6];
extern struct mem_map_indpt *ptr_list_indpt;
extern uaddr_t (*ent_pts[4])(int); // servers new entry points
extern struct com_fmt *ptr_mails[4*2];

static void
reqst_io(struct com_fmt *ptr_out)
{
	switch (ptr_out->payld.cmd) {
		case ALLOC:
			repl_alloc(ptr_out, IO_ENT);
			break;
		default:
			kprintf("Service not supported\n");
	}
}

static void
reqst_fs(struct com_fmt *ptr_out)
{
	switch (ptr_out->payld.cmd) {
		case ALLOC:
			repl_alloc(ptr_out, FS_ENT);
			break;
		default:
			kprintf("Service not supported\n");
	}
}

static void
reqst_mem(struct com_fmt *ptr_out)
{
	switch (ptr_out->payld.cmd) {
		case NEW_ENTRY:
			repl_newent(ptr_out, MEM_ENT);
			break;
		default:
			kprintf("Service not supported\n");
	}
}

static void
reqst_proc(struct com_fmt *ptr_out)
{
	switch (ptr_out->payld.cmd) {
		case ALLOC:
			repl_alloc(ptr_out, PROC_ENT);
			break;
		default:
			kprintf("Service not supported\n");
	}
}

/*
 * Process a request from a user-level server.
 */
void 
kprs_reqst(uint_t num_mail)
{
	struct com_fmt *ptr_out;
	uint_t i;

	ptr_out=ptr_mails[(num_mail*2)+1]; // point to corresponding mail box
	/*
	 * Process one or more requests from the user-level server.
	 */
	do {
		switch (num_mail) {
			case MEM_ENT:
					reqst_mem(ptr_out);
					break;
			case PROC_ENT:
					reqst_proc(ptr_out);
					break;
			case FS_ENT:
					reqst_fs(ptr_out);
					break;
			case IO_ENT:
					reqst_io(ptr_out);
					break;
		}
		ptr_out++;
	} while (ptr_out->src_port!=0);
}
