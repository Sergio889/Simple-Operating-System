/*
 * Data structures for utils programs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#define ENTS_DIR_PER_SEC 32 /* how many directory entries per sector */
#define ENTS_IVEC_LIST_PER_SEC 32 /* how many directory entries per sector */
#define ENTS_IVEC_PER_SEC 256 /* how many ivector entries per sector */
#define ENTS_BITS_PER_SEC 4096 /* how many ivector entries per sector */
#define BLOCK_SIZE 8192 /* minimum file size unit */
#define SECS_PER_ADDR_SP 65535 /* sectors per address space */
#define SEC_SIZE 512 /* sector size */
#define WORD_SIZE 4 /* word size for i386 */
#define PT_OFF 446 /* offset to partition table */
#define FALSE 0
#define ROOT 0
#define TRUE 1
#define INIT_ROOT_ENTS 2

/*
 * Common variables used in utils programs.
 */
int part_fd; /* descriptor for the special file */
int disk_fd; /* descriptor for whole disk special file */
char block[512]; /* buffer to store sectors */
char mbr[512]; /* buffer to store MBR */
char magic_block[513];
char found_magic;
int find_magic(int);
int result;

/*
 * Private local variables.
 */
static unsigned int sb_off_local; /* offset to super block */

///*
// * The ivector is an vector following the super block with 4-byte entries.
// * Each entry is a sector number, that describes an address for an inode.
 //* The following structure is for creating lists of the ivector vector.
// * For example, we want to create a linked-list of free entries in the ivector.
// */
//struct ivec_list {
//	unsigned int ivec_num; /* number that serves as index into the ivector */
//	struct ivec_list *ivec_next; /* next element in the list */
//	struct ivec_list *ivec_prev; /* previous element in the list */
//	int parity;
//};

struct free_blocks_list {
	unsigned int block_num; /* block address in sectors */
	struct free_blocks_list *block_next; /* next free block */
	struct free_blocks_list *block_prev; /* previous free block */
	int parity;
};

/*
 * This structure template describes the superblock.
 */
struct superblock {
	/* Begining of partition in sectors */
	unsigned int sb_part_start;
	/* Offset in sectors to super block from beginning of partition */
	unsigned short sb_off;
	/* Size in sectors of super block */
	unsigned short sb_size;
	/* Total size in sectors of partition (excluding boot blocks) */
	unsigned int sb_part_size_total;
	/* Total size in sectors of partition (excluding boot blocks and metadata) */ 
	unsigned int sb_part_size;
	/* The number of divisions inside the partition (ie. address spaces) */
	unsigned int sb_num_addr_sp;
	/* Offset in sectors to beginning of ivector from partition start */
	unsigned int sb_ivec_off;
	/* 
	 * Size in sectors of ivector.
	 * Since the total amount of ivector entry's might not be a multiple of
	 * SEC_SIZE, the size of ivector is calculated out of 2 components:
	 * First is the amount of sectors necessary to write all ivector entry's.
	 * Second is the offset within the last sector where the last entry is.
	 */
	unsigned int sb_ivec_size_sec; /* unit is sectors (SEC_SIZE) */
	unsigned int sb_ivec_size_rem; /* unit is word size (WORD_SIZE) */
	/* End in sectors of first ivector */
	unsigned int sb_ivec_end; 
	/* Address in sector of free blocks bit map */
	unsigned int sb_free_blocks_map_addr;
	/* Size in sectors of free blocks bit map */
	unsigned int sb_free_blocks_map_size_sec;
	/* Remainder of the bit map in bits */
	unsigned int sb_free_blocks_map_size_rem;
	/*
	 * Padding is 'necessary' because writes to disk must be in multiples of
	 * the sector size.
	 */
	char pad[512-50];
};

/*
 * Beginning of data is implied from the fact that it starts right after
 * the inode.
 */
struct inode {
	char i_type; /* inode represents: DIRECTORY, REGULAR FILE... */
	char i_dat_cont; /* boolean if data is contigious */
	unsigned int i_dsize; /* size of data in sectors */
	unsigned int i_ivec; /* entry number in ivector */
	unsigned int i_pivec; /* entry number in ivector of parent directory */

	/* Specific information according to file type */
	union i_filespec {
		/* Directory specific information */
		struct i_dirspec {
			unsigned int i_dir_ents; /* number of entries inside directory */
			char i_dirname[255]; /* name of directory */
		} i_dirspec;
		/* Regular file specific information */
		struct i_regspec {

		} i_regspec;
	} i_filespec;
	char pad[512-276];
};
#define DIRECTORY 0x01
#define REGULAR 0x02
int add_boot(const char[], int, struct superblock);

int
find_magic(int sb_off_local)
{
	/* 
	 * Read until beginning of useful block (ie. the block following magic block).
	 * Magic block remains untouched.
	 */
	for (sb_off_local=0; sb_off_local<200; sb_off_local++) {
		if ( (result = read(part_fd, block, SEC_SIZE)) == -1) {
			perror("pread()");
			return 0;
		}
		if ( (result=strncmp(block, magic_block, SEC_SIZE)) == 0) {
			found_magic++;
			break;
		}
	}
	if (found_magic == 0) {
		printf("Did not find the magic block!\n");
		printf("Please install first the boot program\n");
		printf("Aborting!\n");
		return 0;
	}

	return sb_off_local; /* off by one */
}

/*
 * This function adds an entry to a directory.
 * It receives the sector describing the start of the ivector, the name
 * of the new entry and the entry's index of the directory where to add 
 * the new entry for extracting the address of it's inode from the ivector.
 */
int
add_boot(const char entry_name[], int parent_dir, struct superblock sb)
{
	int block_int[512/4];
	int *block_int_ptr;
	unsigned int i, inode_sector, ivec_sector_quo_off, data_sector, ivec_sector_mod_off, par_dir_ents, par_dir_ents_quo, par_dir_ents_rem;
	unsigned int new_ivec=0;
	struct inode ino;
	char block[512];
	char *block_ptr;
	unsigned short *block_ptr_short;
	unsigned short block_short;
	int kern_fd;

	/* 
	 * Using the quotient from the above operation, read correct sector by adding
	 * to the ivector start sector the number of sectors where the entry is.
	 */
	pread(part_fd, block_int, SEC_SIZE, (sb.sb_ivec_off*SEC_SIZE));
	block_int_ptr = block_int;

	/*
	 * The sector here assigned to inode_sector represents the relative sector
	 * number of the new entry's parent directory's inode.
	 */
	inode_sector = *block_int_ptr;
	
	/*
	 * Data starts one sector after the corresponding inode.
	 */
	data_sector = inode_sector+1;

	/*
	 * Here the inode of the new entry's parent directory is read for
	 * determining the data's size and number of entries.
	 */
	pread(part_fd, &ino, SEC_SIZE, (sb.sb_ivec_off+inode_sector)*SEC_SIZE);

	/*
	 * The new_ivec variable represents the relative sector of the new entry's
	 * inode.
	 */
	/* Add offset to data sector of parent directory and size of that data */
	new_ivec += data_sector + ino.i_dsize; 

	/*
	 * If the amount of entries is greater than what it fits in a sector,
	 * advance the data_sector variable to the correct sector to read, then
	 * reduce i by the fit value, to serve later as offset to the sector
	 * read.
	 */ 
	data_sector += ino.i_filespec.i_dirspec.i_dir_ents/32;

	/*
	 * Read parent directory data.
	 */
	pread(part_fd, block, SEC_SIZE, (sb.sb_ivec_off+data_sector)*SEC_SIZE);
	block_ptr = block; // pointer to the block just read
	/* point to the entry where 'boot' is being written */
	block_ptr += ((ino.i_filespec.i_dirspec.i_dir_ents+32)%32)*16;
	/* copy entry's name */
	strncpy(block_ptr, "boot", 255);
	block_ptr += 8; // advance to next field
	*block_ptr = 0; // copy parent's ivector
	block_ptr += 4; // advance to next field
	*block_ptr = 1; // copy file's ivector
	/* write entry to root directory */
	pwrite(part_fd, block, SEC_SIZE, (sb.sb_ivec_off+data_sector)*SEC_SIZE);
	printf("Directory boot was added to root's entries at sector %d",
				sb.sb_ivec_off+data_sector);
	printf(" relative to partition start.\n");

	/*
	 * Update the parent's directory inode to reflect a new entry.
	 */
	ino.i_filespec.i_dirspec.i_dir_ents++;
	pwrite(part_fd, &ino, SEC_SIZE, (sb.sb_ivec_off+inode_sector)*SEC_SIZE);
	printf("Parents directory updated.\n");

	/*
	 * Update ivector to reflect new inode.
	 */
	pread(part_fd, block, SEC_SIZE, (sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec)*SEC_SIZE);
	block[2] = block[0]; /* let root grow */
	block[3] = block[1]; /* let root grow */
	block_ptr_short = (unsigned short *)&block[2];
	*block_ptr_short += 8;
	block_short = *block_ptr_short;
	pwrite(part_fd, block, SEC_SIZE, (sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec)*SEC_SIZE);
	printf("Boot's ivector entry 1 (0-relative) with number %hd\n",
				*block_ptr_short);

	/*
	 * Update bit map before writing inode.
	 */
	unsigned char bit_num;
	pread(part_fd, block, SEC_SIZE, (sb.sb_off+sb.sb_size) * SEC_SIZE);
	block_ptr = block;
	block_ptr++;
	bit_num = 1;
	*block_ptr = *block_ptr | bit_num;
	pwrite(part_fd, block, SEC_SIZE, (sb.sb_off+sb.sb_size) * SEC_SIZE);

	/*
	 * Form boot directory inode.
	 */
	ino.i_type = DIRECTORY;
	ino.i_ivec = 1;
	strcpy(ino.i_filespec.i_dirspec.i_dirname, "boot");
	ino.i_pivec = ROOT;
	ino.i_dsize = 1;
	ino.i_filespec.i_dirspec.i_dir_ents = 7;
	ino.i_dat_cont = TRUE;
	/*
	 * Write boot inode.
	 */
	pwrite(part_fd, &ino, SEC_SIZE, (sb.sb_ivec_off+block_short)*SEC_SIZE);
	printf("Boot's inode written to sector %d partition relative\n",
				sb.sb_ivec_off+block_short);
	/*
	 * Update bit map before writing data.
	 */
	pread(part_fd, block, SEC_SIZE, (sb.sb_off+sb.sb_size) * SEC_SIZE);
	block_ptr = block;
	block_ptr+=3;
	bit_num = 1;
	*block_ptr = *block_ptr | bit_num;
	pwrite(part_fd, block, SEC_SIZE, (sb.sb_off+sb.sb_size) * SEC_SIZE);
	/*
	 * Write boot data.
	 */
	pread(part_fd, block, SEC_SIZE, (sb.sb_ivec_off+(block_short+1))*SEC_SIZE);
	block_ptr = block;
	*block_ptr = '.'; /* entry */
	block_ptr += 8; /* next field */
	*block_ptr = 0; /* address space */
	block_ptr += 4; /* next field */
	*block_ptr = 1; /* ivector */
	block_ptr += 4; /* next entry */
	*block_ptr = '.'; /* entry */
	*(block_ptr+1) = '.'; /* entry */
	block_ptr += 8; /* next field */
	*block_ptr = 0; /* address space */
	block_ptr += 4; /* next field */
	*block_ptr = 0; /* ivector */
	pwrite(part_fd, block, SEC_SIZE, (sb.sb_ivec_off+(block_short+1))*SEC_SIZE);

	return 0;
}
