/*
 * Common values of the 32-bits ELF file format structure.
 */
#ifndef COMMON
#define COMMON
#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))
#define EI_NIDENT 4


/*
 * Relocation entries.
 */
struct elf_rel {
	unsigned int r_offset;
	unsigned int r_info;
};
struct elf_rela {
	unsigned int r_offset;
	unsigned int r_info;
	int r_addend;
};

/*
 * This is the format of the ELF header.
 */
struct elf_hdr {
	unsigned char e_ident[16];
	unsigned short e_type;
	unsigned short e_machine;
	unsigned int e_version;
	unsigned int e_entry;
	unsigned int e_phoff;
	unsigned int e_shoff;
	unsigned int e_flags;
	unsigned short e_ehsize;
	unsigned short e_phentsize;
	unsigned short e_phnum;
	unsigned short e_shentsize;
	unsigned short e_shnum;
	unsigned short e_shstrndx;
};

/*
 * Format of the section header.
 */
struct elf_shdr {
	int sh_name;
	unsigned int sh_type;
	unsigned int sh_flags;
	unsigned int sh_addr;
	unsigned int sh_offset;
	unsigned int sh_size;
	unsigned int sh_link;
	unsigned int sh_info;
	unsigned int sh_addralign;
	unsigned int sh_entsize;
};
/*
 * Dynamic section.
 */
struct elf_sdyn {
	int d_tag;
	union {
		unsigned int d_val;
		unsigned int d_ptr;
	} d_un;
};
/*
 * Dynamic array tags.
 */
#define DT_NULL 0 // element marks end of the _DYNAMIC array
#define DT_NEEDED 1 // element holds a string table offset to needed library
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4 
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_BIND_NOW 24
#define DT_LOWPROC 0x70000000
#define DT_HIPROC 0x7fffffff

/*
 * Format of the program header.
 */
struct elf_phdr {
	int p_type;
	unsigned int p_offset;
	unsigned int p_vaddr;
	unsigned int p_paddr;
	unsigned int p_filesz;
	unsigned int p_memsz;
	unsigned int p_flags;
	unsigned int p_align;
};
/*
 * Segment types.
 */
#define PT_NULL 0 // segment is unused
#define PT_LOAD 1 // segment is loadable
#define PT_DYNAMIC 2 // segment has dynamic linking information
#define PT_INTERP 3 // segment specifies location and size of interpreter
#define PT_NOTE 4 // segment specifies location and size of auxiliary information
#define PT_SHLIB 5 // reserved
#define PT_PHDR 6 // segment specifies the location and size of program header table
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff
/*
 * Segment flag bits.
 */
#define PF_X 0x1 // execute
#define PF_W 0x2 // write
#define PF_R 0x4 // read
#define PF_MASKPROC 0xf0000000 // write

/*
 * Format of the symbol table.
 */
struct elf_sym {
	int st_name;
	unsigned int st_value;
	unsigned int st_size;
	unsigned char st_info;
	unsigned char st_other;
	unsigned short st_shndx;
};
/*
 * Symbol binding definitions.
 */
#define STB_LOCAL 0 // symbol is not visible outside the object file containing their definitions.
#define STB_GLOBAL 1 // symbol is visible outside the object file containing their definitions.
#define STB_WEAK 2 // weak symbols resemble global symbols, but their definitions have lower precedence. (from ELF specification)
#define STB_LOPROC 13 // processor specific semantics
#define STB_HIPROC 15 // processor specific semantics

/*
 * Symbol types definitions.
 */
#define STT_NOTYPE 0 // symbol type is not defined
#define STT_OBJECT 1 // symbol is data
#define STT_FUNC 2 // symbol is text
#define STT_SECTION 3 // symbol refers to a section
#define STT_FILE 4 // symbol refers to a section
#define STT_LOPROC 13 // processor specific semantics
#define STT_HIPROC 15 // processor specific semantics
#endif
