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

static uint_t
get_entry(struct com_fmt **cli_in, byte client_num)
{
	uint_t i=0, m;
	/* find an empty entry for responding in inbox mail */
	*cli_in=ptr_mails[client_num*2];
	if ((*cli_in)->src_port != 0)
		for (i=0; (*(cli_in+1))->src_port != 0; (*cli_in)++,i++)
			;

	return i;
}

/*
 * Used for registering a new entry point.
 */
void
repl_newent(struct com_fmt *req_ent, byte client_num)
{
	struct com_fmt *cli_in;
	uint_t i=0;

	/* save new entry point */
	ent_pts[client_num] = (void *)req_ent->payld.ptr_smwh;
	i=get_entry(&cli_in, client_num);

	/* write response */
	cli_in->ent = req_ent->ent; // respond to entry number
	cli_in->src_port = 100;
	cli_in->payld.cmd = NONE;
	cli_in->payld.flags |= (RESPONSE | ACK);

	/* send response to client */
	kprs_send(client_num, i);
}

/*
 * Used for allocating memory from the big page-based pools created
 * at boot.
 */
void
repl_alloc(struct com_fmt *req_ent, byte client_num)
{
	struct com_fmt *mem_in;
	struct com_fmt *mem_out;
	struct com_fmt *cli_in;
	uint_t i, m;

	/* 
	 * Memory allocation must be requested to the memory-management
	 * server.
	 */
	i=get_entry(&mem_in, MEM_ENT);
	mem_in += i;
	mem_in->payld.number = req_ent->payld.number; // number of pages
	mem_in->payld.cmd = ALLOC; // command to memory
	/* allocate from which heap? */
	switch (client_num) {
		case PROC_ENT:
			mem_in->payld.flags |= PROC_POOL; // from pool
			break;
		case FS_ENT:
			mem_in->payld.flags |= FS_POOL; // from pool
			break;
		case IO_ENT:
			mem_in->payld.flags |= IO_POOL; // from pool
			break;
		case OTHER:
			mem_in->payld.flags |= USR_POOL; // from pool
			break;
	}
	i=kprs_send(MEM_ENT, i);
	mem_out = ptr_mails[MEM_OUT];
	mem_out += i;

	i=get_entry(&cli_in, client_num);
	cli_in += i;
	cli_in->ent = req_ent->ent; // respond to entry number...
	cli_in->src_port = 100;
	cli_in->payld.cmd = ALLOC;
	cli_in->payld.flags |= (ACK | RESPONSE);
	cli_in->payld.ptr_smwh = mem_out->payld.ptr_smwh;
	invalidate((byte *)mem_out);
	i=kprs_send(client_num, i);
}
