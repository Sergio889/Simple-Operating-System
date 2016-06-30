#include "dev.h"
#include "i386/smbios.h"

ubyte_t sm_cnts[NUM_TYPES];
struct sm_local *sm_addrs[NUM_TYPES]={(struct sm_local *)-1};
struct sm_hdr *ptr_hdr;

static void
smbios_print()
{
	struct sm_hdr *ptr_hdr;
	struct sm_local *ptr_loc;
	ubyte_t *ptr_tmp, *ptr_tmp1, *ptr_str;
	ubyte_t i, go_next;
	uint_t o, m, s;

	low_putstr("\n");

	/* traverse the entries and print information */
	for (i=0; i<sm_bios.sm_num; i++) {
		go_next=0;
		for (ptr_loc=sm_addrs[i]; ptr_loc!=NULL; ptr_loc=ptr_loc->sm_next) {
			ptr_hdr=(struct sm_hdr *)ptr_loc->sm_addr;
			ptr_str=(byte *)(((uint_t)ptr_hdr)+ptr_hdr->sm_len);
			ptr_tmp1=(byte *)ptr_hdr;
			switch (i) {
				case TYPE_BIOS:
					low_putstr("BIOS Information\n");
					for (m=0; m<3; m++) {
						switch (m) {
							case 0:
								ptr_tmp1+=4; // vendor string
								low_putstr(" vendor: ");
								break;
							case 1:
								ptr_tmp1++;
								low_putstr(" version: ");
								break;
							case 2:
								ptr_tmp1+=3;
								low_putstr(" release date: ");
								break;
						}
						ptr_str=(byte *)(((uint_t)ptr_hdr)+ptr_hdr->sm_len);
						for (s=0; s<(*ptr_tmp1-1); s++)
							while (*ptr_str++ != '\0')
								;
						low_putstr(ptr_str);
						low_putstr("\n");
					}
					break;
				case TYPE_SYS:
					low_putstr("System Information\n");
					for (m=0; m<5; m++) {
						switch (m) {
							case 0:
								ptr_tmp1+=4; // vendor string
								low_putstr(" manufacturer: ");
								break;
							case 1:
								ptr_tmp1++;
								low_putstr(" product name: ");
								break;
							case 2:
								ptr_tmp1++;
								low_putstr(" version: ");
								break;
							case 3:
								ptr_tmp1++;
								low_putstr(" serial number: ");
								break;
							case 4:
								ptr_tmp1+=18;
								low_putstr(" family: ");
								break;
						}
						ptr_str=(byte *)(((uint_t)ptr_hdr)+ptr_hdr->sm_len);
						for (s=0; s<(*ptr_tmp1-1); s++)
							while (*ptr_str++ != '\0')
								;
						low_putstr(ptr_str);
						low_putstr("\n");
					}
					break;
				default:
					go_next=1;
			}
			if (go_next)
				break;
		}
	}
}

void
smbios()
{
	uint_t i, o;
	char bcd_rev[2];
	byte *ptr_smbios, *ptr_tmp, *ptr_tmp1;
	short *ptr_sh;
	word *ptr_wr;
	word pnp_biwr;
	ushort_t pnp_ent[2];
	struct smbios *ptr_sm;
	struct sm_local *ptr_hdr_tr;
	struct sm_local *ptr_hdr_tr1;

	/* find the anchor string */
	for (i=SMBIOS_START; i<SMBIOS_END; i+=16) {
		ptr_smbios=(byte *)i;
		if (*ptr_smbios=='_' && *(ptr_smbios+1)=='S' && *(ptr_smbios+2)=='M' && 
				*(ptr_smbios+3)=='_')
			break;
	}
	if (i==SMBIOS_END)
		low_putstr("No SMBIOS table!\n\n");
	/* we have found a SMBIOS compliant BIOS */
	else {
		low_putstr("Found SMBIOS at ");
		for (o=0; o<512; o++)
			buffer[o]=0;
		memdump32((int *)&i, buffer, 1, 4);
		low_putstr(buffer);
		ptr_tmp=(byte *)&sm_bios; // point to structure to fill
		ptr_tmp1=ptr_smbios; // point to SMBIOS structure in RAM
		/* copy SMBIOS to local */
		for (i=0; i<sizeof(struct smbios); i++)
			*ptr_tmp++=*ptr_tmp1++;
		/* print revision */
		bcd_rev[1]=0;
		low_putstr(" revision ");
		bcd_rev[0]=((sm_bios.sm_rev&0xf0)>>4)+0x30;
		low_putstr(bcd_rev);
		low_putstr(".");
		bcd_rev[0]=(sm_bios.sm_rev&0x0f)+0x30;
		low_putstr(bcd_rev);

		/* point to beginning of structures */
		ptr_hdr=(struct sm_hdr *)sm_bios.ptr_tab;
		ptr_hdr_tr=(struct sm_local *)sm_bios.ptr_tab;

		/* initialize values */
		for (i=0; i<127; i++)
			sm_cnts[i]=0;
		for (i=0; i<127; i++)
			sm_addrs[i]=NULL;

		/* save addresses of entries to different lists */
		for (i=0; i<(uint_t)sm_bios.sm_num-1; i++) {
			if (sm_cnts[ptr_hdr->sm_type]<128) {
				/* if structure fits in available space */ 
				if (sm_cnts[ptr_hdr->sm_type]<(OTHER_POOL_GRAN-8)) {
					/* if first element of corresponding list is NULL, allocate */
					if (sm_addrs[ptr_hdr->sm_type]==NULL) {
						sm_addrs[ptr_hdr->sm_type]=(struct sm_local *)
						 pool_alloc(dev_mem.ptr_pool_others, 1, OTHER_POOL_GRAN);
						ptr_hdr_tr=sm_addrs[ptr_hdr->sm_type];
						sm_addrs[ptr_hdr->sm_type]->sm_next=NULL;
					}
					else {
						/* go to last element */
						for (ptr_hdr_tr1=sm_addrs[ptr_hdr->sm_type]; 
								 ptr_hdr_tr1->sm_next!=NULL; ptr_hdr_tr1=ptr_hdr_tr1->sm_next)
						{
							;
						}
						ptr_hdr_tr=ptr_hdr_tr1;
						ptr_hdr_tr++;
						ptr_hdr_tr1->sm_next=ptr_hdr_tr;
						ptr_hdr_tr->sm_next=NULL;
					}
					/* advance counter by structure size */
					sm_cnts[ptr_hdr->sm_type]+=8;
				}
				else {
					/* go to last element */
					for (ptr_hdr_tr1=sm_addrs[ptr_hdr->sm_type]; 
							 ptr_hdr_tr1->sm_next!=NULL; ptr_hdr_tr1=ptr_hdr_tr1->sm_next)
					{
						;
					}
					ptr_hdr_tr=ptr_hdr_tr1;
					/* allocate new structure */
					ptr_hdr_tr->sm_next=(struct sm_local *)
						pool_alloc(pool_alloc(dev_mem.ptr_pool_others, 1, OTHER_POOL_GRAN));
					ptr_hdr_tr=ptr_hdr_tr->sm_next;
					ptr_hdr_tr->sm_next=NULL;
					sm_cnts[ptr_hdr->sm_type]=0; // restart count
				}
				ptr_hdr_tr->sm_addr=(uaddr_t *)ptr_hdr; // save address of structure
				/* find next entry */
				for (ptr_tmp=(byte *)(((uint_t)ptr_hdr)+ptr_hdr->sm_len);1;ptr_tmp++) {
					if (*ptr_tmp==0 && *(ptr_tmp+1)==0) {
						ptr_tmp+=2;
						ptr_hdr=(struct sm_hdr *)ptr_tmp;
						break;
					}
				}
			}
		}
		smbios_print();
	}
}
