/*
 * This template provides the mechanisms for low-level management of the
 * notion of a process/thread.
 */
#ifndef COMMON
#define COMMON
#include "../../../libcommon/com.h"
#endif
#define NINITPROCS 4
#define NPROCVM 100
#define NPROCVMMST 100
#define NPROCINTOPS 100
#define NPROC 100
#define PROC_IS_SERV 0x00
#define PROC_IS_APP 0x01
#define PROC_RUN 0
#define MEM_RUN 1
#define FS_RUN 2
#define IO_RUN 3

uaddr_t *(*jmp_mem)(uaddr_t *);
int (*jmp_addrs[NINITPROCS-1])(uaddr_t *);

/*
 * One instance per process/thread.
 */
struct proc_servs {
	uaddr_t proc_phys;
	uint_t	proc_flags;
	byte proc_type;
	struct proc_servs *proc_frun; // next runnable process in run list
	struct proc_servs *proc_brun; // previous runnable process in run list
	struct proc_servs *proc_fslp; // next slept process in sleep list
	struct proc_servs *proc_bslp; // previous slept process in sleep list

	void *(*proc_int_ops[NPROCINTOPS])(void *);
};
struct proc_servs *ptr_proc_run; // pointer to head of runnable list
struct proc_servs *ptr_proc_slp; // pointer to head of slept-process list
/*
 * Some fundamental primitives for process management that provides the
 * mechanisms for process management.
 */
static void *proc_upd_pri(); /* update the current process' priority */
