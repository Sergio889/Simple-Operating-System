#include "ddb/dbg.h"

int
parse_line(char *line_arg, char *ptr_line)
{
	uint_t line_len;
	int argc=0;

	/* allocate space for copying the line */
	line_len=strnlen(line_arg, 255);

	/* traverse the line */
	while (*line_arg!='\0')
		/* copy the line replacing spaces for end of string characters */
		if (*line_arg!=' ' && *line_arg!='\n')
			*ptr_line++=*line_arg++;
		else if (*ptr_line=='\n')
			line_arg++;
		else
			argc++, *ptr_line++='\0', line_arg++;

	return argc;
}

int
parse_com(char *com_arg, char **coms)
{
	char **ptr_str; 
	char *ptr_com, *ptr_tmp;
	uint_t com_len, str_len;
	byte buffer[512];
	uint_t i;
	int com_indx=0;

	ptr_com=com_arg;
	com_len=strnlen(ptr_com, 255);
	for (ptr_str=coms; *ptr_str!=NULL; ptr_str++) {
		if ( (str_len=strnlen(*ptr_str, 255)) == com_len) {
			ptr_tmp=*ptr_str;
			ptr_com=com_arg;
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
