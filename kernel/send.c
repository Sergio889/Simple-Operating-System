#include "defs/proc.h"
#include "defs/vm.h"
#include "templates/PROC/proc_abs.h"

/* external variables */
extern uaddr_t (*ent_pts[5])(int); // servers new entry points
extern struct com_fmt *ptr_mails[4*2];

void
kprs_send(uint_t num_mail, uint_t num_in_ent)
{
	switch (num_mail) {
		case PROC_ENT:
						ent_pts[PROC_ENT](num_in_ent);
						break;
		case MEM_ENT:
						ent_pts[MEM_ENT](num_in_ent);
						break;
		case FS_ENT:
						ent_pts[FS_ENT](num_in_ent);
						break;
		case IO_ENT:
						ent_pts[IO_ENT](num_in_ent);
						break;
	}
}
