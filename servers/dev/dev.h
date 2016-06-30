#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#define NUM_INIT_ALLOCS 3 // number of initial structures
#define IS_DEV 0x01
#define IS_BRG 0x02
#define DEV_IS_HOST 0x01
#define DEV_IS_ISA 0x02
#define DEV_IS_PCI 0x04
#define MEM_MAPPED 0x08
#define IO_MAPPED 0x10
#define OTHER_POOL_GRAN 64
#define MAPS_POOL_GRAN 32

struct com_fmt *pool_in_out[2];

struct dev_mem {
	struct pool_mgmt *ptr_pool_devs;
	struct pool_mgmt *ptr_pool_maps;
	struct pool_mgmt *ptr_pool_others;
	uint_t granularities[3];
} dev_mem;

struct dev_maps {
	uint_t io_or_mem; // is it io or mem mapped?
	uaddr_t ent_addr[2]; // base address in entry
	struct dev_maps *next_map; // next entry in map chain
};

struct dev {
	uint_t dev_class; // bus, bridge or device
	struct pci_conf *pci_conf; // pointer to configuration space structure
	word bus_num;
	word dev_num;
	word func_num;
	byte dev_irq;
	word flags; // bit map if bus
	word dev_regs[6];
	word dev_regs_save[6];
	void (*dev_entry)();
	struct dev_maps *dev_maps; // describe mappings to io/mem
};
#define DEV_POOL_GRAN sizeof(struct dev)
struct devs_products {
	uint_t devs_pid;
	char *devs_pstr;
};
struct devs_vendors {
	uint_t devs_vid;
	char *devs_vstr;
	struct devs_products *devs_products;
};

struct dev_types {
	char **dev_class;
	char **dev_sub;
} dev_typ[18];
