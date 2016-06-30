/*
 * Common routines for the SOS file-system.
 */
#include "SOSfs.h"
#include "com.h"
#include "bios.h"

unsigned char sec_buf[512]={-1}; /* buffer to store sectors */
short ivec_buf[256]={-1}; /* buffer to store sectors */

void
sosfs_ifill()
{

}
/*
 * Reads 1 sector from disk's LBA rd_sec and writes it to buf_fill.
 * This routine cooperates with low-level services provided by bios.s
 */
void
sosfs_read_raw(unsigned int rd_sec, byte *buf_fill)
{
	struct ext_rw_dap ext_rw_packet;

	ext_rw_packet.dap_size = 16; // 16 bytes
	ext_rw_packet.res_1 = NULL; // reserved
	ext_rw_packet.res_2 = NULL; // reserved
	ext_rw_packet.dap_blknum = 1;
	ext_rw_packet.dap_abs_lba[0] = rd_sec;
	ext_rw_packet.dap_abs_lba[1] = NULL;

	byte temp_buf[512];
	/*
	 * Use BIOS interrupts to read a sector filling the buffer.
	 */
	bios_ext_rd(rd_sec, buf_fill, &ext_rw_packet); // read MBR to buf_fill
}

/*
 * Read the superblock to buffer.
 */
void
sosfs_find_sb(byte *buffer)
{
	byte cmp_buf[512]; // buffer with magic string
	byte temp_buf1[512];
	byte *ptr_cmp=cmp_buf, *ptr_buffer=temp_buf1;
	uint_t count=0, count1=0, count2=0;  // for loop counting and sector counting

	do {
		sosfs_read_raw(part_abs_sec+count, temp_buf1);
		count++;
		for (count1=0, ptr_buffer=temp_buf1; *ptr_buffer++ == 'a'; count1++)
			if (count1 > 500)
				goto out_out;

	} while (1);
out_out:
	sosfs_read_raw(part_abs_sec+count, buffer);
}

/*
 * Read the MBR and find SOS's corresponding partition.
 */
void
sosfs_find_part()
{
	byte mbr_local[512]; // buffer for reading MBR
	byte temp_buffer[512];
	byte *ptr_mbr = mbr_local; // pointer to MBR's buffer

	/* read first sector to mbr_local */
	sosfs_read_raw(0, mbr_local);

	/* find SOS's entry in MBR */
	for (ptr_mbr+=446+4; *ptr_mbr != SOS_FS; ptr_mbr+=16)
		;
	ptr_mbr+=4; // point to absolute sector start of partition

	part_abs_sec = (uint_t)*((uint_t *)ptr_mbr); // store absolute start of SOS's partition (LBA)
}

/*
 * Reads from an ivector number and index and return the entry number in the
 * ivector.
 */
void
sosfs_read_ivec(uint_t ivec, uint_t ivec_indx, short *ivec_buf)
{
	uint_t ivec_indx_local = ivec_indx; // copy of ivector number
	uint_t ivec_local = ivec; // copy of ivector number
	unsigned int ivec_sec_num=ivec_first_abs_sec; // start from first ivector

	byte local_buf[512]; // sector buffer
	byte *ptr_buf = local_buf; // pointer to sector buffer

	/* go to correct ivector */
	while (ivec_local--)
		ivec_sec_num += NEXT_IVEC;

	/*
	 * Increment the sector to load from the ivector according to the index
	 * given.
	 */
	while ( (ivec_indx_local /= 256) != 0)
		ivec_sec_num++;

	sosfs_read_raw(ivec_sec_num, (byte *)ivec_buf);
}

/*
 * Receives a path and returns the correct entry and ivector.
 */
void
sosfs_lookup(char *fp, uint_t ret_ivec[2])
{
	uint_t fp_len, fp_len_tmp, count_temp; // filepath lenght
	unsigned char num_slsh = 0; // number of names to resolve
	uint_t ivec_cur = 0; // current ivector
	uint_t ivec_cur_cpy = 0; // current ivector copy
	uint_t ivec_indx = 0; // current index
	uint_t ivec_indx_cpy = 0; // current index copy
	/* absolute sector for current ivector */
	uint_t ivec_cur_abs_sec = ivec_first_abs_sec; 
	uint_t num_ents_cur, cur_file_size, count, path_found=FALSE;
	char *ptr_fp; // pointer to path passed as argument
	byte name_temp[MAX_FILE_LEN]; // buffer to store file name
	byte *ptr_name = name_temp; // pointer to file name buffer
	byte sec_buf[512]; // buffer for reading sectors
	byte temp_temp[512]; // buffer for reading sectors
	byte *ptr_buf = sec_buf; // pointer to sector buffer
	struct inode cur_ino; // structure to hold current inode
	uint_t i=0;

	/* if invalid path */
	if (*fp != '/') {
		//kprintf("Path must be absolute\n");
		ret_ivec[0] = ret_ivec[1] = -1;
	}

	/* else valid path */
	else {
		 /* if it's root */
		if ( (fp_len = strnlen(fp, 255)) == 1)
			ret_ivec[0] = ret_ivec[1] = 0;
		/* else it has more than 1 slash */
		else {
			/* get number of names to resolve */
			for (fp_len_tmp=fp_len, ptr_fp=fp; fp_len_tmp > 0; fp_len_tmp--) {
				if (*ptr_fp++ == '/') 
					num_slsh++;
			}
			/* resolve each name */
			for (ptr_fp = fp; num_slsh > 0; num_slsh--) {
				/* get absolute sector for beginning of ivector and... */
				while (ivec_cur_cpy--)
					ivec_cur_abs_sec+=NEXT_IVEC;
				/* ...add the number of sectors relative to the index */
				while ( (ivec_indx_cpy /= 256) != 0)
					ivec_cur_abs_sec++;
				/* read the appropiate sector inside the appropiate ivector */
				sosfs_read_ivec(ivec_cur, ivec_indx, ivec_buf);
				/* read the file's inode */
				sosfs_read_raw(ivec_cur_abs_sec+ivec_buf[ivec_indx%256], 
											 (byte *)&cur_ino);
				/* if not end of path and not directory */
				if ((num_slsh > 0) && (cur_ino.i_type != DIRECTORY)) {
					low_putstr("No directory at not end of path\n");
					ret_ivec[0] = ret_ivec[1] = -1;
					break;
				}
				/* copy name to buffer */
				for (ptr_name=name_temp; *(++ptr_fp) != '/' && *ptr_fp != '\0';)
					*ptr_name++ = *ptr_fp;
				*ptr_name = '\0';
				/* 
				 * Loop current-file-size-in-sectors times looking for entry.
				 * We know data follows inode sector.
				 */
				for (cur_file_size=cur_ino.i_dsize, count=1; (cur_file_size>0) && (path_found==FALSE); cur_file_size--, count++) {
					sosfs_read_raw((ivec_cur_abs_sec+ivec_buf[ivec_indx%256])+count, sec_buf);
					ptr_buf = sec_buf; // pointer to the sector just read
					/* loop through number-of-entries-in-one-sector times */
					for (num_ents_cur=cur_ino.i_filespec.i_dirspec.i_dir_ents; (num_ents_cur>0) && (cur_ino.i_filespec.i_dirspec.i_dir_ents-num_ents_cur < 32); num_ents_cur--) {
						/* if entry matches */
						if ( (strncmp(ptr_buf, name_temp, 255)) == 0) {
							path_found=TRUE; 
							break;
						}
						ptr_buf += 16; // next entry string
					}
				}
				/* if we found a matching entry */
				if (path_found == TRUE) {
					path_found = FALSE;
					/* get new values for ivector and index */
					ivec_cur = ivec_cur_cpy = ret_ivec[0] = (uint_t)ptr_buf[8];
					ivec_indx = ivec_indx_cpy = ret_ivec[1] = (uint_t)ptr_buf[12];
				}
				else {
					low_putstr("Entry ");
					low_putstr(fp);
					low_putstr(" not found.");
					ret_ivec[0] = ret_ivec[1] = -1;
					break;
				}
				unsigned int ivec_cur_abs_sec = ivec_first_abs_sec;
			}
		}
	}
}

/*
 * From a path return the absolute sector of its inode.
 */
unsigned int
sosfs_iget(char *path)
{
	byte sec_local[512]; // sector buffer
	ushort_t *ptr_sec=(ushort_t *)sec_local; // pointer to sector buffer
	uint_t ivec_both[2]; // ivector number and index to inode
	uint_t i_abs_sec; // absolute sector for requested inode
	uint_t ivec_indx;
	uint_t count; // counter
	uint_t ivec_sec_loc; // absolute sector for ivector

	/* get ivector number and index in it */
	sosfs_lookup(path, ivec_both);

	/* get absolute sector for ivector */
	for (count=0, ivec_sec_loc=ivec_first_abs_sec; count++ < ivec_both[0] ;)
		ivec_sec_loc += NEXT_IVEC;

	/* get absolute sector for index number */
	for (ivec_indx=ivec_both[1]; (ivec_indx/=256) != 0; ivec_sec_loc++)
		;

	/* read the sector */
	sosfs_read_raw(ivec_sec_loc, sec_local);
 
	/* modulo 256 gives the entry number inside the sector read */
	ptr_sec += ivec_both[1] % 256;
	/* absolute sector is absolute sector to ivector+inode sector in ivector */
	i_abs_sec = (uint_t)*(ptr_sec)+ivec_sec_loc;

	return i_abs_sec;
}

/*
 * Called by start.S to read in the first ivector of the file-system.
 */
void
sosfs_ivec_first()
{
	char temp_20[512];
	uint_t i;
	uint_t *ptr_sec;
	/* find partition entry in MBR and copy the absolute sector */
	sosfs_find_part();
	/* find the superblock sector and copy it to sec_buf */
	sosfs_find_sb(sec_buf);
	/* copy the absolute sector of the first ivector from superblock */
	ptr_sec = (uint_t *)(sec_buf+20);
	ivec_first_abs_sec = (*ptr_sec)+part_abs_sec;
}
