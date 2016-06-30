#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#include "dev.h"
#include "pci.h"
#include "irq.h"
#define MAX_BUS 256
#define MAX_DEVS 32
#define MAX_FUNC 8
#define BIT_ENABLE 0x7fffffff

struct dev *dev_head;
struct pci_conf *conf_head;
struct pci_bus *bus_head;
char *dev_classes[] = { NULL, "<Mass Storage>", "<Network>", "<Display>",
													 "<Multimedia>", "<Memory>", "<Bridge>", "<COM>", "<System>",
													 "<Input>", "<Docking>", "<Processor>", "<Serial>", 
													 "<Wireless>", "<Intelligent Input/Output>", 
													 "<Satellite Communication Controller>", 
													 "<Encryption/Decryption Controller>", 
													 "<WTF Controller>"
};

char *dev_subs[] = { "Reserved", "<VGA-compatible Controller>", 
														"<SCSI Bus Controller>", "<IDE Controller>", 
														"<Floppy Disk Controller>", "<IPI Bus Controller>",
													 	"<RAID Controller>", "<ATA Controller>", 
														"<Serial ATA DPA>", 
														"<Other Mass Storage Controller>", 
														"<Ethernet Controller>", "<Token Ring Controller>",
													 	"<FDDI Controller>", "<ATM Controller>", 
														"<ISDN Controller>", "<WorldFip Controller>", 
														"<PCIMG Controller>", "<Other Network Controller>",
													 	"<VGA-compatible Controller>", "<XGA Controller>", 
														"<3D Controller>", "<Other Display Controller>", 
														"<Video Device>", "<Audio Device>", 
														"<Computer Telephony Device>", 
														"<Other Multimedia Device>", "<Flash>", "<RAM>", 
														"<Other Memory Controller>", "<Host Bridge>", 
														"<ISA Bridge>", "<EISA Bridge>", "<MCA Bridge>", 
														"<PCI-PCI Bridge>", "<PCMCIA Bridge>", 
														"<NuBus Bridge>", "<CardBus Bridge>", 
														"<PCMCIA Bridge>", "<Rare PCI-PCI Bridge>", 
														"<Infiniband-PCI Bridge>", 
														"<Other Bridge Controller>", "<IEEE 1394 FireWire>",
													 	"<ACCESS.bus>", "<SSA>", "<USB>", "<Fibre Channel>",
													 	"<SMBus>", "<InfiniBand>", "<IPMI>", "<SERCOS>", 
														"<CANbus>", "<iRDA-compatible Controller>", 
														"<Consumer IR Controller>", "<RF Controller>", 
														"<Bluetooth>", "<Broadband>", 
														"<Ethernet (802.11a-5ghz)>", 
														"<Ethernet (802.11b-2.4ghz)>", 
														"<Other Wireless Controller>" };

char *dev_ints[] = { "<Single DMA>", "<Chained DMA>", 
														"<~Subtractive Decode>", "<Subtractive Decode>", 
														"<UHCI>", "<OHCI>", "<EHCI>", 
														"<Undefined interface>"
};

static char **spec_subs[] = { &dev_subs[0], &dev_subs[2], &dev_subs[10], 
														 &dev_subs[18], &dev_subs[20], &dev_subs[24], 
														 &dev_subs[29], NULL, NULL, NULL, NULL, NULL, 
														 &dev_subs[41], &dev_subs[51], NULL, NULL, NULL,
														 NULL
};

static uint_t
pci_conf_wr(uint_t word_num, uint_t dev_num,
						uint_t bus_num, uint_t dev_func, uint_t wr_value)
{
	word rd_ret; // word to store return value
	word rd_arg; // word to read from io space
	uint_t i, m, n, o;

	rd_arg = 0;
	word_num <<= 2; // configuration-space register
	dev_func <<= 8; // function number
	dev_num <<= 11; // device number
	bus_num <<= 16; // bus number
	rd_arg = dev_func | word_num | dev_num | bus_num | (~BIT_ENABLE);
	rd_ret = low_pci_conf_wr(rd_arg, wr_value);

	return rd_ret;
}

static uint_t
pci_conf_rd(uint_t word_num, uint_t dev_num,
						uint_t bus_num, uint_t dev_func)
{
	word rd_ret; // word to store return value
	word rd_arg; // word to read from io space
	byte buffer[512];
	uint_t i, m, n, o;

	rd_arg = 0;
	word_num <<= 2; // configuration-space register
	dev_func <<= 8; // function number
	dev_num <<= 11; // device number
	bus_num <<= 16; // bus number
	rd_arg = dev_func | word_num | dev_num | bus_num | (~BIT_ENABLE);
	rd_ret = low_pci_conf_rd(rd_arg);

	return rd_ret;
}

static void
pci_verbose()
{
	struct pci_conf *ptr_cfg; // pointer to configuration record
	struct dev *ptr_dev; // pointer to device record
	struct devs_vendors *ptr_vid;
	struct devs_products *ptr_pid;
	struct dev_maps *ptr_maps;
	word rd_reg; // word read from configuration space
	uint_t i, m, n, o, l;
	word prnt_w;
	for (i=0; i<18; i++) {
		dev_typ[i].dev_class=dev_classes;
		dev_typ[i].dev_sub=spec_subs[i];
	}

	low_putstr("\n");
	low_putstr("Probing devices (assuming PCI as local bus)\n");
	low_putstr("======================================================\n");
	/* be verbose */
	for (o=0, ptr_dev=dev_head, ptr_cfg=conf_head; 
			ptr_cfg->pci_lout.pci_dev.id_vendor!=0; ptr_dev++, o++)
	{
		kprintf(" pci%1d: ", &(ptr_dev->bus_num));
		if (ptr_cfg->pci_lout.pci_dev.id_vendor > 0x10fd) {
			ptr_vid = &dev_v6[0];
			if (ptr_cfg->pci_lout.pci_dev.id_vendor > 0x1132)
				ptr_vid = &dev_v7[0];
				if (ptr_cfg->pci_lout.pci_dev.id_vendor > 0x1166)
					ptr_vid = &dev_v8[0];
					if (ptr_cfg->pci_lout.pci_dev.id_vendor > 0x11a2)
						ptr_vid = &dev_v9[0];
						if (ptr_cfg->pci_lout.pci_dev.id_vendor > 0x1220)
							ptr_vid = &dev_v10[0];
							if (ptr_cfg->pci_lout.pci_dev.id_vendor>0x13f6)
								ptr_vid = &dev_v11[0];
								if(ptr_cfg->pci_lout.pci_dev.id_vendor>0x17fe)
									ptr_vid = &dev_v12[0];
		}
		else {
			ptr_vid = &dev_v5[0];
			if (ptr_cfg->pci_lout.pci_dev.id_vendor < 0x10cc)
				ptr_vid = &dev_v4[0];
				if (ptr_cfg->pci_lout.pci_dev.id_vendor < 0x109a)
					ptr_vid = &dev_v3[0];
					if (ptr_cfg->pci_lout.pci_dev.id_vendor < 0x1067)
						ptr_vid = &dev_v2[0];
						if (ptr_cfg->pci_lout.pci_dev.id_vendor < 0x1033)
							ptr_vid = &dev_v1[0];
		}
		/* find vendor string */
		if (ptr_vid != NULL) {
			while (((uint_t)ptr_vid->devs_vid)!=ptr_cfg->pci_lout.pci_dev.id_vendor)
				ptr_vid++;
			low_putstr(ptr_vid->devs_vstr);
			low_putstr(" at ");
		}
		else {
			kprintf("%2x at ", ptr_cfg);
		}
		/* print bus number */
		kprintf("%1d:", &ptr_dev->bus_num);
		/* print device number */
		kprintf("%1d:", &ptr_dev->dev_num);
		/* print function number */
		kprintf("%1d\n", &ptr_dev->func_num);
		/* find product string and print it */
		if (ptr_vid != NULL) {
			for (ptr_pid=&ptr_vid->devs_products[0]; 
					 ptr_pid->devs_pid!=ptr_cfg->pci_lout.pci_dev.id_product && 
					 ptr_pid->devs_pid!=-1; ptr_pid++)
				;
			low_putstr(ptr_pid->devs_pstr);
			if (ptr_pid->devs_pid == -1) {
				kprintf(" %2x", &(ptr_cfg->pci_lout.pci_dev.id_product));
			}
		}
		else {
			kprintf("%2x", &(ptr_cfg->pci_lout.pci_dev.id_product));
		}
		/* print revision number */
		kprintf(" Rev=%1d Class=", &(ptr_cfg->pci_lout.pci_dev.id_rev));
		/* print device-class specific information */
		low_putstr(dev_typ[ptr_cfg->pci_lout.pci_dev.class_code[2]].dev_class[ptr_cfg->pci_lout.pci_dev.class_code[2]]);
		low_putstr(" Subclass=");
		low_putstr(dev_typ[ptr_cfg->pci_lout.pci_dev.class_code[2]].dev_sub[ptr_cfg->pci_lout.pci_dev.class_code[1]]);
		low_putstr(" ");
		if (ptr_cfg->pci_lout.pci_dev.class_code[2]==0x06) {
			switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
				case 0x00:
					ptr_dev->flags |= DEV_IS_HOST;
					break;
				case 0x01:
					ptr_dev->flags |= DEV_IS_ISA;
					break;
				case 0x04:
					ptr_dev->flags |= DEV_IS_PCI;

					kprintf("Primary=%1d", &(ptr_cfg->pci_lout.pci_brg.pri_bus_num));
					kprintf(" Secondary=%1d ", &(ptr_cfg->pci_lout.pci_brg.sec_bus_num));
					kprintf("Subordinate=%1d", &(ptr_cfg->pci_lout.pci_brg.sub_bus_num));
					prnt_w=ptr_cfg->pci_lout.pci_brg.io_base;
					prnt_w&=0xf0;
					kprintf(" IO=%1x", &prnt_w);
					low_putstr("00-");
					prnt_w=ptr_cfg->pci_lout.pci_brg.io_limit;
					prnt_w&=0xf0;
					prnt_w|=0x0f;
					kprintf("%1x", &prnt_w);
					low_putstr("ff");
					prnt_w=ptr_cfg->pci_lout.pci_brg.mem_base;
					prnt_w&=0xfff0;
					kprintf(" MEM=%2x", &prnt_w);
					low_putstr("0000-");
					prnt_w=ptr_cfg->pci_lout.pci_brg.mem_limit;
					prnt_w&=0xfff0;
					prnt_w|=0x000f;
					kprintf("%2x", &prnt_w);
					low_putstr("ffff");
					prnt_w=ptr_cfg->pci_lout.pci_brg.mem_prftch_base;
					prnt_w&=0xfff0;
					kprintf(" PREFTCH=%2x", &prnt_w);
					low_putstr("0000-");
					prnt_w=ptr_cfg->pci_lout.pci_brg.mem_prftch_limit;
					prnt_w&=0xfff0;
					prnt_w|=0x000f;
					kprintf("%2x", &prnt_w);
					low_putstr("ffff");
					break;
			}
		}
		if (ptr_cfg->pci_lout.pci_dev.class_code[2]==0x06) {
			switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
				case 0x03:
					switch (ptr_cfg->pci_lout.pci_dev.class_code[0]) {
						case 0x00:
							low_putstr("<UHCI>");
							break;
						case 0x10:
							low_putstr("<OHCI>");
							break;
						case 0x20:
							low_putstr("<EHCI>");
							break;
						case 0x80:
							low_putstr("<Undefined interface>");
							break;
					}
					break;
			}
		}
		if (ptr_cfg->pci_lout.pci_dev.class_code[2]!=0x06) {
			for (o=0; o<6; o++) {
				if (ptr_dev->dev_regs[o]==0)
					continue;
				for (i=0; i<512; i++)
					buffer[i] = 0;
				if (ptr_dev->flags==MEM_MAPPED)
					low_putstr(" MEM=");
				else
					low_putstr(" IO=");
				kprintf("%x", &(ptr_dev->dev_regs_save[o]));
				low_putstr("-");
				prnt_w=ptr_dev->dev_regs_save[o]+ptr_dev->dev_regs[o];
				prnt_w--;
				kprintf("%x", &(prnt_w));
			}
			kprintf(" IRQ=%1d", &(ptr_cfg->pci_lout.pci_dev.int_line));
			low_putstr(" INT");
			switch (ptr_cfg->pci_lout.pci_dev.int_pin) {
				case 1:
					low_putstr("A");
					break;
				case 2:
					low_putstr("B");
					break;
				case 3:
					low_putstr("C");
					break;
				case 4:
					low_putstr("D");
					break;
			}
		}
		low_putstr("\n");
		ptr_cfg++;
	}
}

static void
read_regs(struct pci_conf *ptr_arg, struct dev *ptr_dev)
{
	word *ptr_word;
	uint_t i;
	byte buffer[512];

	/* 
	 * Read entire configuration space and store it for later use.
	 */
	for (ptr_word=(word *)ptr_arg, i=0; i<64; i++) {
		*ptr_word = pci_conf_rd(i, ptr_dev->dev_num,
							  ptr_dev->bus_num, ptr_dev->func_num);
		ptr_word++;
	}
}

void
pci_conf()
{
	struct pci_conf *ptr_cfg; // pointer to configuration record
	struct dev *ptr_dev; // pointer to device record
	struct devs_vendors *ptr_vid;
	struct devs_products *ptr_pid;
	struct dev_maps *ptr_maps;
	struct pci_bus *ptr_bus;
	word rd_reg; // word read from configuration space
	uint_t i, m, n, o, z, s;

	/*
	 * Allocate memory from the map created for the device record.
	 * Granularity of allocation is sizeof(struct dev).
	 * XXX allocate maximum theoretical device number of records; free the
	 * ones not used at the end.
	 * The allocation is an array.
	 */
	dev_head=(struct dev *)
					 pool_alloc(dev_mem.ptr_pool_devs, 
											(sizeof(struct dev)*40)/DEV_POOL_GRAN, DEV_POOL_GRAN);
	ptr_dev = dev_head; // traverse de array of allocated records
	/*
	 * Allocate memory from the map created for 'other' purposes.
	 * Granularity of allocation is 64-bytes.
	 * XXX allocate maximum theoretical device number of records; free the
	 * ones not used at the end.
	 * The allocation is an array.
	 */
	conf_head=(struct pci_conf *)
						pool_alloc(dev_mem.ptr_pool_others, 
											(sizeof(struct pci_conf)*40)/OTHER_POOL_GRAN, 
											OTHER_POOL_GRAN);
	if (conf_head==NULL) {
		low_putstr("Heap allocation failed.");
		while (1)
			;
	}
	ptr_cfg = conf_head; // traverse de array of allocated records
	/*
	 * Allocate memory from the map created for 'other' purposes.
	 * Granularity of allocation is 64-bytes.
	 * XXX allocate maximum theoretical device number of records; free the
	 * ones not used at the end.
	 * The allocation is an array.
	 */
	bus_head=(struct pci_bus *)pool_alloc(dev_mem.ptr_pool_others, 1, 
									OTHER_POOL_GRAN);
	if (bus_head==NULL) {
		low_putstr("Heap allocation failed.");
		while (1)
			;
	}

	ptr_bus = bus_head; // traverse de array of allocated records
	ptr_bus->bus_num=0;
	ptr_bus->bus_next=(struct pci_bus *)pool_alloc(dev_mem.ptr_pool_others, 1, 
										 OTHER_POOL_GRAN);
	if (ptr_bus->bus_next==NULL) {
		low_putstr("Heap allocation failed.");
		while (1)
			;
	}
	ptr_bus=ptr_bus->bus_next;
	/* loop MAX_BUS numbers for potentially 256 buses */
	for (i=0; i<MAX_BUS; i++) {
		/* loop MAX_DEVS numbers for potentially 32 devices on current bus */
		for (m=0; m<MAX_DEVS; m++) {
			 /* initiate a read configuration transaction to device */
			for (n=0; n<MAX_FUNC; n++) {
				/* read current device; VENDOR_ID is 0xffff if no device */
				if ( ((rd_reg=pci_conf_rd(VENDOR_ID, m, i, n))==(~0)) && 
					     n==0)
					break;
				else if ((rd_reg == (~0)) && n>0)
					continue;
				ptr_dev->pci_conf = ptr_cfg; // pointer to configuration space
				ptr_dev->bus_num = i; // bus number
				ptr_dev->dev_num = m; // device number
				ptr_dev->func_num = n; // function number
				ptr_dev->dev_maps = NULL;
				/* we have a valid device; store predefined header values */
				read_regs(ptr_cfg, ptr_dev);
				/* configure domain space if not a host bridge */
				if ((ptr_cfg->pci_lout.pci_dev.class_code[2]==0x06
						&&ptr_cfg->pci_lout.pci_dev.class_code[1]!=0x00)
						 || ptr_cfg->pci_lout.pci_dev.class_code[2]!=0x06)
				{
					if (ptr_cfg->pci_lout.pci_dev.class_code[2]==0x06) {
						z=2; // number of configuration registers
						ptr_dev->dev_class=IS_BRG;
						ptr_bus->bus_num=ptr_cfg->pci_lout.pci_brg.sec_bus_num;
						ptr_bus->bus_sub=ptr_cfg->pci_lout.pci_brg.sub_bus_num;
						ptr_bus->bus_next=(struct pci_bus *)
															 pool_alloc(dev_mem.ptr_pool_others, 1, 
															 OTHER_POOL_GRAN);
						if (ptr_bus->bus_next==NULL) {
							low_putstr("Heap allocation failed.");
							while (1)
								;
						}
						ptr_bus=ptr_bus->bus_next;
					}
					else {
						z=6; // number of configuration registers
						ptr_dev->dev_class=IS_DEV;
					}
					for (o=0; o<z; o++) {
						/* save configuration registers */
						ptr_dev->dev_regs_save[o]=pci_conf_rd(BASE_REGS+o, m, i, n);
						/* write all ones to obtain size */
						pci_conf_wr(BASE_REGS+o, m, i, n, 0xffffffff);
						/* read devices' size requirenments */
						ptr_dev->dev_regs[o]=pci_conf_rd(BASE_REGS+o, m, i, n);
						/* decode devices' size requirenments */
						switch (ptr_dev->dev_regs[o] & 1) {
							case 0:
								ptr_dev->flags |= MEM_MAPPED;
								ptr_dev->dev_regs[o]&=0xfffffff0;
								ptr_dev->dev_regs[o]=~(ptr_dev->dev_regs[o]);
								ptr_dev->dev_regs[o]++;
								/* restore configuration registers */
								pci_conf_wr(BASE_REGS+o, m, i, n, ptr_dev->dev_regs_save[o]);
								ptr_dev->dev_regs_save[o]&=0xfffffff0;
								break;
							case 1:
								ptr_dev->flags |= IO_MAPPED;
								ptr_dev->dev_regs[o]&=0xfffffffc;
								ptr_dev->dev_regs[o]=~(ptr_dev->dev_regs[o]);
								ptr_dev->dev_regs[o]++;
								ptr_dev->dev_regs[o]&=0xffff;
								/* restore configuration registers */
								pci_conf_wr(BASE_REGS+o, m, i, n, ptr_dev->dev_regs_save[o]);
								ptr_dev->dev_regs_save[o]&=0xfffffffc;
								break;
						}
					}
				}
				/* we have a host bridge */
				else {
					for (o=0; o<6; o++)
						ptr_dev->dev_regs[o] = 0;
					ptr_dev->flags = 0;
				}
				ptr_cfg++; // next devices' configuration space
				ptr_dev++; // next device
				/* multi-function device? */
				if ((((ptr_cfg-1)->pci_lout.pci_dev.type&(~0x7f))==0)
						&& n==0)
					break;
			}
		}
	}

	ptr_cfg->pci_lout.pci_dev.id_vendor=0;
	ptr_bus->bus_next=NULL;
	ptr_dev=NULL;
	ptr_cfg=NULL;
	pci_verbose();
	while (1)
		;

}
