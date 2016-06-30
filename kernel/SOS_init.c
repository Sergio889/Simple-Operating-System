#include "defs/vm.h"
#include "templates/PROC/proc_abs.h"
extern struct set_args kern_args_cpy;

/*
 * Initialize heaps and do first run for server programs.
 */
void
SOS_init()
{
	uint_t i, m;

	procs_init();
	/* enter SOS's debugger? */
	if (kern_args_cpy.debug == TRUE)
		set_single_step();

	main();
}
