#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#define SMBIOS_START 0xf0000
#define SMBIOS_END 0xfffff
#define NUM_TYPES 127
#define TYPE_BIOS 0
#define TYPE_SYS 1
#define TYPE_PROC 4
#define TYPE_CACH 7

static char *proc_fams[] = { "Other", "Unknown", "8086", "80286", "i386",
														 "i486", "8087", "80287", "80387", "80487",
														 "Pentium", "Pentium PRO", "Pentium II",
														 "Pentium MMX", "Celeron", "Pentium II Xeon",
														 "Pentium III", "M1", "M2", "Celeron M",
														 "Pentium IV", NULL, NULL, "Duron", "AMD K5",
														 "AMD K6", "AMD K6-2", "AMD K6-3", "Athlon" };
struct sm_hdr {
	ubyte_t sm_type;
	ubyte_t sm_len;
	ushort_t sm_han;
	struct sm_hdr *sm_next;
};

struct sm_local {
	uaddr_t *sm_addr;
	struct sm_local *sm_next;
};

struct smbios {
	char anch_str[4];
	ubyte_t ent_chk; // structure's checksum
	ubyte_t ent_len;
	ubyte_t smbios_ma;
	ubyte_t smbios_mi;
	ushort_t sm_lgst; // largest SMBIOS entry
	ubyte_t ent_rev;
	ubyte_t form_area[5];
	char int_anch[4];
	ubyte_t int_chk;
	ushort_t tab_len; // total table length
	word *ptr_tab; // table's address
	ushort_t sm_num; // number of SMBIOS entries
	ubyte_t sm_rev;
} sm_bios;
