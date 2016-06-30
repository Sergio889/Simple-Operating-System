#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif

struct pnp_dev_nod {
	ushort_t nod_size;
	ubyte_t nod_handl;
	uint_t nod_id;
	ubyte_t nod_bas;
	ubyte_t nod_sub;
	ubyte_t nod_infc;
	ushort_t nod_attr;

	byte nod_arb[38]; // XXX
	struct pnp_dev_nod *pnp_next;
};
