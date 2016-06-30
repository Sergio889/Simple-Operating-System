/*
 * After start maps low-memory and the kernel to physical memory it jumps
 * here to do further initialization.
 * Each server has a defined physical address, which will be identicaly mapped
 * when virtual-memory is working.
 */
#include "../../libcommon/com.h"
#include "../../libcommon/elf32.h"
#include "../../libcommon/SOSfs.h"
#include "../../libcommon/bios.h"
#include "../defs/vm.h"
#define PT_FIRST 0x01401000 // first page table
#define IDENT_MAP_TO (4<<20)*50 // identical map to address
#define NUM_LOADS 5
#define MON_PGNUM
#define PHYS_KMON 0x00000000 // start of monitor
#define PHYS_PROC 0x00400000 // start of virtual-memory server
#define PHYS_VM 0x00800000 // start of file-system server
#define PHYS_FS 0x00c00000 // start of device server
#define PHYS_DEV 0x01000000 // start of virtual-memory server
#define VIR_MEM_STRUCS 0x01400000 // start of paging structures
#define HEAP_STRUCS 0x01800000 // start of paging structures
#define SEC_SHIFT 0x9
#define PAG_SHIFT 0x0c
#define LOW_MEM 0x400000

static byte *ptr_temp;

struct ext_mem_format {
	uaddr_t base_addr[2];
	usize_t reg_len[2];
	uint_t reg_type;
	uint_t acpi_ext;
	struct ext_mem_format *next_free; // keep list of usable RAM
};
struct ext_mem_format *ptr_list; // 386 usable RAM list format
struct mem_map_indpt *ptr_list_indpt; // machine-independent usable RAM list format

/* physical addresses to load servers */
static uaddr_t phys_addrs[] = { PHYS_KMON, PHYS_PROC, PHYS_VM, PHYS_FS, PHYS_DEV, VIR_MEM_STRUCS, HEAP_STRUCS };

struct set_args kern_args_cpy;
usec_t ivec_first_abs_sec; // absolute sector to first ivector
usec_t ikern_abs_sec; // absolute sector to first ivector
usec_t kern_abs_sec; // absolute sector to first ivector
usize_t kern_size; // kernel size in sectors

static void
mmap_init()
{
	uaddr_t *ptr_pgdir;
	uaddr_t *ptr_pgtab;
	uaddr_t pg_frame;
	uint_t i, m;

	i=pg_frame=0;
	for (ptr_pgdir=(uaddr_t *)VIR_MEM_STRUCS, ptr_pgtab=(uaddr_t *)PT_FIRST; 
			i<IDENT_MAP_TO; i+=(4<<20)) 
	{
		*ptr_pgdir=((uaddr_t)ptr_pgtab)|0xb;
		ptr_pgdir++;
		for (m=0; m<1024; m++) {
			*ptr_pgtab=pg_frame|0xb; 
			pg_frame+=0x1000; 
			ptr_pgtab++;
		}
	}
}

/*
 */
static void
load_init()
{
	struct inode cur_inode; // current-inode structure
	byte sec_buf[512]; // buffer to load a sector

	/* control variables */
	int i, m, count; // counters
	usize_t data_size; // save data size from inode

	/* sector variables */
	usec_t inode_sec; // get inode for each server
	usec_t data_sec; // sector for data is one more than inode

	/* buffers and buffer pointers */
	byte *ptr_buff; // pointer to sector buffer
	char *ptr_args; // pointer to kernel arguments

	uaddr_t *ptr_addrs = phys_addrs;

	/* point to beginning of kernel arguments */
	ptr_args = (char *)&kern_args_cpy;

	/* 
	 * Read each server to predefined physical memory.
	 * On every loop the following is done:
	 * 1. Get inode absolute sector for the program being mapped.
	 *    We do so by passing the path inside the file-system for it.
	 * 2. Read the inode and store it in structure for current inode.
	 * 3. If we are dealing with the first loading:
	 *  - Save kernel's size in sectors plus the number of sectors for
	 *  	mapping the first MB of memory.
	 * 4. Else
	 * 	- Read the program from disc by sectors.
	 * 	- Copy it to predefined locations sector by sector.
	 */
	kprintf("\n");
	kprintf("Monitor/Kernel and user-level servers mappings\n");
	kprintf("======================================================\n");
	for (i=0; i<NUM_LOADS; i++, ptr_args+=4) {
		/* read kernel's and server's inode sector */
		inode_sec = sosfs_iget((char *)*((int *)ptr_args));
		/* if error */
		if (inode_sec == -1) {
			kprintf("Inode not found\n"); 
			while (1);
		}
		data_sec = inode_sec+1; // data follows inode
		/* read kernel's and server's inode */
		sosfs_read_raw(inode_sec, &cur_inode);
		kprintf("File ");
		kprintf((char *)*((int *)ptr_args)); // print file-path
		/* the first mapping is of the monitor program and the kernel */
		if (i == 0) {
			kms_maps[i].base_addr = phys_addrs[i];
			kms_maps[i].len_bytes = (4192+cur_inode.i_dsize)<<SEC_SHIFT;
			kprintf(" and kmon were already loaded to ");
		}
		else {
			/* read file by sectors */
			for (data_size=0; data_size<cur_inode.i_dsize; data_size++) {
				sosfs_read_raw(data_sec+data_size, sec_buf);
				/*
				 * This expression copies the sector just read to the physical 
				 * location where the program's image is linked to.
				 * Wouldn't be much of a C programmer if couldn't find an expression 
				 * almost impossible to decipher. :P
				 */
				for (ptr_buff=sec_buf, count=0; count<512; count++)
					*((uaddr_t *)((*(ptr_addrs+i))+(512*data_size)+(count)))=
					*(ptr_buff+count);
			}
			kprintf(" loaded to ");
			kms_maps[i].base_addr = phys_addrs[i];
			kms_maps[i].len_bytes = data_size<<SEC_SHIFT;
		}
		kprintf("%x", &phys_addrs[i]);
		kprintf("\n");
	}
#ifdef VERBOSE
	low_putstr("======================================================\n");
#endif
	kms_maps[i].base_addr = phys_addrs[i];
	kms_maps[i].len_bytes = (4<<20);
}
	
/*
 * Copy the kernel arguments from low-memory to high-memory using
 * an interrupt strategy.
 */
static void
kargs_init()
{
	low_memcpy(&kern_args_cpy, sizeof(struct set_args));
}

static void
put_list(struct ext_mem_format *put_arg)
{
	static byte first_bool=TRUE; // first-time boolean

	struct ext_mem_format *ptr_tmp;

	/* if first free entry */
	if (first_bool == TRUE)
		ptr_list = put_arg, first_bool=FALSE;

	/* else traverse the list and append the entry */
	else {
		for (ptr_tmp=ptr_list; ptr_tmp->next_free!=NULL; ptr_tmp=ptr_tmp->next_free)
			;
		ptr_tmp->next_free = put_arg;
	}
}

static void
sort_free_list()
{
	uint_t num_ents=0, count;
	struct ext_mem_format *ptr_tmp, *ptr_tmp1;

	/* get number of entries */
	for (ptr_tmp=ptr_list; ptr_tmp->next_free!=NULL; ptr_tmp=ptr_tmp->next_free)
		num_ents++;

	/* simple, linearly complex sorting algorithm */
	for (count=0; count<num_ents; count++) {
		for (ptr_tmp=ptr_list;ptr_tmp->next_free!=NULL;ptr_tmp=ptr_tmp->next_free) {
			if (ptr_tmp->base_addr[0] > ptr_tmp->next_free->base_addr[0]) {
				if (ptr_tmp==ptr_list)
					ptr_list=ptr_tmp->next_free;
				ptr_tmp1 = ptr_tmp->next_free;
				ptr_tmp->next_free = ptr_tmp->next_free->next_free;
				ptr_tmp->next_free = ptr_tmp;
			}
		}
	}
}

/*
 * Convert list to a standard machine-independent list.
 */
static void
dep_2_indep_list(struct ext_mem_format *ptr_lst)
{
	uint_t num_ents; 
	uaddr_t base_tmp;

	struct ext_mem_format *ptr_tmp;
	struct mem_map_indpt *ptr_indp_tmp;

	ptr_list_indpt = (struct mem_map_indpt *)ptr_lst;
	for (ptr_tmp=ptr_lst, ptr_indp_tmp=ptr_list_indpt; ptr_tmp->next_free!=NULL; ptr_tmp=ptr_tmp->next_free) {
		ptr_indp_tmp->next_free = (struct mem_map_indpt *)ptr_tmp->next_free;
		ptr_indp_tmp = (struct mem_map_indpt *)ptr_tmp->next_free;
	}
	ptr_indp_tmp->next_free=NULL;
	if (ptr_list_indpt->base_addr[0] == 0)
		ptr_list_indpt = ptr_list_indpt->next_free;
	base_tmp = ptr_list_indpt->base_addr[0];
	ptr_list_indpt->base_addr[0] = (4<<20)*5;
	ptr_list_indpt->addr_len[0] -= ((4<<20)*5) - base_tmp;
}

static void
zero_mem(const struct ext_mem_format * const ptr_mem)
{
	/* address variables */
	uaddr_t *ptr_addr; // pointer to address to zero
 	uaddr_t	cur_addr; // holds current address value

	/* control variables */
	usize_t len_cpy;

	for (len_cpy=0, cur_addr=ptr_mem->base_addr[0]; len_cpy<ptr_mem->reg_len[0]; 
			 len_cpy+=4) 
	{
		if (cur_addr < LOW_MEM) // XXX use kmon_heap instead
			;
		else
			ptr_addr = (uaddr_t *)cur_addr, *ptr_addr=0; // clear by words
		cur_addr+=4; // next word
	}
}

void
init()
{
	struct ext_mem_format *ptr_exts; // structure-pointer to structure
	struct ext_mem_format temp_struc; // temporary structure
	struct inode ino_kern;
	struct ext_mem_format ext_mem_strucs[50]; // 386 usable RAM list format

	usec_t ino_sec;
	ubyte_t num_servs; // number of servers to read to memory
	uint_t tot_mem, counter, counter1; // counters
	byte *ptr_mem=(byte *)ext_mem_strucs; // byte-pointer to structure

	/* 
	 * Get absolute sector for first ivector.
	 * It is stored in a global variable.
	 */
	sosfs_ivec_first();

	/* read kernel's and server's inode sector */
	ino_sec = sosfs_iget("/boot/kernel");
	/* if error */
	if (ino_sec == -1) {
		low_putstr("Inode not found\n"); 
		while (1);
	}
	/* read kernel's inode */
	sosfs_read_raw(ino_sec, &ino_kern);

	/* zero out the vector */
	for (tot_mem=0; tot_mem<512; tot_mem++)
		buffer[tot_mem]=0;

	/* 
	 * Copy arguments from real-mode to protected-mode.
	 * The monitor program wishes to pass the arguments to the kernel.
	 * This is done by a little hack that uses an interrupt instruction
	 * passing a buffer to copy from real-mode to protected-mode using
	 * BIOS.
	 */
	kargs_init();

	/* 
	 * Get total amount of RAM.
	 * The most reliable way to know the system's memory-mapping
	 * is by using the BIOS; we use int $0x15 function 0xe820.
	 */
	low_tot_ram(ext_mem_strucs);

	/* point to memory-map vector */
	ptr_exts=ext_mem_strucs;
	/* 
	 * Traverse the structures until magic number found.
	 * Our interrupt handler set a magic number after the last
	 * entry returned by the BIOS handler.
	 */
	kprintf("\n");
	kprintf("BIOS-provided physical-memory mappings\n");
	kprintf("======================================================\n");
	while (ptr_exts->acpi_ext != 0x12345) {
		/* if we must ignore the entry, so we do */
		if (ptr_exts->reg_len == 0)
			continue;
		/* print type of memory to terminal */
		switch (ptr_exts->reg_type) {
			case 1:
				kprintf("Usable RAM at ");
				if (ptr_exts->base_addr[0] != 0) {
					zero_mem(ptr_exts);
					put_list(ptr_exts);
				}
				break;
			case 2:
				kprintf("Reserved/unusable RAM at ");
				break;
			case 3:
			case 4:
			case 5:
				kprintf("ACPI RAM at ");
				break;
		}
		/*
		 * Create a temporary structure and copy the entire structure
		 * to it.
		 * Print the address range.
		 */
		temp_struc = *ptr_exts; // copy structure
		temp_struc.reg_len[0] += temp_struc.base_addr[0];
		kprintf("%x-", &temp_struc);
		kprintf("%x", temp_struc.reg_len);
		kprintf("\n");
		for (tot_mem=0; tot_mem<512; tot_mem++)
			buffer[tot_mem]=0;
		ptr_exts++; // advance one structure
	}
	kprintf("======================================================\n");

	/* 
	 * Set up initial memory mappings.
	 * Load user-level servers to predefined physical memory and
	 * identically map them. Map the monitor program and the kernel
	 * also to identical physical addresses. Also map those programs'
	 * heaps.
	 */
	load_init();
	heap_init();
	/* list returned by BIOS might be unsorted */
	sort_free_list();
	/* machine-dependent format to machine-independent format */
	dep_2_indep_list(ptr_list);
	mmap_init();
	enable_paging();
	SOS_init();
}
