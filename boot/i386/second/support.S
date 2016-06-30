/*
 * Low level support routines.
 */

/*
 * Control definitions.
 * This is used only at probe_bios().
 */
.set	EOI, -1 # end of interrupts mark
.set	EOF, -2 # end of functions mark
.set	EOP, -3 # end of parameters mark

.code16
.globl hook_intX, hexdump16, hex2ascii, putstr, go_bios, probe_bios, low_printf, no_bios, copy_bytes, getscanco, getbaseio, int_cli, int_sti, getasciico, low_memcpy, low_putstr
.text
/*
 * This function makes possible replacing an interrupt or exception handler
 * provided by the BIOS in the IVT with an arbitrary handler.
 * void hook_intX(void *new_handler, void *old_handler, int vec_num)
 */
hook_intX:
	pushw %si # save interrupt vector number
	pushw %ax # save new handler offset
	movw %si, %ax # save interrupt number
	movw $4, %bx # scale it
	mulw %bx # multiply vec_num by 4
	movw %ax, %si # interrupt handler = vec_num * 4
	movw (%si), %bx # copy old handler offset
	movw 2(%si), %cx # copy old handler segment
	movw %bx, (%di) # save offset
	movw %cx, 2(%di) # save segment
	popw %ax # restore new handler offset
	movw %ax, (%si) # copy the new offset
	movw $0, 2(%si) # copy the new segment
	movw $1, %ax # number of variables to print
	popw %si # restore interrupt vector number
	pushw %si # print number
	pushw $'h' # format
	pushw $taking_str # string pointer
	movw $taking_str, %bx # beginning of string
	movw $taking_str_end, %cx #  less end of string
	subw %cx, %bx #  == string lenght
	pushw %bx # save string lenght
	addw $8, %sp # restore stack frame
	ret

/*
 * Hexadecimal dump of general-purpose registers in real mode.
 */
hexdump16:
/* This part of the code executes only one time. */
	pusha # push all general purpose registers
	movw $8, %cx # number of registers for looping
	pushw $regs_strs16 # push to pop right away
/* This is the main loop. */
loop_dump:
	popw %si # pop address of string of current register
	movw %cx, %di # save number of registers
	pushw %bx # save 1
	pushw %dx # save 2
	pushw %cx # save 3
	call putstr # print register name
	popw %cx # restore 3
	popw %dx # restore 2
	popw %bx # restore 1
	movw $4, %cx # display by nibbles
	movw $0xf000, %dx # extract first nibble
	popw %bx # pop next register
	pushw %si # save next register string address
1:
	movw %bx, %ax
	andw %dx, %ax # mask it to get a nibble
	testb %al, %al # test if it is high or low %ax
	jz high # it is high
	jmp low # it is low
high:
	movb %ah, %al # work with %al
low:
	pushw %ax # save %ax
	movb shift, %al # copy shift variable
	testb %al, %al # test it
	jnz shift1 # shift register if it is one
	movb $1, shift # next loop we will have to loop
	popw %ax # restore %ax
	jmp 3f # do not shift
shift1:
	popw %ax # restore %ax
	shrb $4, %al # shift register
	movb $0, shift # do not shift on next loop
3:
	cmpb $10, %al # test %al
	jb dump # jump if it is between 0-9
	call hex2ascii # call if it is between a-f
	jmp 2f # print nibble
dump:
	addb $0x30, %al # get ascii of number
2:
	movb %al, one_char_str # copy nibble to one character string
	movw $one_char_str, %si # save it to print it
	pushw %bx
	pushw %dx
	pushw %cx
	call putstr # print nibble
	popw %cx
	popw %dx
	popw %bx
	shrw $4, %dx # next nibble
	loop 1b # if not the four nibbles already
	movw %di, %cx
	loop loop_dump
	popw %si # out by one :S
	ret

/*
 * Maps a number to a hexadecimal ascii character.
 */
hex2ascii:
	cmpb $0xa, %al
	je a
	cmpb $0xb, %al
	je b
	cmpb $0xc, %al
	je c
	cmpb $0xd, %al
	je d
	cmpb $0xe, %al
	je e
	cmpb $0xf, %al
	je f
a:
	movb $'a', %al
	ret
b:
	movb $'b', %al
	ret
c:
	movb $'c', %al
	ret
d:
	movb $'d', %al
	ret
e:
	movb $'e', %al
	ret
f:
	movb $'f', %al
	ret

/*
 * Prints string of characters by calling bios_int()
 * on each character.
 * Note that printing a single character corresponds
 * to printing a one character string.
 */
/* void putstr(char *) */
putstr:
	movb no_bios, %ah
	testb %ah, %ah # using BIOS handler?
	jnz own_handler
1:
	lodsb /* load next byte */
	testb %al, %al
	jz end_handler
	movb $0xe, %ah # function
	movb $0, %bh # page
	int $0x10
	jmp 1b
2:
own_handler:
	movb $2, %al # just print
	int $0x10 # pass control to handler; %si points to string
end_handler:
	ret

/*
 * The BIOS interface is standard, so we can use it to get 
 * essential information for further configuring the system and,
 * ultimately, passing control to a small kernel.
 * Lets make this routine a very general loop:
 * Each interrupt service handler provided by the BIOS has its set
 * of functions. Functions, meanwhile, has its own set of parameters.
 * Loop through all the interrupts we are interested in. For each
 * interrupt, loop its number of functions. For each function, loop
 * its number of parameters.
 * Lets try to get what we care about from the BIOS so we can get 
 * rid of it.
 */
probe_bios:
	movw $int_nums, %si # point to vector of interrupt numbers
	movw $function_nums, %di # point to vector of function numbers
	movw $params_nums, %bp # point to vector of parameter numbers
/* Loop through interrupt numbers */
1:
	cmpb $EOI, (%si) # end of interrupts?
	je end_probe_bios # yes; go hunt some BIOS :P
	xorw %ax, %ax # will push low bits of %ax, so clean up bits
	movb (%si), %al
	addw $1, %si
	movw %ax, %dx # save interrupt number to %dx
/* Loop through function numbers */
2:
	pushw %dx # save it on stack
	movb (%di), %ah # load next function to %ah
	addw $1, %di
/* Loop through parameter numbers */
3:
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %al # save parameter
	popw %bp # restore pointer to argument vector
	addw $1, %bp
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %bh # save parameter
	popw %bp # restore pointer to argument vector
	addw $1, %bp
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %bl # save parameter
	popw %bp # restore pointer to argument vector
	addw $1, %bp
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %ch # save parameter
	popw %bp # restore pointer to argument vector
	addw $1, %bp
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %cl # save parameter
	popw %bp # restore pointer to argument vector
	addw $1, %bp
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %dh # save parameter
	popw %bp # restore pointer to argument vector
	addw $1, %bp
	pushw %bp # save pointer to argument vector
	movw (%bp), %dx # move to compare
	cmpb $EOP, %dl # test end of parameters
	je next_func # if yes, go to next function
	movb %dl, %dl # REDUNDANT
next_func:
	popw %bp # restore pointer to argument vector
	popw %dx # restore interrupt number
	pusha
	call go_bios # implement the current function
	popa
	addw $1, %bp
	cmpb $EOF, (%di) # end of functions?
	je next_interrupt # yes, go to next interrupt
	jmp 2b # loop back to next function
next_interrupt:
	addw $1, %di
	jmp 1b # loop back to next interrupt
end_probe_bios:
	ret

/*
 * Receives the function being called and parameters, so just
 * cause the interrupt.
 */
go_bios:
	pusha # save registers
	cmpw $0x10, %dx # video-service BIOS interrupt?
	je int10h
	cmpw $0x12, %dx # low-memory-service BIOS interrupt?
	je int12h
	cmpw $0x13, %dx # disk-service BIOS interrupt?
	je int13h
	cmpw $0x14, %dx # serial-line-service BIOS interrupt?
	je int14h
	cmpw $0x15, %dx # general-service BIOS interrupt?
	je int15h
	cmpw $0x16, %dx # keyboard-service BIOS interrupt?
	je int16h
	cmpw $0x1a, %dx
	je int1ah
	cmpw $0x1d, %dx
	je int1dh
int10h:
	int $0x10
	jmp end_int
int12h:
	int $0x12
	jmp end_int
int13h:
	movb $0x80, %dl # damn small number of registers...
	pushw %ax # save function
	int $0x13
	call parse_int13h
	popw %ax # restore
	jmp end_int
int14h:
	int $0x14
	jmp end_int
int15h:
	pushw %ax # save function
	int $0x15
	call parse_int15h
	popw %ax
	jmp end_int
int16h:
	int $0x16
	jmp end_int
int1ah:
	pushw %ax # save function
	int $0x1a
	call parse_int1ah
	popw %ax
	jmp end_int
int1dh:
	call parse_int1dh
	jmp end_int
end_int:
	popa
	ret

parse_int1dh:
	movw $io_offsets, %si # beginning of offsets
	movw $num_columns, %di # beginning of variables to save
	movw $0x400, %bx # base address of video information
	movw $4, %cx # loops
	xorb %ah, %ah
1:
	pushw %bx # save base address
	lodsb # load next variable
	addw %ax, %bx # point to correct entry
	movw (%bx), %bx # dereference
	movw %bx, (%di) # save variable
	addw $2, %di # point to next variable
	popw %bx # restore base address
	loop 1b
	movw num_rows, %ax
	xorb %ah, %ah
	movw %ax, num_rows
	movw mode, %ax
	xorb %ah, %ah
	movw %ax, mode
	ret

/*
 * Parse the results of the interrupt 10h
 */
parse_int10h:
	ret

parse_int13h:
	pushw %bp # form
	movw %sp, %bp #  stack frame
	cmpb $0x08, 4(%bp)
	je int13h_08h
int13h_08h:
	xorw %ax, %ax
	movb %dh, %al
	incb %al # maximum head number
	movw %ax, max_head # save it
	movb %dl, %al
	movw %ax, num_drives # number of drives in system
	movb %cl, %al # save
	movb %ch, %ah # save
	shl $2, %cl # %ch = top 2 bits of maximum track number
	movb %ah, %cl # %cl = bottom 8 bits of maximum track number
	andb $0x3, %ch # %ch = top 2 bits of maximum track number
	movw %cx, max_track
	andb $0x3f, %al # %al = 6 bit maximum sector number
	xorb %ah, %ah # clear top 8 bits of maximum sector number
	movw %ax, max_sector # maximum sector number
	popw %bp
	ret

parse_int15h:
	pushw %bp # form
	movw %sp, %bp #  stack frame
	cmpb $0x88, 4(%bp)
	je int15h_88h
int15h_88h:
	movw %ax, extended_memory
	popw %bp
	ret

parse_int1ah:
	pushw %bp # form
	movw %sp, %bp #  stack frame
	cmpb $0x02, 4(%bp)
	je int1ah_02h
int1ah_02h:
	movb %ch, hours
	movb %cl, minutes
	movb %dh, seconds
	popw %bp
	ret

/*
 * The classical printf() function to print arbitrary
 * strings and/or numbers.
 * int low_printf(int num, const char *string, int str_len)
 * Arguments are passed in the stack.
 */
low_printf:
	pushw %bp # form
	movw %sp, %bp #  stack frame
	movw %ax, %bx # save number of variables
	addw $4, %bp # point to string lenght
	movw (%bp), %cx # recover string lenght
	movw %cx, %dx # save TOTAL string lenght
	addw $2, %bp # point to string address
	movw (%bp), %di # recover string pointer
	movw $temp_str, %si # pointer to temporary area
1:
	testw %bx, %bx # test number of variables
	jz end_printf
	pushw %di # save original string pointer
	movw $'%', %ax	
	repne
	scasb
	popw %di
	call copy_str
	xchg %si, %di # they work reversed in this function
	addw $4, %bp # point to number
	movw (%bp), %ax # save number
	pushw %cx # save left string lenght
	pushw %dx # save total string lenght
	call print_hex
	popw %dx
	popw %cx
	decw %bx
	jmp 1b
end_printf:
	movw $'\0', %ax
	pushw %di
	repne
	scasb
	popw %di
	call copy_str
	movw $temp_str, %si
	call putstr
	popw %bp
	ret

/*
 * Print hexadecimal of a number.
 */
print_hex:
	movw $2, %cx # display by nibbles
	movw $0xf0, %dx # extract first nibble
1:
	pushw %ax # save number
	andw %dx, %ax # mask it to get a nibble
	testb %al, %al # test if it is high or low %ax
	jz high1 # it is high
	jmp low1 # it is low
high1:
	movb %ah, %al # work with %al
low1:
	pushw %ax # save %ax
	movb shift, %al # copy shift variable
	testb %al, %al # test it
	jnz shift2 # shift register if it is one
	movb $1, shift # next loop we will have to loop
	popw %ax # restore %ax
	jmp 3f # do not shift
shift2:
	popw %ax # restore %ax
	shrb $4, %al # shift register
	movb $0, shift # do not shift on next loop
3:
	cmpb $10, %al # test %al
	jb dump2 # jump if it is between 0-9
	call hex2ascii # call if it is between a-f
	jmp 4f
dump2:
	addb $0x30, %al # get ascii of number
4:
	movb %al, (%si) # overwrite with ascii number
	incw %si # go to next space
	shrw $4, %dx # next nibble
	popw %ax
	loop 1b # if not the four nibbles already
	ret

/*
 * Displace string.
 */
copy_str:
	xchg %si, %di # they work reversed in this function
	movw %dx, %ax
	subw %cx, %ax # less string lenght to % character
	jz end_disp # if end % of file % ?
	movw %ax, %cx
	rep
	movsb
	movw %dx, %cx
	decw %di
end_disp:
	ret

/*
 * Copy physical bytes from %ds:%si to %es:%di the number at %cx
 */
copy_bytes:
	pushw %es # save VRAM segment
	pushw $0 # new segment
	popw %es # load it
	rep
	movsb
	popw %es # restore VRAM segment
	ret

tmp_str:
	.fill 255, 1, 0

.data
no_bios:
	.byte 0x00

.code32
.text
getbaseio:
	movl base_io, %eax
	ret

getscanco:
	movl key_scanco, %eax
	ret

getasciico:
	movl key_asciico, %eax
	ret

int_sti:
	sti
	ret

int_cli:
	cli
	ret

low_memcpy:

low_putstr:
	int $0x10
	ret
