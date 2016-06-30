#include "defs/proc.h"
#include "defs/vm.h"
#include "templates/PROC/proc_abs.h"

/* external variables */
extern struct mem_abs *ptr_heaps[6];

/* global variables */
uaddr_t *ent_pts[5]; // servers new entry points
struct com_fmt *ptr_mails[4*2]; // pointers to mails

/* private variables */
static uaddr_t ld_addrs[] = { 0x800000, 0x400000, 0xc00000, 0x1000000 };
static struct proc_servs procs_servs[NINITPROCS]={-1};

static void
run_servs()
{
	/* control variables */
	uint_t i, m;

	/* address variables */
	struct proc_servs *ptr_slp; // pointer to head of slept processes
	struct com_fmt *ptr_in; // pointer to inbox mail
	void *ptr_mls; // pointer to mails

	/* initialize each server */
//#ifdef VERBOSE
	kprintf("\n");
	kprintf("First call for user-level servers' initializations\n");
	kprintf("======================================================\n");
//#endif
	for (i=0; i<NINITPROCS; i++) {
		if (i==0)
			/* entry for memory server */
			jmp_mem = (void *)(ptr_proc_run->proc_phys+ENT_OFF);
		else
			/* entry for proc, fs and io servers */
			jmp_addrs[i] = (void *)(ptr_proc_run->proc_phys+ENT_OFF);
//#ifdef VERBOSE
		/* be verbose */
		kprintf("Calling server for initial running to address ");
		for (m=0; m<512; m++)
			buffer[m]=0;
//#endif
		/* first call: Memory Server */
		if (i==0) {
//#ifdef VERBOSE
			kprintf("%x\n", &jmp_mem);
//#endif
			/* 
			 * Call initialization routine for the memory-management server.
			 * Return base address for mail boxes.
			 */
			ptr_mls = (void *)(jmp_mem((uaddr_t *)ptr_heaps));
			/* save mail addresses */
			for (m=0; m<4*2; m++)
				ptr_mails[m] = (struct com_fmt *)ptr_mls, ptr_mls += PAGE_SIZE;
		}
		/* subsequent calls: proc, fs and io servers */
		else {
//#ifdef VERBOSE
			kprintf("%x\n", &jmp_addrs[i]);
//#endif
			/* 
			 * Call initialization routine for proc, fs and io servers.
			 * Return their new entry point.
			 */
			ptr_mls = (void *)(jmp_addrs[i]((uaddr_t *)ptr_mails[i*2]));
			ent_pts[i] = ptr_mls;
		}
		/* on first pass initialize list of slept processes */
		if (i==0) 
			ptr_proc_slp = ptr_proc_run;
		/* on subsequent passes, go to end of slept processes list add current one*/
		else {
			for (ptr_slp=ptr_proc_slp; ptr_slp->proc_fslp!=NULL; 
					ptr_slp=ptr_slp->proc_fslp)
				;
			ptr_slp->proc_fslp = ptr_proc_run;
		}
		if (i==NINITPROCS-1)
			ptr_proc_run=NULL;
		else
			ptr_proc_run+=1;
	}
#ifdef VERBOSE
	low_putstr("======================================================\n");
#endif
}

void
procs_init()
{
	/* control variables */
	uint_t i;

	ptr_buf=(byte *)kmalloc_page(1);
	/* address variables */
	struct mem_map_indpt *lst; // pointer to memory-mapping list

	/* initialize server processes managed by kernel */
	for (i=0; i<NINITPROCS; i++) {
		if (i==0) {
			procs_servs[i].proc_frun = procs_servs+1;
			procs_servs[i].proc_brun = NULL;
		}
		else if (i==NINITPROCS-1) {
			procs_servs[i].proc_frun = NULL;
			procs_servs[i].proc_brun = procs_servs-1;
		}
		procs_servs[i].proc_phys=ld_addrs[i];
		procs_servs[i].proc_fslp=NULL;
		procs_servs[i].proc_bslp=NULL;
		procs_servs[i].proc_type=PROC_IS_SERV;
	}

	/* new head of runnable list */
	ptr_proc_run = procs_servs;

	/* server processes initial runs */
	run_servs();

}
