#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#ifndef PCI
#define PCI
#include "dev.h"
#include "pci.h"
#endif
extern struct dev *dev_head;
extern struct pci_conf *conf_head;
extern struct pci_bus *bus_head;

static void
pci_verbose()
{
	struct pci_conf *ptr_cfg; // pointer to configuration record
	struct dev *ptr_dev; // pointer to device record
	struct devs_vendors *ptr_vid;
	struct devs_products *ptr_pid;
	struct dev_maps *ptr_maps;
	word rd_reg; // word read from configuration space
	byte buffer[512];
	uint_t i, m, n, o, l;
	word prnt_w;

	low_putstr("\n");
	low_putstr("Probing devices (assuming PCI as local bus)\n");
	low_putstr("======================================================\n");
	/* be verbose */
	for (ptr_dev=dev_head, ptr_cfg=conf_head; 
			ptr_cfg->pci_lout.pci_dev.id_vendor!=0; ptr_dev++)
	{
		low_putstr("pci");
		for (i=0; i<512; i++)
			buffer[i] = 0;
		memdump32((int *)&(ptr_dev->bus_num), buffer, 1, 1);
		low_putstr(buffer);
		low_putstr(": ");
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
			for (i=0; i<512; i++)
				buffer[i] = 0;
			memdump32((int *)ptr_cfg, buffer, 1, 2);
			low_putstr(buffer);
			low_putstr(" at ");
		}
		/* print bus number */
		for (i=0; i<512; i++)
			buffer[i] = 0;
		memdump32((int *)&ptr_dev->bus_num, buffer, 1, 1);
		low_putstr(buffer);
		low_putstr(":");
		/* print device number */
		for (i=0; i<512; i++)
			buffer[i] = 0;
		memdump32((int *)&ptr_dev->dev_num, buffer, 1, 1);
		low_putstr(buffer);
		low_putstr(":");
		/* print function number */
		for (i=0; i<512; i++)
			buffer[i] = 0;
		memdump32((int *)&ptr_dev->func_num, buffer, 1, 1);
		low_putstr(buffer);
		low_putstr(" ");
		/* find product string and print it */
		if (ptr_vid != NULL) {
			for (ptr_pid=&ptr_vid->devs_products[0]; 
					 ptr_pid->devs_pid!=ptr_cfg->pci_lout.pci_dev.id_product && 
					 ptr_pid->devs_pid!=-1; ptr_pid++)
				;
			low_putstr(ptr_pid->devs_pstr);
			if (ptr_pid->devs_pid == -1) {
				for (i=0; i<512; i++)
					buffer[i] = 0;
				low_putstr(" ");
				memdump32((int*)&(ptr_cfg->pci_lout.pci_dev.id_product), buffer, 1, 2);
				low_putstr(buffer);
			}
		}
		else {
			for (i=0; i<512; i++)
				buffer[i] = 0;
			memdump32((int *)&(ptr_cfg->pci_lout.pci_dev.id_product), buffer, 1, 2);
			low_putstr(buffer);
		}
		/* print revision number */
		low_putstr("\n");
		low_putstr(" Rev=");
		for (i=0; i<512; i++)
			buffer[i] = 0;
		memdump32((int *)&(ptr_cfg->pci_lout.pci_dev.id_rev), buffer, 1, 1);
		low_putstr(buffer);
		low_putstr(" Class=");
		/* print device-class specific information */
		switch (ptr_cfg->pci_lout.pci_dev.class_code[2]) {
			case 0x00:
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						break;
					case 0x01:
						low_putstr("<VGA-compatible Controller>");
						break;
				}
				break;
			case 0x01:
				low_putstr("<Mass Storage Controller>,");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<SCSI Bus Controller>");
						break;
					case 0x01:
						low_putstr("<IDE Controller>");
						break;
					case 0x02:
						low_putstr("<Floppy Disk Controller>");
						break;
					case 0x03:
						low_putstr("<IPI Bus Controller>");
						break;
					case 0x04:
						low_putstr("<RAID Controller>");
						break;
					case 0x05:
						low_putstr("<ATA Controller>,");
						switch (ptr_cfg->pci_lout.pci_dev.class_code[0]) {
							case 0x20:
								low_putstr("<Single DMA>");
								break;
							case 0x30:
								low_putstr("<Chained DMA>");
								break;
						}
						break;
					case 0x06:
						low_putstr("<Serial ATA DPA>");
						break;
					case 0x80:
						low_putstr("<Other Mass Storage Controller>");
						break;
				}
				break;
			case 0x02:
				low_putstr("<Network Controller>,");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<Ethernet Controller>");
						break;
					case 0x01:
						low_putstr("<Token Ring Controller>");
						break;
					case 0x02:
						low_putstr("<FDDI Controller>");
						break;
					case 0x03:
						low_putstr("<ATM Controller>");
						break;
					case 0x04:
						low_putstr("<ISDN Controller>");
						break;
					case 0x05:
						low_putstr("<WorldFip Controller>");
						break;
					case 0x06:
						low_putstr("<PCIMG Controller>");
						break;
					case 0x80:
						low_putstr("<Other Network Controller>");
						break;
				}
				break;
			case 0x03:
				low_putstr("<Display Controller>,");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<VGA-compatible Controller>");
						break;
					case 0x01:
						low_putstr("<XGA Controller>");
						break;
					case 0x02:
						low_putstr("<3D Controller>");
						break;
					case 0x80:
						low_putstr("<Other Display Controller>");
						break;
				}
				break;
			case 0x04:
				low_putstr("<Multimedia Device>,");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<Video Device>");
						break;
					case 0x01:
						low_putstr("<Audio Device>");
						break;
					case 0x02:
						low_putstr("<Computer Telephony Device>");
						break;
					case 0x80:
						low_putstr("<Other Multimedia Device>");
						break;
				}
				break;
			case 0x05:
				low_putstr("<Memory Controller>");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<Flash>");
						break;
					case 0x01:
						low_putstr("<RAM>");
						break;
					case 0x80:
						low_putstr("<Other Memory Controller>");
						break;
				}
				break;
			case 0x06:
				low_putstr("<Bridge Device>,");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<Host Bridge>");
						ptr_dev->flags |= DEV_IS_HOST;
						break;
					case 0x01:
						low_putstr("<ISA Bridge>");
						ptr_dev->flags |= DEV_IS_ISA;
						break;
					case 0x02:
						low_putstr("<EISA Bridge>");
						break;
					case 0x03:
						low_putstr("<MCA Bridge>");
						break;
					case 0x04:
						low_putstr("<PCI-PCI Bridge>,");
						ptr_dev->flags |= DEV_IS_PCI;
						switch (ptr_cfg->pci_lout.pci_dev.class_code[0]) {
							case 0x00:
								low_putstr("<~Subtractive Decode>");
								break;
							case 0x01:
								low_putstr("<Subtractive Decode>");
								break;
						}
						low_putstr("\n");
						low_putstr(" ");
						break;
					case 0x05:
						low_putstr("<PCMCIA Bridge>");
						break;
					case 0x06:
						low_putstr("<NuBus Bridge>");
						break;
					case 0x07:
						low_putstr("<CardBus Bridge>");
						break;
					case 0x08:
						low_putstr("<PCMCIA Bridge>");
						break;
					case 0x09:
						low_putstr("<Rare PCI-PCI Bridge>");
						break;
					case 0x0a:
						low_putstr("<Infiniband-PCI Bridge>");
						break;
					case 0x80:
						low_putstr("<Other Bridge Controller>");
						break;
				}
				low_putstr(" Primary=");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				memdump32((int *)&(ptr_cfg->pci_lout.pci_brg.pri_bus_num), 
					buffer, 1, 1);
				low_putstr(buffer);
				low_putstr(" Secondary=");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				memdump32((int *)&(ptr_cfg->pci_lout.pci_brg.sec_bus_num), 
					buffer, 1, 1);
				low_putstr(buffer);
				low_putstr(" Subordinate=");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				memdump32((int *)&(ptr_cfg->pci_lout.pci_brg.sub_bus_num),buffer,1, 1);
				low_putstr(buffer);
				low_putstr(" IO=");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				prnt_w=ptr_cfg->pci_lout.pci_brg.io_base;
				prnt_w&=0xf0;
				memdump32((int *)&prnt_w,buffer, 1, 1);
				low_putstr(buffer);
				low_putstr("00-");
				prnt_w=ptr_cfg->pci_lout.pci_brg.io_limit;
				prnt_w&=0xf0;
				memdump32((int *)&prnt_w,buffer, 1, 1);
				low_putstr(buffer);
				low_putstr("ff");
				low_putstr(" MEM=");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				prnt_w=ptr_cfg->pci_lout.pci_brg.mem_base;
				prnt_w&=0xfff0;
				memdump32((int *)&prnt_w,buffer, 1, 2);
				low_putstr(buffer);
				low_putstr("0000-");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				prnt_w=ptr_cfg->pci_lout.pci_brg.mem_limit;
				prnt_w&=0xfff0;
				prnt_w|=0x000f;
				memdump32((int *)&prnt_w,buffer, 1, 2);
				low_putstr(buffer);
				low_putstr("ffff");
				low_putstr(" PREFTCH=");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				prnt_w=ptr_cfg->pci_lout.pci_brg.mem_prftch_base;
				prnt_w&=0xfff0;
				memdump32((int *)&prnt_w,buffer, 1, 2);
				low_putstr(buffer);
				low_putstr("0000-");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				prnt_w=ptr_cfg->pci_lout.pci_brg.mem_prftch_limit;
				prnt_w&=0xfff0;
				prnt_w|=0x000f;
				memdump32((int *)&prnt_w,buffer, 1, 2);
				low_putstr(buffer);
				low_putstr("ffff");
				break;
			case 0x07:
				low_putstr("<Simple Communcation Controller>");
				break;
			case 0x08:
				low_putstr("<Base System Peripheral>");
				break;
			case 0x09:
				low_putstr("<Input Device>");
				break;
			case 0x0a:
				low_putstr("<Docking Station>");
				break;
			case 0x0b:
				low_putstr("<Processor>");
				break;
			case 0x0c:
				low_putstr("<Serial Controller>");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<IEEE 1394 FireWire>");
						break;
					case 0x01:
						low_putstr("<ACCESS.bus>");
						break;
					case 0x02:
						low_putstr("<SSA>");
						break;
					case 0x03:
						low_putstr("<USB>,");
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
					case 0x04:
						low_putstr("<Fibre Channel>");
						break;
					case 0x05:
						low_putstr("<SMBus>");
						break;
					case 0x06:
						low_putstr("<InfiniBand>");
						break;
					case 0x07:
						low_putstr("<IPMI>");
						break;
					case 0x08:
						low_putstr("<SERCOS>");
						break;
					case 0x09:
						low_putstr("<CANbus>");
						break;
				}
				break;
			case 0x0d:
				low_putstr("<Wireless Controller>");
				switch (ptr_cfg->pci_lout.pci_dev.class_code[1]) {
					case 0x00:
						low_putstr("<iRDA-compatible Controller>");
						break;
					case 0x01:
						low_putstr("<Consumer IR Controller>");
						break;
					case 0x10:
						low_putstr("<RF Controller>");
						break;
					case 0x11:
						low_putstr("<Bluetooth>");
						break;
					case 0x12:
						low_putstr("<Broadband>");
						break;
					case 0x20:
						low_putstr("<Ethernet (802.11a-5ghz)>");
						break;
					case 0x21:
						low_putstr("<Ethernet (802.11b-2.4ghz)>");
						break;
					case 0x80:
						low_putstr("<Other Wireless Controller>");
						break;
				}
				break;
			case 0x0e:
				low_putstr("<Intelligent Input/Output>");
				break;
			case 0x0f:
				low_putstr("<Satellite Communication Controller>");
				break;
			case 0x10:
				low_putstr("<Encryption/Decryption Controller>");
				break;
			case 0x11:
				low_putstr("<WTF Controller>");
				break;
		}
		if (ptr_cfg->pci_lout.pci_dev.class_code[2]!=0x06) {
			for (o=0; o<6; o++) {
				if (ptr_dev->dev_regs[o]==0)
					continue;
				for (i=0; i<512; i++)
					buffer[i] = 0;
				if (ptr_dev->flags==MEM_MAPPED)
					low_putstr("MEM=");
				else
					low_putstr("IO=");
				memdump32((int *)&(ptr_dev->dev_regs_save[o]), buffer, 1, 4);
				low_putstr(buffer);
				low_putstr("-");
				for (i=0; i<512; i++)
					buffer[i] = 0;
				prnt_w=ptr_dev->dev_regs_save[o]+ptr_dev->dev_regs[o];
				memdump32((int *)&(prnt_w), buffer, 1, 4);
				low_putstr(buffer);
				low_putstr(" ");
			}
		}
		low_putstr("\n");
		ptr_cfg++;
	}
}
