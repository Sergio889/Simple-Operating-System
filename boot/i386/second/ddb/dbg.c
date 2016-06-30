#include "dbg.h"
#define KERN_ADDR (uint_t)0x200000
#define PROC_ADDR (uint_t)0x400000
#define MEM_ADDR (uint_t)0x800000
#define FS_ADDR (uint_t)0xc00000
#define IO_ADDR (uint_t)0x1000000

/* definiciones externas */
extern int com_help(char *);
extern int com_inf(char *);
extern int com_stp(char *);
extern int com_brk(char *);
extern int com_dis(char *);
extern int com_con(char *);
extern int com_prn(char *);

/* definiciones locales */
static char *ptr_strs[6]; // vector de apuntadores a cadenas
static struct elf_hdr *ptr_hdr[6]; // vector de apuntadores a ptr_hdr
static struct elf_shdr *ptr_shdr[6]; // vector de apuntadores a ptr_shdr
static struct elf_sym *ptr_sym[6]; // vector de apuntadores a ptr_sym
static uaddr_t elf_addrs[6]={KERN_ADDR, PROC_ADDR, MEM_ADDR, FS_ADDR, IO_ADDR,
                             IO_ADDR}; // vector de direcciones en memoria
static uint_t welcome[6]={1, 1, 1, 1, 1, 1}; // bienvenida para cada archivo ELF
static char *coms[]={"help", "info", "step", "break", "print", "disasm", 
                     "continue", NULL}; // comandos disponibles
static struct sym_list ptr_lists[6][5]={1};
static int com_indx;
static int (*ptr_funcs[])(char *)={com_help, com_inf, com_stp, com_brk, com_prn,
                                   com_dis, com_con};
static char *ptr_stt[] = { "Variables", "Functions" };

static void
elf_get_smthg(byte *ptr_src, byte *ptr_des, uint_t size)
{
	uint_t i;
	int *ptr_tmp;

	/* copy ELF something */
	for (i=0; i<size; i++)
		*ptr_des++=*ptr_src++;

	ptr_tmp=(int *)ptr_des;

	*ptr_tmp=-1;
}

static void
dbg_prompt()
{
	kprintf("\nSOS's DDB\n");
	kprintf("======================================================\n");
}


/* definiciones globales */
int argc;
uaddr_t eip_glb;
uaddr_t *eflags_arg_glb;
uaddr_t *regs_glb;
uint_t elf_indx_glb;

int
dbg_symbols_printout(char *ptr_str, uint_t elf_indx, uint_t com_len)
{
	byte buffer[512];
	uint_t m, n, i, str_len;
	struct elf_sym *ptr_tmp1; 
	struct sym_list *ptr_tmp2;
	char cur_char=0;
	int ret_val;
	char *ptr_tmp3;
	char *ptr_com;

	for (n=0; n<2; n++) {
		for (ptr_tmp2=&(ptr_lists[elf_indx][n+1]),
				 ptr_tmp1=ptr_lists[elf_indx][n+1].ptr_cur; 
				 ptr_tmp2->ptr_cur!=NULL;
				 ptr_tmp2=ptr_tmp2->ptr_next, ptr_tmp1=ptr_tmp2->ptr_cur)
		{
			ptr_tmp3=ptr_strs[elf_indx];
			ptr_tmp3+=ptr_tmp1->st_name;
			ptr_com=ptr_str;
			if ( (str_len=strnlen(ptr_tmp3, 255))==com_len) {
				while (*ptr_com++==*ptr_tmp3++) {
					str_len--;
					if (str_len==0)
						goto out;
				}
			}
		}
	}
out:
	if (ptr_tmp2->ptr_cur!=NULL) {
		for (i=0; i<512; i++)
			buffer[i]=0;
		switch (n) {
			case 0:
				kprintf("Variable ");
				break;
			case 1:
				kprintf("Function ");
				break;
		}
		kprintf(ptr_str);
		kprintf(" at: %x\n", &(ptr_tmp1->st_value));
	}
	else
		ret_val=-1;

	return ret_val;
}

void
dbg_symbols_info(uint_t elf_indx)
{
	byte buffer[512];
	uint_t m, n, i;
	struct elf_sym *ptr_tmp1; 
	struct sym_list *ptr_tmp2;
	char *ptr_tmp3;
	char cur_char=0;

	for (n=0; n<2; n++) {
		kprintf("==========\n");
		kprintf(ptr_stt[n]);
		kprintf("\n");
		kprintf("==========\n");
		for (ptr_tmp2=&(ptr_lists[elf_indx][n+1]),
				 ptr_tmp1=ptr_lists[elf_indx][n+1].ptr_cur; 
				 ptr_tmp2->ptr_cur!=NULL;
				 ptr_tmp2=ptr_tmp2->ptr_next, ptr_tmp1=ptr_tmp2->ptr_cur)
		{
			ptr_tmp3=ptr_strs[elf_indx];
			ptr_tmp3+=ptr_tmp1->st_name;
			kprintf(ptr_tmp3);
			kprintf(" ");
		}
		if (n==0) {
			kprintf("[More]");
			while (cur_char!=ENTER && cur_char != 'q')
				cur_char=until_key();
			if (cur_char=='q') {
				kprintf("\n");
				break;
			}
		}
	}
}

int
dbg_symbol_lookup(char *ptr_str, uint_t elf_indx, uint_t com_len)
{
	byte buffer[512];
	uint_t m, i, str_len;
	struct elf_sym *ptr_tmp1; 
	struct sym_list *ptr_tmp2;
	char *ptr_tmp3;
	char *ptr_com;

	for (ptr_tmp2=&(ptr_lists[elf_indx][STT_FUNC]),
			 ptr_tmp1=ptr_lists[elf_indx][STT_FUNC].ptr_cur; 
			 ptr_tmp2->ptr_cur!=NULL;
			 ptr_tmp2=ptr_tmp2->ptr_next, ptr_tmp1=ptr_tmp2->ptr_cur)
	{
		ptr_tmp3=ptr_strs[elf_indx];
		ptr_tmp3+=ptr_tmp1->st_name;
		ptr_com=ptr_str;
		if ( (str_len=strnlen(ptr_tmp3, 255))==com_len) {
			while (*ptr_com++==*ptr_tmp3++) {
				str_len--;
				if (str_len==0)
					goto out;
			}
		}
	}
out:
	if (ptr_tmp2->ptr_cur==NULL)
		return -1;
	else
		return ptr_tmp2->ptr_cur->st_value;
}

void
dbg_symbol(uint_t eip_arg, uint_t elf_indx)
{
	byte buffer[512];
	uint_t m, i;
	struct elf_sym *ptr_tmp1; 
	struct sym_list *ptr_tmp2;
	char *ptr_tmp3;

	for (ptr_tmp2=&(ptr_lists[elf_indx][STT_FUNC]),
			 ptr_tmp1=ptr_lists[elf_indx][STT_FUNC].ptr_cur; 
			 (eip_arg<ptr_tmp1->st_value ||
				ptr_tmp1->st_value+ptr_tmp1->st_size<eip_arg) &&
			  ptr_tmp2->ptr_cur!=NULL;
			 ptr_tmp2=ptr_tmp2->ptr_next)
		ptr_tmp1=ptr_tmp2->ptr_cur;

	if (ptr_tmp2->ptr_cur==NULL)
		kprintf("%x", &eip_arg);
	else {
		ptr_tmp3=ptr_strs[elf_indx];
		ptr_tmp3+=ptr_tmp1->st_name;
		kprintf(" ");
		kprintf(ptr_tmp3);
	}
	kprintf("+");
	m=eip_arg-ptr_tmp1->st_value;
	kprintf("%x ", &m);
}

void
dbg_main(uaddr_t *eflags_arg, uint_t cs_arg, uint_t eip_arg, uaddr_t *regs_sav)
{
	byte cur_char;
	uint_t i, m, eip_tmp, elf_indx, shnum, shsize, ret, o;
	byte buffer[512];
	struct elf_shdr *ptr_tmp;
	struct elf_sym *ptr_tmp1; 
	struct sym_list *ptr_tmp2;
	char *ptr_com, *ptr_line;
	uint_t tmp;

	cur_char=0;
	eip_tmp=eip_arg;
	/* get index: kernel->0, mem->1, proc->2, fs->3, io->4 */
	elf_indx = eip_tmp/(4<<20);
	if (welcome[elf_indx])
		dbg_prompt(), welcome[elf_indx]=0;
	int_cli(); // for heap manipulation
	/* allocate space for the ELF header */
	heap_alloc((sizeof(struct elf_hdr)/ALLOC_SIZE)+1, &(ptr_hdr[elf_indx]));
	/* copy ELF header to allocated space */
	elf_get_smthg((byte *)(elf_addrs[elf_indx]),
								(byte *)(ptr_hdr[elf_indx]), sizeof(struct elf_hdr));
	/* allocate space for the section headers */
	shnum=ptr_hdr[elf_indx]->e_shnum;
	shsize=ptr_hdr[elf_indx]->e_shentsize;
	heap_alloc(((shnum*shsize)/ALLOC_SIZE)+1, &(ptr_shdr[elf_indx]));
	/* copy the section headers to allocated space */
	elf_get_smthg((byte *)((ptr_hdr[elf_indx]->e_shoff)+(elf_addrs[elf_indx])), 
					(byte *)ptr_shdr[elf_indx], shnum*shsize);
	/* traverse the section header list to find the symbol table */
	for (ptr_tmp=ptr_shdr[elf_indx]; ptr_tmp->sh_type!=2 
			 && ptr_tmp->sh_name!=-1; ptr_tmp++)
		;
	/* allocated space for the symbol headers */
	heap_alloc(((ptr_tmp->sh_size)/ALLOC_SIZE)+1, &(ptr_sym[elf_indx]));
	/* copy the symbol headers to allocated space */
	elf_get_smthg((byte *)(ptr_tmp->sh_offset+elf_addrs[elf_indx]), 
					(byte *)ptr_sym[elf_indx], ptr_tmp->sh_size);
	/* traverse the section header list to find the symbol-string table */
	for (ptr_tmp=ptr_shdr[elf_indx]; ptr_tmp->sh_type!=3
			 && ptr_tmp->sh_name!=-1; ptr_tmp++)
		;
	if (elf_indx!=0) {
		ptr_tmp++;
		while (ptr_tmp->sh_type!=3)
			ptr_tmp++;
	}
	ptr_strs[elf_indx]=(char *)(ptr_tmp->sh_offset+elf_addrs[elf_indx]);

	ptr_lists[elf_indx][STT_OBJECT].ptr_cur=NULL;
	ptr_lists[elf_indx][STT_FUNC].ptr_cur=NULL;
	ptr_lists[elf_indx][STT_SECTION].ptr_cur=NULL;
	ptr_lists[elf_indx][STT_FILE].ptr_cur=NULL;
	/* traverse the symbol table and link lists of symbol types */
	for (ptr_tmp1=ptr_sym[elf_indx]; ptr_tmp1->st_name!=-1; ptr_tmp1++) {
		/* continue if no type */
		if (((ptr_tmp1->st_info&0x0f)==STT_NOTYPE) || 
				((ptr_tmp1->st_info&0x0f)>STT_FILE) )
			continue;
		/* add entry to the appropiate list */
		else {
			for (ptr_tmp2=&(ptr_lists[elf_indx][(ptr_tmp1->st_info)&0x0f]);
					 ptr_tmp2->ptr_cur!=NULL; ptr_tmp2=ptr_tmp2->ptr_next)
				;
			ptr_tmp2->ptr_cur=ptr_tmp1;
			/* allocate space for the next element */
			heap_alloc((sizeof(struct sym_list)/ALLOC_SIZE)+2, 
								 &(ptr_tmp2->ptr_next));
			ptr_tmp2->ptr_next->ptr_cur=NULL;
		}
	}
	int_sti(); // enable heap again

	/* deshacer punto de interrupción */
	dbg_unbreak();

	/* imprimir dirección e instrucción de punto de interrupción */
	kprintf("Breakpoint at %x", &eip_arg);
	dbg_disasm(eip_arg, 1, regs_sav, elf_indx);
	kprintf("\n> ");

	/* deshabilitar interrupciones; asignar memoria; habilitar interrupciones */
	int_cli();
	heap_alloc((255/ALLOC_SIZE)+1, &ptr_line);
	int_sti();

	/* modo interactivo de comandos */
	while (1) {
		/* llenar variables globales */
		eflags_arg_glb=eflags_arg;
		eip_glb=eip_arg;
		regs_glb=regs_sav;
		elf_indx_glb=elf_indx;
		argc=0;
		com_indx=0;
		/* aceptar una línea */
		while ( (cur_char=until_key())!=ENTER)
			;
		/* si la línea es simplemente un caracter de fin de línea, salir */
		if (*(simple_tty.tty_com_buf)=='\n')
			break;

		/* si no, tomar comando e interpretarlo */
		else {
			argc=parse_line(simple_tty.tty_com_buf, ptr_line);
			com_indx=parse_com(ptr_line, coms);
			if (com_indx!=-1) {
				if ( (ret=ptr_funcs[com_indx](ptr_line)) == TRUE) {
					*eflags_arg = *eflags_arg & (~256);
					break;
				}
			}
			else
				kprintf("Invalid command\n");
			kprintf("\n");
			kprintf("> ");
		}
	}

	/* deshabilitar interrupciones; liberar memoria; habilitar interrupctiones */
	int_cli();
	for (m=1, ptr_tmp2=&(ptr_lists[elf_indx][m]); m<5; 
			 ptr_tmp2=&(ptr_lists[elf_indx][m]))
	{
		while (ptr_tmp2->ptr_cur!=NULL) {
			ptr_tmp2=ptr_tmp2->ptr_next;
			heap_free((void *)ptr_tmp2);
		}
		m++;
	}
	heap_free(ptr_shdr[elf_indx]);
	heap_free(ptr_sym[elf_indx]);
	heap_free(ptr_hdr[elf_indx]);
	heap_free(ptr_line);
	int_sti();
}
