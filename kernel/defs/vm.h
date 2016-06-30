#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#include "../../libcommon/elf32.h"
#include "../../libcommon/SOSfs.h"
#include "../../libcommon/bios.h"
#include "../templates/MEM/mem_abs.h"
#endif
#define PAGE_SIZE 4096
#define PAGE_SHIFT 0xc

/*
 * The kernel only knows about page frames.
 */
struct phys_pag_frams {
	uint_t pag_num; // page number (4K units)
	struct phys_pag_frams *next_pag;
	struct phys_pag_frams *prev_pag;
};

/*
 * Machine-independent format for free memory-mapping list.
 */
struct mem_map_indpt {
	uaddr_t base_addr[2]; // base address
	uint_t addr_len[2]; // hole lenght
	struct mem_map_indpt *next_free; // next entry
};

struct kmon_servs_maps {
	uaddr_t base_addr;
	uint_t len_bytes;
};
struct kmon_servs_maps kms_maps[7];
