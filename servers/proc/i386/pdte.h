/*
 * Page-directory table entry structure describes the first level of a virtual
 * memory-mapping for 386.
 */
struct pdt {
	unsigned	pdt_present:1;	/* present bit */
	unsigned	pdt_rdwr:1;			/* read/write permissions bit */
	unsigned	pdt_usrsup:1;		/* user/supervisor bit */	
	unsigned	pdt_wrthrough:1;
	unsigned	pdt_cache:1;		/* enable cache bit */
	unsigned	pdt_accsd:1;		/* accesed bit */
	unsigned	pdt_ign1:1;			/* ignored */
	unsigned	pdt_ps:1;				/* if CR4.PSE==1, must be 0, else, ignored */
	unsigned	pdt_ign2:4;			/* ignored */
	unsigned	pdt_addr:20;		/* address of 4-Kbyte aligned page table */
};
