/*
 * Simple utility that does the following:
 * 1. Reads sectors from partition until magic block is found.
 * 2. Reads super block to determine offset to ivector.
 * 3. Reads the offset to root's inode.
 * 4. Determines if boot directory is present in root.
 * 5. If not, creates the entry boot in root.
 * 6. Determines if kernel file is present in boot.
 * 7. If not, copies the kernel to boot.
 */
#include "support.h"
char *ptr_names[] = { "kernel", "proc_serv", "vm_serv", "fs_serv", "dev_serv" };

#define SEC_SIZE 512
#define NUM_INSTS 5
extern int part_fd; /* descriptor for the special file */
extern char block[512]; /* buffer to store sectors */
extern char magic_block[513];
extern char found_magic;
extern int find_magic(int);

static void instructions();

int
main(int argc, char *argv[])
{

	/* control variables */
	unsigned int i, sb_off_local, result, ivec_off_local, inode_off, data_off;
	unsigned int boot_ivector, count, count1;

	/* fundamental file-system structures */
	struct superblock root_sb; // instance of superblock
	struct inode ino; // instance of inode

	/* address variables */
	char *block_ptr;
	unsigned short *block_ptr_short;
	unsigned short block_short;
	int kserv_fd;

	/* buffer variables */
	unsigned int block_int[512/4];
	unsigned short block_short_vec[512/2];
	char block1[512];
	char block2[512];
	char block3[512];

	if (argc != 7) {
		instructions();
		return 0;
	}

	/* Construct magic block */
	for (i=0; i<=511; i++)
		magic_block[i] = 'a';
	magic_block[512] = '\0';

	/* Open partition's special file */
	if ( (part_fd = open(argv[1], O_RDWR)) == -1) {
		perror("open()");
		return 0;
	}

	sb_off_local = find_magic(sb_off_local);
	if (sb_off_local == 0)
		return 0;
	sb_off_local++; /* off by one */

	/*
	 * Be verbose.
	 */
	printf("\nInstalling user-level servers...");
	printf("\n==========================================================\n");
	printf("Found magic block at offset %d from beginning of partition\n",
				sb_off_local);

	/*
	 * Read superblock.
	 */
	if ( (pread(part_fd, &root_sb, SEC_SIZE, sb_off_local*SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}

	/*
	 * Read first ivector.
	 */
	if ( (pread(part_fd, block_int, SEC_SIZE, root_sb.sb_ivec_off*SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}

	inode_off = block_int[0]; /* sector offset to root inode */
	data_off = inode_off+1; /* sector offset to root data */

	/*
	 * Read root inode.
	 */
	pread(part_fd, &ino, SEC_SIZE, (root_sb.sb_ivec_off+inode_off)*SEC_SIZE);

	/*
	 * Read root data.
	 */
	pread(part_fd, block, SEC_SIZE, (root_sb.sb_ivec_off+data_off)*SEC_SIZE);

	/* Find boot entry in root directory */
	for (i=ino.i_filespec.i_dirspec.i_dir_ents, block_ptr=block; i>0; i--) {
		if ( (result = strncmp(block, "boot", 255)) == 0)
			break;
		block_ptr += 16;
	}

	/* No boot entry */
	if (i == 0) 
		boot_ivector = add_boot("boot", ROOT, root_sb);
	else 
		printf("Found boot at ivector 1 number %d\n", *(block_ptr+12));

	/* Update ivector */
	if ( (pread(part_fd, block, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size+root_sb.sb_free_blocks_map_size_sec)*SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}
	block[4] = block[2]; /* let previous entry grow */
	block[5] = block[3]; /* let previous entry grow */
	block_ptr_short = (unsigned short *)&(block[4]);
	*block_ptr_short +=8;
	if ( (pwrite(part_fd, block, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size+root_sb.sb_free_blocks_map_size_sec)*SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}
	/*
	 * Update bit map before writing inode.
	 */
	block_short = *block_ptr_short;
	unsigned char bit_num;
	pread(part_fd, block, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size) * SEC_SIZE);
	block_ptr = block;
	block_ptr+=2;
	bit_num = 1;
	*block_ptr = *block_ptr | bit_num;
	pwrite(part_fd, block, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size) * SEC_SIZE);
	/*
	 * Write inode.
	 */
	if ( (pread(part_fd, &ino, SEC_SIZE, (root_sb.sb_ivec_off+block_short)*SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}
	ino.i_type = REGULAR;
	ino.i_dat_cont = TRUE;
	ino.i_ivec = 2; /* ivector */
	ino.i_pivec = 1; /* parent's ivector (boot) */
	ino.i_dsize = 0; /* for now */
	if ( (pwrite(part_fd, &ino, SEC_SIZE, (root_sb.sb_ivec_off+block_short)*SEC_SIZE)) == -1) {
		perror("pwrite()");
		return 0;
	}

	if ( (pread(part_fd, block, SEC_SIZE, root_sb.sb_ivec_off*SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}
	block_ptr_short = block;
	block_ptr_short += 2;
	/*
	 * Copy kernel to disk.
	 */
	for (count1=0; count1<NUM_INSTS; count1++) {
		if ( (kserv_fd = open(argv[2+count1], O_RDONLY)) == -1) {
			perror("open()");
			return 0;
		}
		for (count=0, i=1; 1; count++) {
			i=read(kserv_fd, block1, SEC_SIZE);
			if (i == 0)
				break;
			/* Write sector */
			/*
			 * Update bit map before writing data.
			 */
			pread(part_fd, block2, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size) * SEC_SIZE);
			block_ptr = block2+count1;
			block_ptr += 4;
			block_ptr += count/8;
			/*
			 * Ugly.
			 */
			switch ((count+8)%8) {
				case 0:
					*block_ptr = *block_ptr | 1;
					break;
				case 1:
					*block_ptr = *block_ptr | 2;
					break;
				case 2:
					*block_ptr = *block_ptr | 4;
					break;
				case 3:
					*block_ptr = *block_ptr | 8;
					break;
				case 4:
					*block_ptr = *block_ptr | 16;
					break;
				case 5:
					*block_ptr = *block_ptr | 32;
					break;
				case 6:
					*block_ptr = *block_ptr | 64;
					break;
				case 7:
					*block_ptr = *block_ptr | 128;
					break;
			}
			/*
			 * Write bit map.
			 */
			pwrite(part_fd, block2, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size) * SEC_SIZE);
			/*
			 * Update inode.
			 */
			if ( (pread(part_fd, &ino, SEC_SIZE, (root_sb.sb_ivec_off+(*(block_ptr_short+count1)))*SEC_SIZE)) == -1) {
				perror("pread()");
				return -1;
			}
			/* initialize inode for each file except kernel */
			if ((count == 0) && (count1 > 0)) {
				ino.i_type = REGULAR;
				ino.i_dat_cont = TRUE;
				ino.i_ivec = 2; /* ivector */
				ino.i_pivec = 1; /* parent's ivector (boot) */
				ino.i_dsize = 0; /* for now */
			}
			ino.i_dsize++;
			if ( (pwrite(part_fd, &ino, SEC_SIZE, (root_sb.sb_ivec_off+(*(block_ptr_short+count1)))*SEC_SIZE)) == -1) {
				perror("pwrite()");
				return -1;
			}
			/*
			 * Write data.
			 */
			if ( (pwrite(part_fd, block1, SEC_SIZE, (root_sb.sb_ivec_off+(*(block_ptr_short+count1)+1+count))*SEC_SIZE)) == -1) {
				perror("pwrite()");
				return -1;
			}
		}
		/* Update ivector */
		if ( (pread(part_fd, block3, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size+root_sb.sb_free_blocks_map_size_sec)*SEC_SIZE)) == -1) {
			perror("pread()");
			return 0;
		}
		block_ptr_short = block3;
		block_ptr_short+=2;
		block3[(count1*2)+6] = block3[2]; /* let previous entry grow */
		block3[((count1*2)+1)+6] = block3[3]; /* let previous entry grow */
		*(block_ptr_short+count1+1) = (*(block_ptr_short+count1))+(count+1);
		if ( (pwrite(part_fd, block3, SEC_SIZE, (root_sb.sb_off+root_sb.sb_size+root_sb.sb_free_blocks_map_size_sec)*SEC_SIZE)) == -1) {
			perror("pread()");
			return 0;
		}
	}
		/*
		 * Add entry of kernel in boot directory's data.
		 */
		pread(part_fd, block_short_vec, SEC_SIZE, root_sb.sb_ivec_off*SEC_SIZE);
		block_short = block_short_vec[1];
		/* Read data */
		pread(part_fd, block, SEC_SIZE, (root_sb.sb_ivec_off+(block_short+1))*SEC_SIZE);
		block_ptr = block+32;
		block_ptr[0] = 'k';
		block_ptr[1] = 'e';
		block_ptr[2] = 'r';
		block_ptr[3] = 'n';
		block_ptr[4] = 'e';
		block_ptr[5] = 'l';
		block_ptr[6] = '\0';
		block_ptr += 8; // ivector number
		*block_ptr = 0; // first ivector
		block_ptr += 4; // ivector entry
		*block_ptr = 2; // first entry
		block_ptr += 4; // next entry
		block_ptr[0] = 'p';
		block_ptr[1] = 'r';
		block_ptr[2] = '_';
		block_ptr[3] = 's';
		block_ptr[4] = 'e';
		block_ptr[5] = 'r';
		block_ptr[6] = 'v';
		block_ptr[7] = '\0';
		block_ptr += 8; // ivector number
		*block_ptr = 0; // first ivector
		block_ptr += 4; // ivector entry
		*block_ptr = 3; // first entry
		block_ptr += 4; // next entry
		block_ptr[0] = 'v';
		block_ptr[1] = 'm';
		block_ptr[2] = '_';
		block_ptr[3] = 's';
		block_ptr[4] = 'e';
		block_ptr[5] = 'r';
		block_ptr[6] = 'v';
		block_ptr[7] = '\0';
		block_ptr += 8; // ivector number
		*block_ptr = 0; // first ivector
		block_ptr += 4; // ivector entry
		*block_ptr = 4; // first entry
		block_ptr += 4; // next entry
		block_ptr[0] = 'f';
		block_ptr[1] = 's';
		block_ptr[2] = '_';
		block_ptr[3] = 's';
		block_ptr[4] = 'e';
		block_ptr[5] = 'r';
		block_ptr[6] = 'v';
		block_ptr[7] = '\0';
		block_ptr += 8; // ivector number
		*block_ptr = 0; // first ivector
		block_ptr += 4; // ivector entry
		*block_ptr = 5; // first entry
		block_ptr += 4; // next entry
		block_ptr[0] = 'd';
		block_ptr[1] = 'v';
		block_ptr[2] = '_';
		block_ptr[3] = 's';
		block_ptr[4] = 'e';
		block_ptr[5] = 'r';
		block_ptr[6] = 'v';
		block_ptr[7] = '\0';
		block_ptr += 8; // ivector number
		*block_ptr = 0; // first ivector
		block_ptr += 4; // ivector entry
		*block_ptr = 6; // first entry
		block_ptr += 4; // next entry
		pwrite(part_fd, block, SEC_SIZE, (root_sb.sb_ivec_off+(block_short+1))*SEC_SIZE);
}


static
void instructions()
{
	printf("Partition+kernel_path\n");
}
