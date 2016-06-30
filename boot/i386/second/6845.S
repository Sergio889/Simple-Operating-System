/*
 * These are the addresses where IBM PC/AT and compatibles computers map
 * the internal registers of the 6845 CRT controller.
 * This code works exclusively in real-mode or v86 mode.
 * It is essential to understand real-mode addressing.
 * The only task done at the moment is write to the 6845 CRT controller to 
 * initialize the cursor.
 * The second part of the program works in 32-bit protected-mode.
 * It repositions the address and cursor to the value passed as argument.
 */
.set NIL, -1

.code16
.globl new_int10, update_cursor, update_position 
.text
/*
 * This handler for interrupt 0x10 is used only for initializing the cursor in
 * real-mode. Quickly we switch to protected-mode and install the appropiate
 * handler for the display part of the terminal.
 */
new_int10:
/*
 * This code replaces the interrupt handler provided by the BIOS to print to
 * the display.
 */
	movb initialize, %al # do initialization just once
	testb %al, %al # do initialize?
	jnz 1f
	call initialize_routine # initialize cursor
	movb $1, initialize
1:
	ret

/*
 * Just bring up a funny cursor.
 */
initialize_routine:
	movw $7, %cx # number of loops
	movw $reg_select, %bx # pointer to beginning of registers to select
	movw $reg_values_out, %si # pointer to beginning of values to out
	movw $reg_values_in, %di # pointer to beginning of values to in
	movw base_io, %dx # base io address
1:
	pushw %dx # save base io address
	movw (%bx), %ax # extract register to select
	incw %bx # point to next register
	xorb %ah, %ah # io with 6845 is byte oriented
	pushw %ax # save register to select
	outb %al, %dx # select a register
	incw %dx # port address to read/write data
output:
	movw (%si), %ax # load next value to out
	xorb %ah, %ah # only interested in low bits
	cmpb $NIL, %al # is there something to write?
	je input
	outb %al, %dx # write register
input:
	incw %si # point to next out value
	movw (%di), %ax # load next value to in
	xorb %ah, %ah
	cmpb $NIL, %al # is there a value to read?
	je end_loop
	inb %dx, %al # read register
end_loop:
	addw $2, %sp # restore stack
	incw %di # point to next in value
	popw %dx # restore base io address
	loop 1b
	ret

.data
reg_select:
	.byte 10, 11, 12, 13, 14, 15
reg_values_out:
	.byte 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00
reg_values_in:
	.byte NIL, NIL, NIL, NIL, NIL, NIL
initialize:
	.byte 0x00
.code32
/*
 * Routine to update the position of the cursor in screen.
 */
update_cursor:
	pushl %ebp
	movl %esp, %ebp
	movl 8(%ebp), %ebx # copy cursor address
	movb $15, %al # select low bits of cursor position register
	movl base_io, %edx # base address
	outb %al, %dx # select register
	incl %edx # data io
	movb %bl, %al # write low bits
	outb %al, %dx # write low bits to register
	movb $14, %al # select high bits of cursor position register
	movl base_io, %edx # base address
	outb %al, %dx # select register
	incl %edx # data io
	inb %dx, %al # read high register to mask bits
	andb $0xc0, %al # mask bits 0 through 5 of address read
	andb $0x3f, %bh # mask bits 6 and 7 of next address
	orb %bh, %al # write high bits
	movb %al, %ah # save bits
	movb $14, %al
	movl base_io, %edx
	outb %al, %dx
	incl %edx
	movb %ah, %al
	outb %al, %dx # write to register
	popl %ebp
	ret

/*
 * Routine to update the address space position.
 */
update_position:
	pushl %ebp
	movl %esp, %ebp
	movl 8(%ebp), %ebx # copy address
	movb $13, %al # select low bits of position register
	movl base_io, %edx # base address
	outb %al, %dx # select register
	incl %edx # data io
	movb %bl, %al # copy low bits
	outb %al, %dx # write low bits to register
	movb $12, %al # select high bits of position register
	movl base_io, %edx # base address
	outb %al, %dx # select register
	incl %edx # data io
	movb %bh, %al # copy high bits
	outb %al, %dx # write high bits
	popl %ebp
	ret
