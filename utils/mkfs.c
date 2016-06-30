/*
 * This program can be executed on any POSIX compliant OS, it creates a 
 * file system.
 * The file system in SOS is organized as follows:
 * The first block written to disk by this program (ie. the beginning of the 
 * file system) is the superblock.
 * The superblock contains, among other things, the number of ivectors 
 * in the partition.
 * The ivectors are structures with 1 entry per sector available in 
 * the address space for data. The whole disk address space is divided in one
 * or more address spaces, according to the number of sectors.
 * Each entry is an address given in sectors and relative to the start of the
 * corresponding address space describing the position of the inode that 
 * describes the file.
 * For using the minimum amount of space per entry, a new address space
 * is formed every (2^16)-1 sectors, and 2 bytes suffices each entry.
 * For example, the first entry in the ivector in the first address space
 * gives the offset to the inode of the root directory, from the beginning of 
 * the first address space, in sector units.
 * When accessing a file, the corresponding address in the corresponding
 * ivector for the inode describing the file must be read; let's hope 
 * it is feasible to mantain the ivectors in main memory for fast access.
 * When a file is created, the allocation policy will select an address space, 
 * next an empty entry is taken from the ivector and, also
 * according to the layout policies, a block is allocated on disk inside the
 * corresponding address space. Then the corresponding inode is written to 
 * the allocated block.
 * This mechanism guarantize that the inodes immediately precedes, at least,
 * the first block of data; it is up to the layout policies to try to keep
 * enough space following the inode for, lets hope, all blocks of data.
 */
#ifndef SUPPORT_H
#define SUPPORT_H
#include "support.h"

/*
 * Variables defined in support.h.
 */
extern int part_fd; /* descriptor for the special file */
extern int disk_fd; /* descriptor for whole disk special file */
extern char block[512]; /* buffer to store sectors */
extern char mbr[512]; /* buffer to store MBR */
extern unsigned int sb_off_local; /* offset to super block */
extern char magic_block[513]; /* buffer to store sector */
extern char found_magic; /* boolean if found magic block */

/*
 * Private variables for mkfs.
 */
static char dir_entries[512]; /* store a directory entry to save to disk */
static char *root_ents[] = {".", ".."}; /* current and parent directory entries */
static unsigned int num_sectors; /* total number of sectors of partition */
static unsigned int i; /* a counter */
static unsigned int part_start_local; /* sector offset to partition start */
static unsigned int part_size; /* size of partition in sectors */
static struct superblock sb; /* structure describing the partition's superblock */
static struct inode i_root; /* inode for root directory */
static struct free_blocks_list free_init;
static char do_you[] = "Do you really want to destroy the current filesystem?\n";
static int buf_int[128]; /* buffer to store sector */

static void instructions();
//static struct ivec_list ivec_init;
//static void init_ivec_free_list(struct superblock *);
static void init_block_free_list(struct superblock *);
static void verbose_percentage(int, int);

int
main(int argc, char *argv[])
{

	/* control variables */
	int result; // store result of various system calls
	int part_number; // copy partition number from argv[3]
	int count;  // counter
	char temp; // destroy partition or not
	int part_entry[4]; // copy partition entry
	int file_desc;

	/* address variables */
	char *dir_entry_tmp; // pointer to dir_entry
	char *part_type_ptr; // pointer to partition type in partition table
	unsigned int *pt_ptr; // pointer to partition table
	unsigned int *empty_ivec; // store empty vector on disk for ivector
	int *buf_int_ptr; // buffer to store sector
	struct free_blocks_list *free_local;
	double percent;

	if (argc != 4) {
		instructions();
		return 0;
	}

	/* Magic block */
	for (i=0; i<=511; i++)
		magic_block[i] = 'a';
	magic_block[512] = '\0';

	/* Open special file representing partition */
	if ( (part_fd = open(argv[1], O_RDWR | O_SYNC)) == -1) {
		perror("open()");
		return 0;
	}
	printf("\nInstalling file system...");
	printf("\n==========================================================\n");
	printf("Using partition %s ", argv[1]);
	/* Open special file representing disk */
	if ( (disk_fd = open(argv[2], O_RDONLY)) == -1) {
		perror("open()");
		return 0;
	}
	printf("From device %s\n", argv[2]);

	sb_off_local = find_magic(sb_off_local);
	if (sb_off_local == 0)
		return 0;
	sb_off_local++; /* off by one */

	/*
	 * Determine total number of sectors.
	 */
	if ( (result = read(disk_fd, mbr, SEC_SIZE)) == -1) {
		perror("pread()");
		return 0;
	}

	/* Convert to partition number */
	part_number = atoi(argv[3]);

	/* Point to partition entry */
 	for (pt_ptr = (unsigned int *)(mbr + PT_OFF); part_number != 0; pt_ptr += 4, part_number--)
		;

	/*
	 * Make sure not to delete sensible data.
	 */
	printf("Partition entry is: \n");
	printf("Partition type -> 0x%x\n", (unsigned char)pt_ptr[1]);
	part_type_ptr = (char *)(pt_ptr[1]); /* save pointer to partition type */
	printf("Starting at sector -> %d\n", pt_ptr[2]);
	printf("And size -> %d (%dMB)\n", pt_ptr[3], ((pt_ptr[3]/1024)*SEC_SIZE)/1024);
	printf("\n");
	if ((unsigned char)pt_ptr[1] != 0) {
		printf("It appears to be a ");
		switch ((unsigned char)pt_ptr[1]) {
			case 0x83:
							printf("Linux ");
							break;
			case 0xa5:
							printf("FreeBSD ");
							break;
			case 0x0c:
							printf("FAT ");
							break;
			case 0x50:
							printf("SOS ");
							break;
			default:
							printf("unknown ");
		}
		printf("partition.\n\n");
		write(1, do_you, strnlen(do_you, 255));
		do {
			printf("\n(y or n)\n-> ");
			scanf("%c", &temp);
		} while(temp != 'y' && temp != 'n');
		if (temp == 'n')
			return 0;
	}
	printf("DESTROYING!!\n"); /* MUAJAJAJAJAJAJAJAJA */

	pt_ptr += 2; /* point to 4 bytes that determine start of partition */

	/*
	 * Start of partition in sectors.
	 */
	part_start_local = *pt_ptr;
	pt_ptr += 1;
	part_size = *pt_ptr;

	/*
	 * Save some values to the superblock structure.
	 */
	sb.sb_part_start = part_start_local;
	sb.sb_off = sb_off_local;
	sb.sb_part_size_total = part_size;
	sb.sb_size += sizeof(sb)/SEC_SIZE;
	/*
	 * Determine the size of one ivector as follows:
	 * Each entry in the ivector is 2 bytes, so 256 entries fit in one sector.
	 * The minimum unit size is BLOCK_SIZE, so we divide the total amount of
	 * sectors in a single address space by BLOCK_SIZE to obtain the number
	 * of blocks in one address space. Finally we divide the number of blocks by
	 * the number of entries in one sector to obtain the number of sectors
	 * necessary to address all blocks in the address space. We add one because
	 * the division might not be exact, and store the remainder.
	 * sb.sb_ivec_size_sec is in sector units.
	 * sb.sb_ivec_size_rem is in entry units (2 bytes).
	 */
	sb.sb_ivec_size_sec = (SECS_PER_ADDR_SP/ENTS_IVEC_PER_SEC)+1;
	sb.sb_ivec_size_rem = SECS_PER_ADDR_SP%ENTS_IVEC_PER_SEC;
	/* Determine size of free blocks bit map */
	sb.sb_free_blocks_map_size_sec = ((sb.sb_part_size_total-
																	 (sb.sb_off+sb.sb_size))/ENTS_BITS_PER_SEC)+1;
	sb.sb_free_blocks_map_size_sec -= sb.sb_free_blocks_map_size_sec/
																		ENTS_BITS_PER_SEC;
	sb.sb_free_blocks_map_size_rem = sb.sb_part_size_total%ENTS_BITS_PER_SEC;
	/*
	 * Calculate usable partition size
	 */
	sb.sb_part_size = sb.sb_part_size_total - sb.sb_free_blocks_map_size_sec - 
										sb.sb_off - sb.sb_size;

	/*
	 * Determine amount of ivectors.
	 */
	for (count=0,i=sb.sb_part_start+sb.sb_ivec_off; i<sb.sb_part_start+sb.sb_part_size_total; i+=(SECS_PER_ADDR_SP+1), count++) {
		sb.sb_num_addr_sp++;
	}
	sb.sb_part_size -= sb.sb_ivec_size_sec*count;
	sb.sb_num_addr_sp += (sb.sb_free_blocks_map_size_sec - 
											 sb.sb_ivec_size_sec/count)/SECS_PER_ADDR_SP;
	sb.sb_free_blocks_map_size_sec -= (sb.sb_ivec_size_sec*count)/
																		ENTS_BITS_PER_SEC;
	/* Offset in sectors to the first ivector */
	sb.sb_ivec_off = sb.sb_off + sb.sb_size + sb.sb_free_blocks_map_size_sec;
	/* Offset in sectors to the end of the first ivector */
	sb.sb_ivec_end = (sb.sb_ivec_off + sb.sb_ivec_size_sec)+1;

	/*
	 * Write bit map.
	 */
	lseek(part_fd, (sb.sb_off+sb.sb_size)*SEC_SIZE, SEEK_SET);
	printf("Writing bit map of free blocks...\n");
	for (i=0; i<sb.sb_free_blocks_map_size_sec; i++) {
		if ( (result=write(part_fd, buf_int, SEC_SIZE)) == -1) {
			perror("write()");
			return 0;
		}
		verbose_percentage(i, sb.sb_free_blocks_map_size_sec);
	}
	/*
	 * Write ivector entries initialized to zero.
	 * Also subtract the ivectors from the partition size and bit map size.
	 */
	/* Vector to write to ivector */
	empty_ivec = calloc(sb.sb_ivec_size_sec*SEC_SIZE, sizeof(char));
	printf("Writing ivectors...\n");
	for (count=0; count<sb.sb_num_addr_sp; count++) {
		if ( (pwrite(part_fd, empty_ivec, sb.sb_ivec_size_sec*SEC_SIZE, 
				 (((sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec)*(SEC_SIZE))+
				 ((SECS_PER_ADDR_SP+1)*count)))) == -1) {
			perror("write()");
			return 0;
		}
		verbose_percentage(count, sb.sb_num_addr_sp);
	}
	/*
	 * Be verbose.
	 */
	if ( (file_desc = open("./SECTORS_IVEC", O_RDWR | O_CREAT)) == -1) {
		perror("open()");
		return 0;
	}
	printf("%d ivectors of size %d sectors (%dKB)\n", 
		sb.sb_num_addr_sp,
		sb.sb_ivec_size_sec,
		((sb.sb_ivec_size_sec*SEC_SIZE)/1024));
	for (i=0; i<sb.sb_num_addr_sp; i++) {
		dprintf(file_desc, "%d-%d\n", 
			(sb.sb_ivec_off+(i*SECS_PER_ADDR_SP)), 
			(sb.sb_ivec_off+sb.sb_ivec_size_sec)+(i*SECS_PER_ADDR_SP));
	}
	printf("Sectors used were written to SECTORS_IVEC in current dir\n");
	/*
	 * Point to superblock.
	 */
	lseek(part_fd, sb.sb_off*SEC_SIZE, SEEK_SET);
	/*
	 * Write superblock.
	 */
	if ( (result=write(part_fd, &sb, SEC_SIZE)) == -1) {
		perror("write()");
		return 0;
	}
	/*
	 * Be verbose.
	 */
	printf("The superblock of size %d bytes and bit map of size %d ",
		sizeof(sb), sb.sb_free_blocks_map_size_sec);
	printf("sectors (%dKB) was written to sectors %d-%d (%dKB-%dKB) ",
		(((sb.sb_free_blocks_map_size_sec)*SEC_SIZE)/1024),
		sb.sb_off,
		sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec,
		(((sb.sb_off)*SEC_SIZE)/1024),
		(((sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec)*SEC_SIZE)/1024));
	printf("from beginning of partition\n");

	/*
	 * Take first ivector and write address in sectors of root inode.
	 */
	/*
	 * Point to beginning of first ivector.
	 */
	lseek(part_fd, (sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec) * SEC_SIZE, SEEK_SET);
	pread(part_fd, buf_int, SEC_SIZE, (sb.sb_off+sb.sb_size+sb.sb_free_blocks_map_size_sec)*SEC_SIZE);
	buf_int[0] = sb.sb_ivec_size_sec;

	/*
	 * Write ivector entry for root directory.
	 */
	write(part_fd, buf_int, SEC_SIZE);

	/*
	 * Update bit map to reflect root's inode.
	 */
	pread(part_fd, block, SEC_SIZE, ((sb.sb_off+sb.sb_size)*SEC_SIZE));
	block[0] = 0x01; /* bit 1 and 2 */
	pwrite(part_fd, block, SEC_SIZE, ((sb.sb_off+sb.sb_size)*SEC_SIZE));

	/*
	 * Fill inode.
	 */
	i_root.i_type = DIRECTORY; /* root is a directory */
	i_root.i_dat_cont = TRUE; /* data is contigious */
	i_root.i_ivec = ROOT; /* ivector is entry ROOT */
	i_root.i_pivec = ROOT; /* parent ivector is entry ROOT */
	i_root.i_dsize = 1; /* initial size in sectors of root directory */
	count=0;
	i_root.i_filespec.i_dirspec.i_dirname[0] = '/';
	i_root.i_filespec.i_dirspec.i_dir_ents = INIT_ROOT_ENTS;

	/*
	 * Point to inode describing root directory.
	 */
	lseek(part_fd, (sb.sb_off+sb.sb_size+sb.sb_ivec_size_sec+sb.sb_free_blocks_map_size_sec) * SEC_SIZE, SEEK_SET);
	/*
	 * Write inode of root directory.
	 */
	write(part_fd, &i_root, SEC_SIZE);

	/*
	 * Point to beginning of data in root directory.
	 */
	pread(part_fd, buf_int, SEC_SIZE, (sb.sb_ivec_off * SEC_SIZE));
	lseek(part_fd, (sb.sb_ivec_off+(buf_int[ROOT]+1))*SEC_SIZE, SEEK_SET);

	/*
	 * Update bit map to reflect root's data.
	 */
	pread(part_fd, block, SEC_SIZE, ((sb.sb_off+sb.sb_size)*SEC_SIZE));
	block[1] = 0x01; /* bit 1 and 2 */
	pwrite(part_fd, block, SEC_SIZE, ((sb.sb_off+sb.sb_size)*SEC_SIZE));

	/*
	 * Write directory data.
	 */
	dir_entry_tmp = dir_entries;
	for (count=0; count<i_root.i_filespec.i_dirspec.i_dir_ents; count++) {
		strncpy(dir_entry_tmp, root_ents[count], 255);
		dir_entry_tmp = dir_entry_tmp + 8; /* point to second part of entry */
		*dir_entry_tmp = 0; /* copy address space of entry */
		dir_entry_tmp += 4; /* next entry */
		*dir_entry_tmp = i_root.i_ivec; /* copy current and parent ivector */
		dir_entry_tmp += 4; /* next entry */
	}
	write(part_fd, dir_entries, SEC_SIZE);

	/*
	 * Be verbose.
	 */
	printf("Root directory inode is at sector %d (%dKB) from partition start\n", sb.sb_off+sb.sb_size+sb.sb_ivec_size_sec+sb.sb_free_blocks_map_size_sec, (((sb.sb_off+sb.sb_size+sb.sb_ivec_size_sec+sb.sb_free_blocks_map_size_sec)*SEC_SIZE)/1024));

	return 0;
}

static
void
verbose_percentage(int less, int greater)
{
	if (less == greater/4 || less == greater/2 || less == ((greater/2)+greater/4)|| less == ((greater/2)+greater/2) || less == greater-1)
		printf("%.0f%%\n", ((float)less/(float)greater)*100);
}

static 
void
instructions()
{
	printf("Partition+disk+part_num\n");
}
#endif
