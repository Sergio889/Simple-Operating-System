/*
 * This is a small terminal-handling program for the primitive command
 * interpreter that runs in real mode.
 */

/*
 * Definitions of some mappings of scan codes.
 */
.set ENTER_SCAN, 43

.code32
.globl term_handler, read_key, until_key, nice_char, going_32, clean_page, putchar_low, stack_hack, iret_low, have_data, test_copy, low_reboot
.text
term_handler:
	ret
read_key:
	xorb %al, %al
1:
	testb %al, %al
	jz 1b
	ret

iret_low:
	addl $4, %esp
	iret

/*
 * This function reads characters up to a new line sequence.
 */
until_key:
	xorl %eax, %eax
1:
	testl %eax, %eax
	jz 1b
	ret

test_copy:
	movl $0xb8000, %esi
	movl $256, %ecx
1:
	movb $'a', (%esi)
	addl $2, %esi
	loop 1b
	jmp .

low_reboot:
	int $23

/*
 * This interrupt will ultimately lead the code to the C routine
 * term_handl_scr.
 */
putchar_low:
	int $0x10
	ret

.data
save_char:
	.long 0
