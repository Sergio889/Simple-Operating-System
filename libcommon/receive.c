#ifndef COMMON
#define COMMON
#include "com.h"
#endif
#include "receive.h"

void
invalidate(byte *ptr_rm)
{
	uint_t i;
	/* invalidate entry */
	for (i=0; i<sizeof(struct com_fmt); i++)
		*ptr_rm++=0;
}

void *
receive(uint_t cli_num, uint_t msg_ent, void *mem_in)
{
	struct com_fmt *ptr_ent;
	struct com_fmt *ptr_in;
	void *ret_val;
	byte buffer[512];
	uint_t i;

	ptr_ent = (struct com_fmt *)mem_in; // point to beginning of inbox

	/* add offset to entry of response/request */
	ptr_ent += msg_ent;
	ret_val = ptr_ent; // return pointer to received message

	return ret_val;
}
