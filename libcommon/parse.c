#include "term.h"

void
parse_line(struct tty_SOS *tty)
{
	byte *tty_buf; 
	tty_buf = tty->tty_argv[0] = tty->tty_com_buf;
	byte i;

	/* traverse the line */
	for (i=0; *tty_buf++!='\0'; ++i) {
		/* replace spaces for end of string characters */
		if (*tty_buf==' ')
			tty->tty_argc++, *tty_buf='\0', tty->tty_argv[i] = tty_buf+1;
	}
}

int
parse_com(struct tty_SOS *tty, char **coms)
{
	char **ptr_str; 
	char *ptr_com, *ptr_tmp;
	uint_t com_len, str_len;
	byte buffer[512];
	uint_t i;
	int com_indx=0;

	ptr_com = tty->tty_argv[0];
	com_len=strnlen(ptr_com, 255);
	for (ptr_str=coms; *ptr_str!=NULL; ptr_str++) {
		if ( (str_len=strnlen(*ptr_str, 255)) == com_len) {
			ptr_tmp=*ptr_str;
			ptr_com=tty->tty_argv[0];
			while (*ptr_com++==*ptr_tmp++) {
				str_len--;
				if (str_len==0)
					goto out;
			}
		}
		com_indx++;
	}
	com_indx=-1;

out:
	return com_indx;
}
