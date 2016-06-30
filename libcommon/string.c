/*
 * Common routines for string manipulation.
 */
#include "string.h"
#include "com.h"

void
strncpy(const char *src_arg, char *dst_arg, int num_chars)
{
	char count;

	for (count = 0; *src_arg != '\0'; count++) {
		if (count > num_chars)
			break;
		*dst_arg++ = *src_arg++;
	}
}

ubyte_t
strnlen(const char *src_str, unsigned int num)
{
	uint_t count;

	for (count=0; (num > 0) && (*src_str != '\0') ; src_str++, num--, count++)
		;
	return count;
}

ubyte_t
strncmp(const char *src_str, const char *dst_str, unsigned int num)
{
	ubyte_t src_len;
	ubyte_t dst_len;

	if ( (src_len = (strnlen(src_str, MAX_SIZE_STR))) == (dst_len = (strnlen(dst_str, MAX_SIZE_STR)))) {
		src_len--;
		while (src_len > 0) {
			if (*src_str != *dst_str)
				break;
			src_str++;
			dst_str++;
			src_len--;
		}
	}

	else {
		src_len = 1;
	}

	return src_len;
}
