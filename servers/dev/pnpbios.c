/*
 * Plug and Play using the BIOS interface.
 */

#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif
#include "i386/pnp.h"
#include "pnpbios.h"
#include "dev.h"
static char pnp_str[]="$PnP";
static uint_t node_lrgs;
static byte nodes_num;
static ushort_t pnp_ent[2];
static word pnp_biwr;
struct pnp_dev_nod *ptr_pnp;
static char hextoascii[] = "0123456789abcdef";

static void
pnp_get_device_nodes(nodes_num)
{
	uint_t i, o, isa_dec;
	/* buffer to copy all device nodes */
	byte pnp_buf[80]; // XXX try with dinamic arrays
	byte node_num;
	byte *ptr_byte, *ptr_byte1;
	word *ptr_wrd;
	struct pnp_dev_nod *ptr_nod, *ptr_tmp;
	char pnp_id[8];

	low_putstr("\n");
	ptr_pnp=NULL;
	for (i=0; i<nodes_num; i++) {
		node_num=i;
		low_pnp(pnp_ent[0], pnp_ent[1], 7, 1, 2, 1, 1, &node_num, node_lrgs,
						pnp_buf, 1, pnp_biwr);
		ptr_nod=(struct pnp_dev_nod *)
						pool_alloc(dev_mem.ptr_pool_others, 1, OTHER_POOL_GRAN);
		if (ptr_nod==NULL)
			low_putstr("It's NULL");
		ptr_byte=pnp_buf;
		ptr_byte1=(byte *)ptr_nod;
		for (o=0; o<pnp_buf[0]; o++)
			*ptr_byte1++=*ptr_byte++;
		/* head of PnP nodes list */
		if (ptr_pnp==NULL)
			ptr_pnp=ptr_nod, ptr_nod->pnp_next=NULL;
		else {
			ptr_tmp=ptr_pnp;
			while (ptr_tmp->pnp_next!=NULL)
				ptr_tmp=ptr_tmp->pnp_next;
			ptr_tmp->pnp_next=ptr_nod;
			ptr_nod->pnp_next=NULL;
		}
	}
	for (i=1, ptr_nod=ptr_pnp; ptr_nod!=NULL; ptr_nod=ptr_nod->pnp_next, i++) {
		kprintf("\npnpbios: Device %1d\n", &i);
		ptr_byte=(byte *)ptr_nod;
		ptr_byte+=3; // point to product identifier
		pnp_id[0]='@'+((*ptr_byte&0x7c)>>2);
		pnp_id[1]='@'+(((*ptr_byte&0x3)<<3)+(((*(ptr_byte+1))&0xe0)>>5));
		pnp_id[2]='@'+((*(ptr_byte+1))&0x1f);
		pnp_id[3]=hextoascii[(*(ptr_byte+2))>>4];
		pnp_id[4]=hextoascii[(*(ptr_byte+2))&0xf];
		pnp_id[5]=hextoascii[(*(ptr_byte+3))>>4];
		pnp_id[6]=hextoascii[(*(ptr_byte+3))&0xf];
		pnp_id[7]=0;
		kprintf(pnp_id);
		ptr_byte1=(byte *)ptr_nod;
		ptr_byte1+=7; // point to device type
		kprintf(" Base Type:%1d", ptr_byte1);
		ptr_byte1++; // point to device type
		kprintf(" Sub Type:%1d", ptr_byte1);
		ptr_byte1++; // point to device type
		kprintf(" Interface:%1d", ptr_byte1);
		ptr_byte1++;
		kprintf(" Attribute:%1d", ptr_byte1);
		ptr_byte1+=2; // beginning of resources
		/* until not end-of-resources tag */
		while ((((*ptr_byte1)>>3)&0xf)!=0xf) {
			kprintf("\n ");
			ptr_byte=ptr_byte1; // work with ptr_byte
			if (*ptr_byte1&0x80) {
				kprintf("Large Resource: ");
				ptr_wrd=(word *)(ptr_byte1+1);
				ptr_byte1+=(*ptr_wrd)&0xff; // point to next resource
			}
			else {
				kprintf("Small Resource: ");
				ptr_byte1+=((*(ptr_byte1))&0x7)+1; // point to next resource
				while (ptr_byte<ptr_byte1) {
					switch (((*ptr_byte)>>3)&0xf) {
						case 0x0:
							low_putstr("0");
							ptr_byte=ptr_byte1;
							break;
						case 0x1:
							low_putstr("1");
							ptr_byte=ptr_byte1;
							break;
						case 0x2:
							low_putstr("2");
							ptr_byte=ptr_byte1;
							break;
						case 0x3:
							low_putstr("3");
							ptr_byte=ptr_byte1;
							break;
						case 0x4:
							ptr_byte++;
							kprintf("IRQ mask: %1x", ptr_byte);
							ptr_byte=ptr_byte1;
							break;
						case 0x5:
							ptr_byte++;
							kprintf("DMA mask: %1x", ptr_byte);
							ptr_byte++;
							kprintf("DMA byte: %1x", ptr_byte);
							ptr_byte=ptr_byte1;
							break;
						case 0x6:
							low_putstr("6");
							ptr_byte=ptr_byte1;
							break;
						case 0x7:
							low_putstr("7");
							ptr_byte=ptr_byte1;
							break;
						case 0x8:
							ptr_byte++;
							kprintf("DMA mask: %1x", ptr_byte);
							ptr_byte++;
							kprintf("DMA byte: %1x", ptr_byte);
							ptr_byte=ptr_byte1;
							break;
						case 0x9:
							ptr_byte++;
							if ((*ptr_byte)&1)
								low_putstr("IO 16-bit");
							else
								low_putstr("IO 10-bit");
							ptr_byte++;
							kprintf("MIN base: %1x", ptr_byte);
							ptr_byte+=2;
							kprintf("MAX base: %1x", ptr_byte);
							ptr_byte+=3;
							kprintf("Len: %1d", ptr_byte);
							ptr_byte=ptr_byte1;
							break;
						case 0xa:
							low_putstr("a");
							ptr_byte=ptr_byte1;
							break;
						case 0xb:
							low_putstr("b");
							ptr_byte=ptr_byte1;
							break;
						case 0xc:
							low_putstr("c");
							ptr_byte=ptr_byte1;
							break;
						case 0xd:
							low_putstr("d");
							ptr_byte=ptr_byte1;
							break;
						case 0xe:
							low_putstr("e");
							ptr_byte=ptr_byte1;
							break;
						case 0xf:
							low_putstr("END");
							ptr_byte=ptr_byte1;
							break;
					}
				}
			}
		}
	}
	low_putstr("\n");
}

void
pnp_bios()
{
	ubyte_t num_nodes; 
	uint_t i, o;
	byte pnp_size;
	byte *ptr_pnp, *ptr_tmp;
	short *ptr_sh;
	word *ptr_wr;
	uint_t node_size;

	/* find the PnP signature */
	for (i=PNP_BIOS_START; i<PNP_BIOS_END; i+=16) {
		ptr_pnp=(byte *)i;
		if (*ptr_pnp=='$' && *(ptr_pnp+1)=='P' && *(ptr_pnp+2)=='n' && 
				*(ptr_pnp+3)=='P')
			break;
	}
	/* no PnP BIOS found */
	if (i==PNP_BIOS_END)
		low_putstr("BIOS is not pnp!\n\n");
	/* we have a BIOS supporting PnP interface */
	else {
		kprintf("Found pnp BIOS at %x", &i);
		ptr_tmp=ptr_pnp;
		ptr_tmp+=4;
		low_putstr(" ver. ");
		if (*ptr_tmp==0x10)
			low_putstr("1.0");
		else
			low_putstr("Unknown");
		ptr_tmp++;
		pnp_size=*ptr_tmp; // store structure's size

		ptr_tmp+=8;
		ptr_sh=(short *)ptr_tmp;
		pnp_ent[0] = *ptr_sh;
		pnp_ent[1] = *(ptr_sh+1);
		for (o=0; o<512; o++)
			buffer[o]=0;
		kprintf(" entry point: %2x:%2x\n", pnp_ent, &(pnp_ent[1]));
		ptr_tmp+=12;
		ptr_wr=(word *)ptr_tmp;
		pnp_biwr=*ptr_wr;
		pnp_biwr&=0x0000ffff;
		num_nodes=0;
		node_size=0;
		/* pnp_off, pnp_seg, num_args, bufs_off, num_bufs, ... */
		low_pnp(pnp_ent[0], pnp_ent[1], 6, 1, 2, 0, 1, &num_nodes, 4, &node_size, 
						pnp_biwr);
		nodes_num=num_nodes;
		node_lrgs=node_size;
		kprintf(" Found %1d", &num_nodes);
		kprintf(" devices\n Largest is %1d bytes.", &node_lrgs);
		pnp_get_device_nodes(2);
	}
}
