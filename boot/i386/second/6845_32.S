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

.globl update_cursor, update_position 
.text
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
