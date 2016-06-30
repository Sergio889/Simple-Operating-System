#include "ints_excps.h"

/*
 * Upgrade the interrupt/exception system to a more flexible, C version.
 */
void
init_ints_excps()
{
	struct idt_desc *ptr_desc;
	byte *idt_byte;
	word *idt_word;
	uint_t cpy_wrds;

}
