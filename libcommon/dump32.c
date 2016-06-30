/*
 * Dump order: %eax, %ebx, %ecx, %edx, %ebp, %esp, %cs, %ds, %es, %ss, %gs,
 * %fs, %cr0, %cr3, %cr4.
 */
#include "dump32.h"
#include "com.h"
/*
 * Writes the corresponding ascii of num_bytes bytes to the address pointed
 * by hex_num.
 */
void
hex2ascii32(uint_t *hex_num, uint_t num_bytes, char *buf)
{
	ubyte_t cur_byte; // hold the byte being handled
	word num_nibbles = num_bytes * 2; // we work by nibbles
	byte *hex_num_local_ptr = (byte *)hex_num; // work with byte pointer
	char hexs[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
							 'c', 'd', 'e', 'f'};

	while (num_nibbles > 0) {
		/* copy current byte */
		cur_byte = *hex_num_local_ptr; 
		/* if working with low nibble */
		if ((num_nibbles%2) == 0)
			cur_byte &= 0x0f; // extract low nibble
		/* else if working with high nibble */
		else
			cur_byte &= 0xf0, cur_byte >>= 4; // extract high nibble
		cur_byte=hexs[cur_byte]; // get character
		if ((num_nibbles % 2) == 1) {
			buf[num_nibbles-1] = cur_byte;
			hex_num_local_ptr++;
		}
		else
			buf[num_nibbles-1] = cur_byte;
		num_nibbles--;
	}
}

/*
 * Writes the corresponding ascii of num_bytes bytes to the address pointed
 * by dec_num.
 */
void
dec2ascii32(uint_t *dec_num, uint_t num_bytes, char *buf)
{
}

void
hexdump32(unsigned int cr0_arg, unsigned int cr3_arg, unsigned int cr4_arg,
						unsigned int eax_arg, unsigned int ebx_arg, unsigned int ecx_arg, 
						unsigned int edx_arg, unsigned int ebp_arg, unsigned int esp_arg, 
						unsigned int esi_arg, unsigned int edi_arg, unsigned int cs_arg, 
						unsigned int ds_arg, unsigned int es_arg, unsigned int ss_arg, 
						unsigned int gs_arg, unsigned int fs_arg, char *buf_arg)
{
	char reg_num_dump;
	char *buf_arg_back = buf_arg; // save pointer to beginning of string
	unsigned int *args_pointer = &cr0_arg;

	for (reg_num_dump = 0; reg_num_dump != REG_NUM_DUMP; reg_num_dump++) {
		strncpy(dump_format[reg_num_dump], buf_arg, 9);
		while (*buf_arg != '\0')
			buf_arg++;
		hex2ascii32(args_pointer, 4, buf_arg);
		while (*buf_arg != '\0')
			buf_arg++;
		*buf_arg++ = ' ';
		args_pointer++;
	}
}

/*
 * Dump order: %cs, %eip, error code.
 */
void
excpdump32(unsigned int cs_arg, unsigned int eip_arg, unsigned int error_arg,
						unsigned int vec_arg, char *buf_arg)
{
	char reg_num_dump;
	char *buf_arg_back = buf_arg; // save pointer to beginning of string
	unsigned int *args_pointer = &cs_arg;

	strncpy(excp_vectors[vec_arg], buf_arg, 255);
	while (*buf_arg != '\0')
		buf_arg++;
	*buf_arg++ = ':';
	*buf_arg++ = ' ';

	for (reg_num_dump = 0; reg_num_dump != EXCP_NUM_DUMP; reg_num_dump++) {
		strncpy(excp_dump_format[reg_num_dump], buf_arg, 9);
		while (*buf_arg != '\0')
			buf_arg++;
		hex2ascii32(args_pointer, 4, buf_arg);
		while (*buf_arg != '\0')
			buf_arg++;
		*buf_arg++ = ' ';
		args_pointer++;
	}
}

/*
 * Dumps a contigious block of memory.
 */
void
memdump32(uint_t *mem_arg, char *buf_arg, ubyte_t num_words, uint_t num_bytes, 
					uint_t num_base)
{
	byte reg_num_dump;
	char *buf_arg_back = buf_arg; // save pointer to beginning of string
	uint_t *args_pointer = mem_arg;

	for (reg_num_dump = 0; reg_num_dump<num_words; reg_num_dump++) {
		switch (num_base) {
			case HEX:
				*buf_arg++ = '0';
				*buf_arg++ = 'x';
				hex2ascii32(args_pointer, num_bytes, buf_arg);
				break;
			case DEC:
				dec2bcd(args_pointer, num_bytes, buf_arg);
				break;
		}
		while (*buf_arg != '\0')
			buf_arg++;
		*buf_arg++ = ' ';
		args_pointer++;
	}
	buf_arg--;
	*buf_arg='\0';
}

