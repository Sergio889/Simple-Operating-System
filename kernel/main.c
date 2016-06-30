#include "defs/proc.h"
#include "defs/vm.h"
#include "templates/PROC/proc_abs.h"
/* global variables */
extern uaddr_t *ent_pts[4]; // servers new entry points
extern struct com_fmt *ptr_mails[4*2];
byte idle_var;

void
idle()
{
	idle_var=TRUE;

	kprintf("Going idle");
	while (idle_var == TRUE)
		;
}

int
main()
{
	/*
	 * Loop for something to do... FOREVER! MUAJAJAJAJA!.
	 */
	while (1) {
		while (ptr_mails[MEM_OUT]->src_port != 0 ||
					ptr_mails[PROC_OUT]->src_port != 0 ||
					ptr_mails[FS_OUT]->src_port != 0 ||
					ptr_mails[IO_OUT]->src_port != 0)
		{
			if (ptr_mails[MEM_OUT]->src_port != 0)
				kprs_reqst(MEM_ENT);
			if (ptr_mails[PROC_OUT]->src_port != 0)
				kprs_reqst(PROC_ENT);
			if (ptr_mails[FS_OUT]->src_port != 0)
				kprs_reqst(FS_ENT);
			if (ptr_mails[IO_OUT]->src_port != 0)
				kprs_reqst(IO_ENT);
			while (1)
				;
		}
		idle();
	}
}
