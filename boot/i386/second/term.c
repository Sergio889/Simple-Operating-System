#include "term.h"
#include "heap.h"

extern key_asciico;
extern ptr_str_term;
struct tty_SOS tty_cpy;
static uint_t clear_bool=0;

static void
up_down(byte scanco)
{
	if (scanco == CURUP) {
		if (tty_cpy.tty_cur_pos>0)
			tty_cpy.tty_cur_pos--;
		else
			tty_cpy.tty_cur_pos=200;
	}
	else {
		if (tty_cpy.tty_cur_pos >= 200)
			tty_cpy.tty_cur_pos=0;
		else
			tty_cpy.tty_cur_pos++;

	}

	update_position(lin_cur_pos[tty_cpy.tty_cur_pos]);
}

uint_t
printable(unsigned char char_arg)
{
	if (char_arg > 31 && char_arg < 128)
		return TRUE;
	else
		return FALSE;
}

static void
clear_line()
{
	/* while logical characters */
	while (simple_tty.tty_num_chars>2) {
		simple_tty.tty_num_chars--;
		*simple_tty.tty_ptr_tail_que = 0; 
		simple_tty.tty_ptr_tail_que--;
		*simple_tty.tty_cur_addr = ' ';
	 	simple_tty.tty_cur_addr-=2;
		if ((simple_tty.tty_cursor)%80==0 && simple_tty.tty_cursor!=0) {
			simple_tty.tty_cur_line--;
			simple_tty.tty_cur_line_scr--;
		}
	 	simple_tty.tty_cursor--;
	}
	*simple_tty.tty_cur_addr = ' '; // clear first character
	simple_tty.tty_cur_addr-=4;
	simple_tty.tty_cursor-=2;
	simple_tty.tty_num_chars=0;

	prompt();
}

/*
 * Clear the screen until beginning of page.
 * New position and cursor is beginning of current page.
 */
static void
clear_screen()
{
	/* pointer to beginning of VRAM */
	byte *ptr_vram_clean=(byte *)simple_tty.tty_base_vram;
	uint_t count;
	
	/* clean page */
	for (count=0; count < 8*2000; count++)
		*ptr_vram_clean = ' ', ptr_vram_clean+=2;

	/* current address is now beginning of VRAM */
	simple_tty.tty_cur_addr = (byte *)(simple_tty.tty_base_vram);

	simple_tty.tty_cur_line = 0;
	simple_tty.tty_cur_line_scr = 0;
	simple_tty.tty_cur_pos = 0;
	simple_tty.tty_cursor = 0;
	/* restart logical and physical line characters count */
	simple_tty.tty_num_chars = 0;
	
	/* print prompt to terminal */
	prompt();

	/* point to beginning of buffer */
	simple_tty.tty_ptr_tail_que = simple_tty.tty_que_buf;

	/* 'restore' buffer to terminal (copy it) */
	simple_tty.tty_write(strnlen(simple_tty.tty_que_buf));
}

/*
 * Here is when control is being hold and the current character is printable.
 */
static void
inter_spec(char ascii_code)
{
	/* clear screen interpretation */
	if (ascii_code == 'l')
		simple_tty.tty_clear_screen();

	/* clear screen interpretation */
	if (ascii_code == 'u')
		simple_tty.tty_clear_line();
}


static void
queue_char(char ascii_code)
{
	*(simple_tty.tty_ptr_tail_que) = ascii_code; // queue character
}

static void
term_unwrite()
{
	if (simple_tty.tty_num_chars > 2) {
		simple_tty.tty_num_chars--; // advance logical line characters
		*simple_tty.tty_ptr_tail_que = 0;
		simple_tty.tty_ptr_tail_que--;
		simple_tty.tty_cursor--;
		simple_tty.tty_cur_addr -= 2;
		if ((simple_tty.tty_cursor)%80==0 && simple_tty.tty_cursor!=0) {
			simple_tty.tty_cur_line--;
			simple_tty.tty_cur_line_scr--;
		}
		*simple_tty.tty_cur_addr = ' ';
	}
	update_cursor(simple_tty.tty_cursor);
	//update_position();
}

/*
 * Write characters to Video RAM.
 */
static void
vram_write(ubyte_t count)
{
	uint_t i;

	/* while we have characters */
	while (count--) {
		simple_tty.tty_num_chars++; // advance char count
		/* if we have a non-printable ascii character */
		if (printable(*simple_tty.tty_ptr_tail_que) == FALSE) {
			switch (*simple_tty.tty_ptr_tail_que) {
				case ENTER:
					count=0;
					simple_tty.tty_ptr_tail_que=simple_tty.tty_que_buf;
					/* save string from queue buffer up to ENTER character */
					while ( (simple_tty.tty_com_buf[count] = *simple_tty.tty_ptr_tail_que)
								  != ENTER) 
					{
						count++;
						simple_tty.tty_ptr_tail_que++;
					}
					simple_tty.tty_com_buf[count] = '\n';
					simple_tty.tty_com_buf[count+1] = '\0';
				case '\n':
					if (*(simple_tty.tty_ptr_tail_que+1) == '\0') {
						simple_tty.tty_ptr_tail_que=simple_tty.tty_que_buf;
						while (simple_tty.tty_num_chars > 0) {
							*simple_tty.tty_ptr_tail_que = 0;
							simple_tty.tty_ptr_tail_que++;
							simple_tty.tty_num_chars--;
						}
						simple_tty.tty_ptr_tail_que=simple_tty.tty_que_buf;
					}
					else
						simple_tty.tty_ptr_tail_que++;
					
					simple_tty.tty_num_chars = 0;
					simple_tty.tty_cur_line++; // advance page lines
					simple_tty.tty_cur_line_scr++; // advance page lines
					simple_tty.tty_cur_addr= (byte *)(simple_tty.tty_base_vram +
													((lin_cur_pos[simple_tty.tty_cur_line])*2));
					simple_tty.tty_cursor=lin_cur_pos[simple_tty.tty_cur_line];
					break;
			}
		}
		/* else we have a printable character */
		else {
			/* print character */
			*simple_tty.tty_cur_addr = *simple_tty.tty_ptr_tail_que;
			simple_tty.tty_cur_addr+=2, simple_tty.tty_cursor++;
			simple_tty.tty_ptr_tail_que++;
		}

		/* if we pass a physical line boundary */
		if (((simple_tty.tty_num_chars%80)==0)&&(simple_tty.tty_num_chars!=0)) {
			simple_tty.tty_cur_line++; // advance physical line
			simple_tty.tty_cur_line_scr++; // advance physical line
		}
		
		/* if out of screen */
		if (simple_tty.tty_cur_line_scr == 25) {
			simple_tty.tty_cur_line_scr--;
			/* for sure we advance one line */
			simple_tty.tty_cur_pos++;
		}
		if (simple_tty.tty_cur_line==640)
			simple_tty.tty_clear_screen();
		/* update position on each loop */
		update_position(lin_cur_pos[simple_tty.tty_cur_pos]);
		/* update cursor on each loop */
		update_cursor(simple_tty.tty_cursor);
	}
	*simple_tty.tty_ptr_tail_que = '\0';
}
	
static void
vram_init()
{
	uint_t count;

	/* empty VRAM */
	for (count=0; count < 80*25*8; count++)		
		*simple_tty.tty_cur_addr = ' ', simple_tty.tty_cur_addr+=2;

	/* point to beginning of VRAM */
	simple_tty.tty_cur_addr = (byte *)simple_tty.tty_base_vram;
}

static void
map_ascii(byte scan_code, char * const ptr_ascii)
{
	byte *ptr_codes; // pointer to map vector

	/* if shift is being hold map from shift_map_vector vector */
	if ((simple_tty.tty_key_holds & SHIFT_HOLD) != 0x00)
		ptr_codes = shift_map_vector;
	/* else map from map_vector vector */
	else
		ptr_codes = map_vector;
	
	*ptr_ascii = ptr_codes[scan_code];
}


/*
 * This is the part of the terminal handler that controls the 6845's Video
 * RAM. It does some generic terminal processing.
 */
void
term_handl_scr()
{
	/*
	 * Pointers to original tty and copy tty.
	 * Duplicate original tty by copying it to copy
	 * tty.
	 */
	byte *ptr_tty_cpy, *ptr_simple_tty;
	/*
	 * Point to external string ptr_str_term.
	 */
	char *str_ptr=(char *)ptr_str_term;

	/* control variables */
	unsigned char count, count_cpy;
	uint_t count1;

	ubyte_t char_ascii; // hold ascii number
	ubyte_t byte_scanco; // hold scan code
	byte *ptr_tail_cpy = simple_tty.tty_ptr_tail_que; // backup copy

	/* 
	 * If the string pointer is NULL then we want to display (i.e process) a
	 * keypress.
	 */
	if (str_ptr == 0x00) {
		char_ascii = byte_scanco = *simple_tty.tty_ptr_tail_que;
		/* if we have a key release */
		if ((byte_scanco & 0x80) != 0) {
			switch (byte_scanco) {
				case CTRL_SCAN|0x80:
					simple_tty.tty_key_holds &= (~CTRL_MASK);
					break;
				case LSHIFT_SCAN|0x80:
					simple_tty.tty_key_holds &= (~SHIFT_MASK);
					break;
				default:
					break;
			}
		}
		/* 
		 * Else lets see if we have a special character,
		 * if there isn't any special characters being hold.
		 */
		else {
			if (printable(char_ascii) == FALSE) {
				switch (char_ascii) {
					case DELETE:
						if ((simple_tty.tty_key_holds&CTRL_MASK)&&
								(simple_tty.tty_key_holds&ALT_MASK))
							low_reboot();
						if ((simple_tty.tty_key_holds & UPDOWN) != 0) {
							simple_tty.tty_key_holds &= (~UPDOWN);
							update_position(lin_cur_pos[simple_tty.tty_cur_pos]);
						}
						else {
							simple_tty.tty_key_holds |= UPDOWN;
							ptr_tty_cpy = (byte *)&tty_cpy;
							ptr_simple_tty = (byte *)&simple_tty;
							for (count1=0; count1<sizeof(simple_tty); count1++)
								*ptr_tty_cpy++=*ptr_simple_tty++;
						}
						break;
					case CTRL:
						simple_tty.tty_key_holds |= CTRL_MASK;
						break;
					case LSHIFT:
						simple_tty.tty_key_holds |= SHIFT_MASK;
						break;
					case ALT:
						simple_tty.tty_key_holds |= ALT_MASK;
						break;
					case 0x9:
						simple_tty.tty_unwrite();
						break	;
					default:
						break;
				}
			}
	
			if ( (simple_tty.tty_key_holds & UPDOWN) != 0) {
				if ((char_ascii == CURUP) || (char_ascii == DOWN))
					simple_tty.tty_updown(char_ascii);
				else
					; // ignore character
			}
			/* 
			 * If the current character requires no special interpretation 
			 * and is printable.
			 * The possible cases for printing the current character are:
			 * 1. If the current character is printable and no special characters
			 *  are being hold.
			 * 2. If the current character is printable and the shift special
			 *  character is being hold.
			 */
			else {
				if (((((simple_tty.tty_key_holds & ANY_SPEC_MASK) == 0x00) ||
						((simple_tty.tty_key_holds & SHIFT_MASK) != 0x00)) && 
						((printable(char_ascii) == TRUE) || char_ascii==ENTER)))
				{
			
					simple_tty.tty_write(1);
				}
				/*
				 * Handle special treatment when control is being hold and current
				 * character is printable.
				 */
				else if (((simple_tty.tty_key_holds & CTRL_MASK) == TRUE) &&
									printable(char_ascii)) {
					simple_tty.tty_inter_spec(char_ascii);
				}
			}
		}
	}
	/* else we have a string to print */
	else {
		/* copy the string to the buffer only temporarily */
		for (count=count_cpy=strnlen(str_ptr, 255); count-- > 0; 
				*simple_tty.tty_ptr_tail_que++=*str_ptr++)
			;
		simple_tty.tty_ptr_tail_que = ptr_tail_cpy; // restore pointer
		simple_tty.tty_write(count_cpy);
		simple_tty.tty_ptr_tail_que = ptr_tail_cpy; // restore pointer

		/* invalidate string from buffer */
		while (count_cpy--)
			*simple_tty.tty_ptr_tail_que++ = '\0';

		simple_tty.tty_ptr_tail_que = ptr_tail_cpy; // restore pointer
	}
}

/*
 * Be nice with software and initialize structure
 */
static void
simple_tty_constructor()
{
	uint_t count;

	/*
	 * Initialize data.
	 */
	heap_alloc(20, &simple_tty.tty_que_buf);

	simple_tty.tty_num_chars=0;
	simple_tty.tty_cursor = 0; // initial cursor position
	/* initial number of lines */
	simple_tty.tty_cur_line=0;
	simple_tty.tty_cur_line_scr=0;
	/* XXX assume a color terminal */
	simple_tty.tty_type = COLOR;
	simple_tty.tty_key_holds=0;
	simple_tty.tty_ptr_tail_que = simple_tty.tty_que_buf;
	/* initial position in VRAM */
	simple_tty.tty_cur_pos = 0;
	if (simple_tty.tty_type == COLOR) {
		simple_tty.tty_base_vram = COLOR_VRAM;
		simple_tty.tty_cur_addr = (byte *)COLOR_VRAM;
	}
	else {
		simple_tty.tty_base_vram = MONO_VRAM;
		simple_tty.tty_cur_addr = (byte *)MONO_VRAM;
	}

	/*
	 * Initialize functions.
	 */
	simple_tty.tty_init = vram_init;
	simple_tty.tty_write = vram_write;
	simple_tty.tty_unwrite = term_unwrite;
	simple_tty.tty_inter_spec = inter_spec;
	simple_tty.tty_que_char = queue_char;
	simple_tty.tty_clear_screen = clear_screen;
	simple_tty.tty_clear_line = clear_line;
	simple_tty.tty_updown = up_down;
}

/*
 * Invoque the constructor to initialize the terminal object.
 * Initialize VRAM.
 */
void
term_init()
{
	/* implement constructor */
	simple_tty_constructor();

	/* initialize the video RAM */
	simple_tty.tty_init();
}

/*
 * After processing the keypress interrupt, we must process the input at
 * terminal level.
 * The function low_putstr does not imply that the character be printed,
 * it just call a low-level assembly code to generate an interrupt that will
 * return to the screen part of the terminal handling code.
 */
static void
putchar(ubyte_t code)
{
	/*
	 * This routine generates an interrupt requesting video services through
	 * vector 0x10. The interrupt handler returns immediately to term_handl_scr,
	 * which does the corresponding interpretation manipulating the VRAM.
	 * Finally, after the processing finishes, control returns here and goes
	 * back.
	 */
	*simple_tty.tty_ptr_tail_que = code;
	low_putstr(0);
}

/*
 * This is the terminal handler when receiving a keypress.
 */
uint_t
term_handl_kbd()
{
	ubyte_t scan_code; // variable to hold scan-code
	ubyte_t ascii_code; // map scan-code to ascii-code here
	
	ptr_str_term=0;
	/* 
	 * If we have a key release...
	 * getscanco returns the current scan-code.
	 */
	if ( !((scan_code=getscanco()) & 0x80)) {
		map_ascii(scan_code, &ascii_code);
	 	key_asciico = ascii_code;
		*simple_tty.tty_ptr_tail_que = ascii_code;
	}
	else
		*simple_tty.tty_ptr_tail_que = scan_code;

	term_handl_scr();

	return ascii_code;
}
