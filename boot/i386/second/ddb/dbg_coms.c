#include "dbg.h"
#define NUM_REGS 10
extern int argc;
extern uaddr_t eip_glb;
extern uaddr_t *eflags_arg_glb;
extern uaddr_t *regs_glb;
extern uint_t elf_indx_glb;

static char *registers[] = { "eax", "ebx", "ecx", "edx", "esi", "edi", "esp", 
														 "ebp", "eip", "eflags" };
static char *info_args[] = { "syms", "regs", NULL };
static int func_indx;

static void
inf_syms()
{
	dbg_symbols_info(elf_indx_glb);
}

static void
inf_regs()
{
	uint_t i, m;
	byte buffer[512];

	for (i=0; i<NUM_REGS; i++) {
		low_putstr(registers[i]);
		low_putstr(": ");
		for (m=0; m<512; m++)
			buffer[m]=0;
		memdump32((int *)&(regs_glb[i]), buffer, 1, 4, 1);
		low_putstr(buffer);
		low_putstr("\n");
	}
}

static void (*funcs_info[])() = { inf_syms, inf_regs };

static void
to_num(char *ptr_str, uaddr_t *ptr_num, uint_t str_len)
{
	uint_t i, m;
	byte num[4];
	byte buffer[512];
	word *ptr_res;

	for (i=0; i<4; i++)
		num[i]=0;

	while (*ptr_str!='\0')
		ptr_str++;
	ptr_str--;

	i=0;
	m=0;
	while (m<str_len) {
		if (*ptr_str<='9')
			num[i] = ((*ptr_str)-0x30);
		else {
			switch (*ptr_str) {
				case 'a':
					num[i] = 10;
					break;
				case 'b':
					num[i] = 11;
					break;
				case 'c':
					num[i] = 12;
					break;
				case 'd':
					num[i] = 13;
					break;
				case 'e':
					num[i] = 14;
					break;
				case 'f':
					num[i] = 15;
					break;
			}
		}
		ptr_str--;
		if (*ptr_str<='9')
			num[i] |= (((*ptr_str)-0x30)<<4);
		else {
			switch (*ptr_str) {
				case 'a':
					num[i] |= 10<<4;
					break;
				case 'b':
					num[i] |= 11<<4;
					break;
				case 'c':
					num[i] |= 12<<4;
					break;
				case 'd':
					num[i] |= 13<<4;
					break;
				case 'e':
					num[i] |= 14<<4;
					break;
				case 'f':
					num[i] |= 15<<4;
					break;
			}
		}
		i++;
		m+=2;
		ptr_str--;
	}

	ptr_res=(word *)num;
	*ptr_num=*ptr_res;

}

static int
is_digit(char *ptr_char)
{
}

int
com_help(char *ptr_line)
{

	return 0;
}

int
com_stp(char *ptr_line)
{
	*eflags_arg_glb |= 256;

	return 0;
}

static int
com_is_hex(char *ptr_str, uint_t str_len)
{
	int ret;

	ret=1;
	if (str_len>10 || str_len<3)
		ret=0;
	else {
		if (*ptr_str!='0' || *(ptr_str+1)!='x')
			ret=0;
		ptr_str+=2;
		str_len-=2;
		while (str_len--) {
			switch (*ptr_str++) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					break;
				default:
					ret=0;
					return 0;
			}
			if (ret==0)
				break;
		}
	}

	return ret;
}

static int
com_get_arg(char *com_arg, char **com_args)
{
	char **ptr_str; 
	char *ptr_com, *ptr_tmp;
	uint_t com_len, str_len;
	byte buffer[512];
	uint_t i;

	ptr_com=com_arg;
	com_len=strnlen(ptr_com, 255);
	for (ptr_str=com_args; *ptr_str!=NULL; ptr_str++) {
		if ( (str_len=strnlen(*ptr_str, 255)) == com_len) {
			ptr_tmp=*ptr_str;
			ptr_com=com_arg;
			while (*ptr_com++==*ptr_tmp++) {
				str_len--;
				if (str_len==0)
					goto out;
			}
		}
		func_indx++;
	}
	func_indx=-1;

out:
	return func_indx;
}

/* advance characters until next command */
char *
com_next_arg(char *ptr_line)
{
	while (*ptr_line!='\0')
		ptr_line++;
	ptr_line++;

	return ptr_line;
}

int
com_prn(char *ptr_line)
{
	uaddr_t addr_disasm=0;
	char *ptr_byte, *ptr_tmp;
	uint_t str_len, str_cpy, i;
	byte buffer[512];

	if (argc==1)
		low_putstr("Missing argument [{address}, {symbol}]");

	else if (argc==2) {
		ptr_byte=ptr_tmp=com_next_arg(ptr_line);
		str_len=strnlen(ptr_tmp, 255);
		str_cpy=str_len-1;
		/* second argument must be in hex */
		if (!(com_is_hex(ptr_tmp, str_len))) {
			addr_disasm=dbg_symbols_printout(ptr_byte, elf_indx_glb, str_len);
			if (addr_disasm==-1) {
				low_putstr("Symbol ");
				low_putstr(ptr_byte);
				low_putstr(" not found");
			}
		}
		else {
			ptr_tmp+=2;
			to_num(ptr_byte+2, &addr_disasm, str_len);
			low_putstr("*");
			low_putstr("(");
			for (i=0; i<512; i++)
				buffer[i]=0;
			memdump32((int *)&addr_disasm, buffer, 1, 4, 1);
			low_putstr(buffer);
			low_putstr(") -> ");
			for (i=0; i<512; i++)
				buffer[i]=0;
			memdump32((int *)addr_disasm, buffer, 1, 4, 1);
			low_putstr(buffer);
		}
	}
}

int
com_inf(char *ptr_line)
{
	char *ptr_byte, *ptr_tmp;
	uint_t com_len, str_len;
	int info_indx;

	if (argc==1)
		low_putstr("Missing argument: info [syms || regs]");
	else if (argc==2) {
		func_indx=0;
		com_len=strnlen(ptr_line, 255);
		ptr_byte=ptr_tmp=com_next_arg(ptr_line);
		info_indx=com_get_arg(ptr_byte, info_args);
		if (info_indx==-1) {
			low_putstr("Invalid argument");
			low_putstr("\"");
			low_putstr(ptr_byte);
			low_putstr("\"");
		}
		else
			funcs_info[info_indx]();
	}

	return 0;
}

int 
com_brk(char *ptr_line)
{
	uaddr_t addr_disasm=0;
	char *ptr_byte, *ptr_tmp;
	uint_t str_len, str_cpy, i;
	byte buffer[512];

	if (argc==1)
		low_putstr("Missing argument: break [{address} || {symbol}]");

	else if (argc==2) {
		ptr_byte=ptr_tmp=com_next_arg(ptr_line);
		str_len=strnlen(ptr_tmp, 255);
		str_cpy=str_len-1;
		/* second argument must be in hex */
		if (!(com_is_hex(ptr_tmp, str_len))) {
			addr_disasm=dbg_symbol_lookup(ptr_byte, elf_indx_glb, str_len);
		}
		else {
			ptr_tmp+=2;
			to_num(ptr_byte+2, &addr_disasm, str_len);
		}
		if (addr_disasm==-1) {
			low_putstr("Symbol ");
			low_putstr("\"");
			low_putstr(ptr_byte);
			low_putstr("\"");
			low_putstr(" not found");
		}
		else {
			for (i=0; i<512; i++)
				buffer[i]=0;
			memdump32((int *)&addr_disasm, buffer, 1, 4, 1);
			low_putstr(buffer);
			dbg_break(addr_disasm);
		}
	}
	return 0;
}

int
com_dis(char *ptr_line)
{
	uaddr_t addr_disasm=0;
	char *ptr_byte, *ptr_tmp, cur_char;
	uint_t str_len, str_cpy;
	uint_t i;
	byte buffer[512];

	if (argc==1) {
		addr_disasm=eip_glb;
		do {
			addr_disasm=dbg_disasm(addr_disasm, 10, regs_glb, elf_indx_glb);
			cur_char=until_key();
		}	while (cur_char != 'q');
	}

	else if (argc==2) {
		ptr_byte=ptr_tmp=com_next_arg(ptr_line);
		str_len=strnlen(ptr_tmp, 255);
		str_cpy=str_len-1;
		/* second argument must be in hex */
		if (!(com_is_hex(ptr_tmp, str_len))) {
			if ((addr_disasm=dbg_symbol_lookup(ptr_byte, elf_indx_glb, str_len))==-1){
				low_putstr("Symbol ");
				low_putstr("\"");
				low_putstr(ptr_byte);
				low_putstr("\"");
				low_putstr(" not found or not a function");
			}
			else
				dbg_disasm(addr_disasm, 10, regs_glb, elf_indx_glb);
		}
		else {
			ptr_tmp+=2;
			to_num(ptr_byte+2, &addr_disasm, str_len);
			dbg_disasm(addr_disasm, 5, regs_glb, elf_indx_glb);
		}
	}
	else
		low_putstr("Bad arguments");
out:

	return 0;
}

int 
com_con(char *ptr_line)
{

	return 1;
}
