#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif

struct com_fmt *pool_in_out[2];

struct proc {
	uint_t p_state; // zombie, runnable... 
	uint_t p_pid; // (b) Process identifier. 

	struct ucred *p_ucred; // (c) Process owner's identity. 
	struct filedesc *p_fd; // (b) Open files. 
	struct pstats *p_stats; // (b) Accounting/statistics (CPU). 
	struct plimit *p_limit; // (c) Process limits.
	struct sigacts *p_sigacts; // (x) Signal actions, state (CPU).
	struct proc	*p_pptr; // (c + e) Pointer to parent process.

	/* The following fields are all zeroed upon creation in fork. */
	int p_sigparent; // (c) Signal to parent on exit.
	int p_pendingcnt; // how many signals are pending

	/* The following fields are all copied upon creation in fork. */
	struct pgrp	*p_pgrp; // (c + e) Pointer to process group.
	struct pargs	*p_args; // (c) Process arguments.

	/* 
	 * Doubly-linked list used either for linking with runnable list or
	 * sleeping list.
	 */
	struct proc *ptr_rsnext; // next runnable/slept process
	struct proc *ptr_rsback; // previous runnable/slept process
};
struct proc *ptr_cur;
struct proc *ptr_hd_run;
struct proc *ptr_hd_slp;

struct proc_mem {
	struct pool_mgmt *ptr_pool_procs;
	uint_t granularities[1];
} proc_mem;
