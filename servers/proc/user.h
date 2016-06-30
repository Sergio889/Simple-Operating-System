/*
 * This traditional Unix structure contain low-level information about
 * a process. For 386, it includes segments selectors for code and data,
 * information about the memory-map, that is, page-directory table and 
 * page-entry table for SOS paging subsystem, the mandatory TSS, and state
 * of general-purpose registers.
 */

#include	"i386/pdte.h"
#include	"i386/pte.h"

struct user {

#ifdef i386
	/* in order: eax, ebx, ecx, edx, esi, edi, ebp */
	int	u_regs[NREGS]; 
	int	u_pcpsw[2]; /* eip and processor status word (EFLAGS) */
	int	u_esp[2]; /* user stack pointer in user and kernel */
	short	*u_same_4_all; /* same segments for all processes */
	struct pdt *u_pge_1st; /* first-level mapping structure */
	struct pt *u_pge_2nd; /* second-level mapping structure */
	struct cr *u_cr_regs; /* system control registers */
#endif
};

/* 
 * These numbers are index to the GDT 
 */
/* cs, ds, ss and ts segment selectors */ 
static short segs[NSEGREGS] = 1, 2, 2, 3; 
