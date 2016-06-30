/*
 * Small and simple command-line interpreter.
 */
#include "term.h"
#include "heap.h"
#define NUM_COMMANDS 6
#define SET_NUM_PATHS 4
#define SET_NUM_VARS 1
#define NUM_DRIS 2
#define VM_ADDR 0x1400000
#define PT_FIRST 0x1401000

extern char *com_next_arg(char *);
static int argc;
static int indx;
char *addr_tmp = (char *)0xb8000;
struct set_args kern_args;
static char *servs_strs[] = { "Process server:  ", "Memory server: ", 
															"File-system server: ", "Device-drivers server: "
														};
static char *vars_strs[] = { "Debug: "};

static byte start_SOS;
static char cur_char; // current character
static char *commands[] = { "set", "help", "start",
			 											NULL };

static int
com_help(char *ptr_line, int help_argc)
{
	return 0;
}

static int
com_start(char *ptr_line, int help_argc)
{
	return 1;
}

static int
com_set(char *ptr_line, int set_argc)
{
	int num_args=0, num_dris=0;
	void *ptr_strs;
	char **ptr_servs=servs_strs;
	char *ptr_tmp;
	uint_t i;
	byte buffer[512];

	if (set_argc > 2)
		low_putstr("Syntax: set [var=val]\n");

	else {
		if (set_argc == 1) {
			for (ptr_strs=(void *)&kern_args; num_args < SET_NUM_PATHS; num_args++) {
				low_putstr(ptr_servs[num_args]); 
				low_putstr(((int *)ptr_strs)[num_args]);
				low_putstr("\n");
			}
			num_args=0;
			for (ptr_tmp=&(kern_args.debug); num_args<SET_NUM_VARS; num_args++) {
				low_putstr(vars_strs[num_args]);
				for (i=0; i<512; i++)
					buffer[i] = 0;
				memdump32((int *)ptr_tmp, buffer, 1, 1, 1);
				low_putstr(buffer);
				low_putstr("\n");
				ptr_tmp++;
			}
		}
		else if (set_argc==2) {
			/* 
			 * As simple as it can be: any variable name is valid. Because of this,
			 * preincrement ptr_tmp for the possible case that the variable name is
			 * the string "=". This way, it is possible to have up to 2 '='
			 * characters the first one representing the variable name.
			 */
			for (i=0, ptr_tmp=com_next_arg(ptr_line); *ptr_tmp!='='&&*ptr_tmp!=0; 
					 ptr_tmp++, i++)
				;
			/* invalid combinations */
			if (*ptr_tmp==0 || (*ptr_tmp=='=' && (*(ptr_tmp+1)=='\0')  || 
					(*ptr_tmp=='=' && *(ptr_tmp+2)!='\0') || i==0))
			{
				low_putstr("Invalid combination ");
				low_putstr("\"");
				low_putstr(com_next_arg(ptr_line));
				low_putstr("\"");
				low_putstr("\n");
			}
			else {
				for (i=0, ptr_tmp=com_next_arg(ptr_line); *ptr_tmp!='='; i++, ptr_tmp++)
					buffer[i] = *ptr_tmp;
				buffer[i] = '\0';
				if ((strncmp(buffer, "debug", 0)) == 0) {
					kern_args.debug=(byte)((*(ptr_tmp+1))-0x30);
					low_putstr(vars_strs[0]);
					for (i=0; i<512; i++)
						buffer[i] = 0;
					memdump32((int *)&(kern_args.debug), buffer, 1, 1, 1);
					low_putstr(buffer);
					low_putstr("\n");
				}
				else {
					low_putstr("Unknown variable ");
					low_putstr("\"");
					low_putstr(buffer);
					low_putstr("\"");
					low_putstr("\n");
				}
			}
		}
		else
			low_putstr("Invalid combination\n");
	}
}
static int (*ptr_funcs[])(char *, int) = { com_set, com_help, com_start };

void
page_serv_high(int laddr_arg, int err_cod, int vaddr_arg, int cs_arg)
{
	byte temp_buf[512];
	uint_t i;
	int pgtab_off;
	/* address variables */
 	uaddr_t vm_strucs; // holds address for paging structures
	uaddr_t cur_pgframe; // holds base address of first page-frame
	uaddr_t *ptr_pgdir=(uint_t *)VM_ADDR; // pointer to current page-dir
	uaddr_t *ptr_pgtab; // pointer to current page-table

	for (i=0; i<512; i++)
		temp_buf[i]=0;
	low_putstr("Handling page-fault exception...\n");
	low_putstr("Error code: ");
	memdump32(&err_cod, temp_buf, 1, 4, 1);
	low_putstr(temp_buf);
	low_putstr("\n");
	for (i=0; i<512; i++)
		temp_buf[i]=0;
	low_putstr("Linear address: ");
	memdump32(&laddr_arg, temp_buf, 1, 4, 1);
	low_putstr(temp_buf);
	low_putstr("\n");
	for (i=0; i<512; i++)
		temp_buf[i]=0;
	low_putstr("Virtual address: ");
	memdump32(&vaddr_arg, temp_buf, 1, 4, 1);
	low_putstr(temp_buf);
	low_putstr("\n");
	for (i=0; i<512; i++)
		temp_buf[i]=0;
	low_putstr("Segment: ");
	memdump32(&cs_arg, temp_buf, 1, 4, 1);
	low_putstr(temp_buf);
	low_putstr("\n");
	while (1)
		;

	/* get offset to correct page directory entry */
	pgtab_off = laddr_arg>>20; // each page table maps 4MB
	/* 
	 * Initialize vm_strucs to hold the address for the beginning of the 
	 * page-dir.
	 * Initialize ptr_pgtab to point to the address for the first page-dir 
	 * entry (i.e page-table).
	 * Increment both variables accordingly.
	 */
	for (vm_strucs=VM_ADDR,ptr_pgtab=(uaddr_t *)PT_FIRST;pgtab_off >= 4;)
		vm_strucs+=4, pgtab_off-=4, ptr_pgtab+=4096/4;
	ptr_pgdir = (uint_t *)vm_strucs; // point to page-directory entry
	*ptr_pgdir = ((uint_t)ptr_pgtab)|0xb; // write entry to page-directory
	/*
	 * Map virtual-memory to physical-memory.
	 */
	pgtab_off = laddr_arg&0xc;
	pgtab_off/=4096;
	ptr_pgtab+=pgtab_off;
	*ptr_pgtab = laddr_arg|0xb;
}


/*
 * Constructor for the initial state of arg structure.
 */
static void
set_args_constr()
{
	/* default locations for user-level servers */
	kern_args.kern_path = "/boot/kernel";
	kern_args.proc_serv_path = "/boot/pr_serv";
	kern_args.vm_serv_path = "/boot/vm_serv";
	kern_args.fs_serv_path = "/boot/fs_serv";
	kern_args.dev_serv_path = "/boot/dv_serv";
	kern_args.debug = 0;
	kern_args.num_servs = 4;
}

/*
 * Print the most original prompt ever.
 */
void 
prompt()
{
	low_putstr("> ");
}

/*
 * This is the main command-interpreter driver.
 */
void
interpreter_init()
{

	uint_t i, ret;
	char *ptr_com, *ptr_line;
	
	set_args_constr(); // call the structure's constructor
	int_sti();
	low_putstr("This is the command-line interpreter.\n\n");

	int_cli();
	heap_alloc((255/ALLOC_SIZE)+1, &ptr_line);
	int_sti();
	while (1) {
		prompt();
		argc=0;
		indx=0;
		cur_char=0;
		/* print a nice and quite prompt */
		while ( (cur_char=until_key())!=ENTER)
			;
		if (*(simple_tty.tty_com_buf)=='\n')
			continue;

		/* parse new line */
		argc=parse_line(simple_tty.tty_com_buf, ptr_line);
		indx=parse_com(ptr_line, commands);
		if (indx!=-1) {
			if ( (ret=ptr_funcs[indx](ptr_line, argc)) == TRUE)
				break;
		}
		else {
			low_putstr("Invalid command ");
			low_putstr("\"");
			low_putstr(ptr_line);
			low_putstr("\"");
			low_putstr("\n");
		}
	}

	int_cli();
	heap_free(ptr_line);
	int_sti();
}
