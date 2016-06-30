/*
 * File-system's structures.
 */
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
#define DIRECTORY 0x01
#define REGULAR 0x02
#define MAX_FILE_LEN 255
#define SOS_FS 0x50
#define NEXT_IVEC 65535 // sectors between ivectors


/*
 * Common variables used in utils programs.
 */
int part_fd; /* descriptor for the special file */
int disk_fd; /* descriptor for whole disk special file */
//unsigned char sec_buf[512]=; /* buffer to store sectors */
//short ivec_buf[256]=; /* buffer to store sectors */
//char mbr[512]; /* buffer to store MBR */
//char magic_block[513];
char found_magic;
int find_magic(int);
int result;

/*
 * Private local variables.
 */
static unsigned int sb_off_local; /* offset to super block */

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

/*
 * Function declarations.
 */
unsigned int part_abs_sec;
unsigned int ivec_first_abs_sec;
