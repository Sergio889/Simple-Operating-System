#include "kstdarg.h"
#include "../com.h"
#include "../dump32.h"
#include "../bios.h"

void
kprintf(char *kprn_fmt, ...)
{
	va_list arg_lst;
	uint_t num_args, i, m, num_bytes, num_words;
	byte *ptr_fmt;
	byte *ptr_buf;
	word *ptr_baddr, *word_hld;

	/* start list */
	va_start(arg_lst, &kprn_fmt);

	/* calculate number of arguments */
	num_args=0;
	for (ptr_fmt=kprn_fmt; *ptr_fmt!='\0'; ptr_fmt++) {
		if (*ptr_fmt=='%')
			num_args++;
	}
	ptr_fmt=kprn_fmt;
	ptr_baddr=(word *)buffer; // point to buffer's address
	ptr_buf=(byte *)ptr_baddr; // point to buffer
	/* clean up buffer */
	for (m=0; m<64; m++)
		ptr_buf[m]='\0';

	if (num_args>0) {
		for (ptr_fmt=kprn_fmt; *ptr_fmt!='\0'; ptr_fmt++) {
			if (*ptr_fmt!='%')
				*ptr_buf++=*ptr_fmt;
			else {
				ptr_fmt++; // begin of format
				if (((*ptr_fmt)-0x30) < 9) {
					num_bytes=(*ptr_fmt)-0x30;
					ptr_fmt++; // begin format
				}
				else
					num_bytes=4;
				if (((*ptr_fmt)-0x30) < 9) {
					num_words=(*ptr_fmt)-0x30;
					ptr_fmt++; // begin format
				}
				else
					num_words=1;
				word_hld=(word *)arg_lst;
				switch (*ptr_fmt) {
					case 'x':
						memdump32(*word_hld, ptr_buf, num_words, num_bytes, HEX);
						break;
					case 'd':
						memdump32(*word_hld, ptr_buf, num_words, num_bytes, DEC);
						break;
					default:
						;
				}
				/* advance pointer to end of string */
				while (*ptr_buf!='\0')
					ptr_buf++;
			}
		}
		*ptr_buf='\0';
	}
	else {
		while (*ptr_fmt!='\0')
			*ptr_buf++=*ptr_fmt++;
		*ptr_buf='\0';
	}

	ptr_buf=(byte *)ptr_baddr; // point to buffer
	low_putstr(ptr_buf); // print string
}
