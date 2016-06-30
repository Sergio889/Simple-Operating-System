/*
 * Page-table entry structure describes the second-level mapping of 386
 * virtual-memory system.
 */
struct pte {
	unsigned	pte_pres:1;		/* present bit */
	unsigned	pte_rdwr:1;		/* read/write bit */
	unsigned	pte_usrsup:1; /* user/supervisor bit */
	unsigned	pte_wrthr:1;
	unsigned	pte_encache:1;	/* cache-enable bit */
	unsigned	pte_accsd:1;		/* accessed bit */
	unsigned	pte_dty:1;			/* dirty bit */
	unsigned	pte_pat:1;			/* pat extension bit */
	unsigned	pte_glo:1;			/* global bit */
	unsigned	pte_ign1:3;			/* ignored */
	unsigned	pte_addr:20;		/* address of 4-Kbyte page */
};
