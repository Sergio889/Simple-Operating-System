/*
 * New interrupt handler for the 8042 keyboard controller.
 */

.set KBD_IO_DATA, 0x60
.set KBD_IO_STAT, 0x64
.set NIL, -1

.globl new_int09, new_int09_32
.code16
/*
 * This handler replaces the one provided by the BIOS for handling the
 * keyboard part of the terminal.
 */
new_int09:
	pusha # save general-purpose registers
	inb $KBD_IO_DATA, %al # retrieve code
	movb %al, %ah # save scan code
	andb $0x80, %al # key releases has the most significant bit on
	testb %al, %al # test for key release
	jz 2f # it's a key press
	movb %ah, %al # restore scan code
	jmp 3f # it's a key release
2:
	inb $KBD_IO_STAT, %al # retrieve controller status
	andb $1, %al # output buffer full?
	jnz 2b # yes; wait until it empties
3:
	cli
	movb $0x20, %al # confirm interrupt
	movw $0x20, %dx #  to 8259 controller
	outb %al, %dx
	sti
	movb %ah, temp # save result
	popa # restore general-purspose registers
	movb temp, %al # restore result
	movl %eax, key_scanco
	iret # resturn from interrupt
.data
temp:
	.byte 0x00

.code32
.text
/*
 * This handler replaces the one provided by the BIOS for handling the
 * keyboard part of the terminal.
 */
new_int09_32:
	inb $KBD_IO_DATA, %al # retrieve code
	movb %al, %ah # save scan code
	andb $0x80, %al # key releases has the most significant bit on
	testb %al, %al # test for key release
	jz 2f # it's a key press
	movb %ah, %al # restore scan code
	jmp 3f # it's a key release
2:
	inb $KBD_IO_STAT, %al # retrieve controller status
	andb $1, %al # output buffer full?
	jnz 2b # yes; wait until it empties
3:
	cli
	movb $0x20, %al # confirm interrupt
	movl $0x20, %edx #  to 8259 controller
	outb $0x20
	sti
	movb %ah, temp1 # save result
	movb temp1, %al # restore result
	movl %eax, key_scanco
	ret # resturn from interrupt
.data
temp1:
	.byte 0x00
