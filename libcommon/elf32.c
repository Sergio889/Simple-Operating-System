/*
 * Receives an absolute path and a structure for the ELF header.
 * Fills the structure.
 */
#include "elf32.h"
#include "com.h"
#include "SOSfs.h"

void
elf_get_smthg(byte *ptr_src, byte *ptr_des, uint_t size)
{
	uint_t i;

	/* copy ELF something */
	for (i=0; i<size; i++)
		*ptr_des++=*ptr_src++;
}

void
elf_read_hdr(char const *filepath, struct elf_hdr *elf_hdr_struc)
{
	unsigned int file_ino;

	file_ino = sosfs_lookup(filepath);
}

uint_t
elf_file_size(char const *file_path)
{
	struct elf_hdr felf_hdr; // ELF header for file
	struct elf_shdr felf_shdr; // ELF section header for file
	struct elf_phdr felf_phdr; // ELF program header for file
	struct inode finode; // inode for file

	byte *felf_pptr=NULL; // pointer to program entries
	byte *felf_sptr=NULL; // pointer to section entries

	byte sector[512]; // sector buffer
	byte *ptr_sector=sector; // pointer to sector buffer

	uint_t ph_sec = 0; // indicates sector offset of file's program header

	/* get absolute sector for file's inode */
	uint_t ifile_abs_sec = sosfs_iget(file_path);
	/* data follows inode */
	uint_t file_abs_sec = ifile_abs_sec+1;

	/* fill a structure in core with inode's contents */
	sosfs_read_raw(ifile_abs_sec, &finode);

	/* return file size in sectors according to inode */
	return finode.i_dsize;
}
