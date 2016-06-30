/*
 * This part of the monitor is the one that most interfaces with
 * the client intended to run (i.e. the kernel).
 * Here is where the reprogramming of the 8259 controller happens, where we
 * copy the real-mode interrupt vector so we don't use reserved vectors from
 * Intel and where the protected-mode Interrupt Descriptor Table is created.
 * This table holds handler functions for servicing exceptions and interrupts. 
 * Each entry in the table is 8 byte long.
 * The first 32 entries, that is, the first 256 bytes, are reserved for Intel
 * internal exceptions. Entries above are user-defined. The problem here is
 * that we can't do IO until device-drivers are loaded, so we must continue
 * using the handlers provided by the BIOS or the ones we constructed while in
 * real-mode. Those handlers can only run in real-mode, so, until we load the
 * appropiate device-drivers for use in protected-mode, we must switch
 * between protected-mode and real-mode when doing IO, and return to
 * protected-mode from real-mode when IO is complete.
 */

/*
 * When in protected mode, i386 processors interpret segment registers as
 * segment selector index values inside the Global Descriptor Table (GDT).
 * The address of the GDT is stored in the Global Descriptor 
 * Table Register (GDTR).
 * When in real mode, any X86 processor directly maps segment registers to
 * physical address, AFTER shifting the segment register 4 bits to the right,
 * and adding it to a 16 bit offset.
 */
.set	SCODE_SEGSEL, 0x8 # segment selector for supervisor code
.set	SDATA_SEGSEL, 0x10 # segment selector for supervisor data and stack
.set	RCODE_SEGSEL, 0x18 # segment selector for real code 
.set	RDATA_SEGSEL, 0x20 # segment selector for real data/stack 
.set	UCODE_SEGSEL, 0x28|3 # segment selector for user code
.set	UDATA_SEGSEL, 0x30|3 # segment selector for user data and stack
.set	TSS_SEGSEL, 0x28 # segment selector for only task
.set	SSTACK_OFF, 0x5000 # supervisor stack offset in TSS
.set	NUM_NEST_INTS, 12 # number of nested interrupts
.set	PG_ENABLE, ~0x7fffffff
.set	REAL_STRING_BUF, 0x2500

/*
 * Definitions for some constants for programming the 8259 IPC.
 */
.set INT_MST, 0x20 /* IO port for master 8259 */
.set INT_SLV, 0xA0 /* IO port for slave 8259 */
.set NUM_HANDLS, 48 # number of handlers to set
.set NUM_EXCNS, 32 # number of exceptions to handle
.set NUM_INTS, 16 # number of interrupts to handle
/*
 * This constants are used when issuing commands while reprogramming
 * the 8259 interrupt controllers.
 */
.set ICW1_LEVEL, 0x1c /* level-triggered, cascade, need ICW4 */
.set ICW1_EDGE, 0x11 /* edge-trigerred, cascade, need ICW4 */
.set ICW2_MST, 0x20 /* relocate because of BIOS locations */
.set ICW2_SLV, 0x28 /* relocate because of protected-mode strategy */
.set ICW3_MST, 4
.set ICW3_SLV, 2
.set ICW4_MST, 0x0d /* ~FNM, not buffered, master, 8086 */
.set ICW4_SLV, 0x09 /* ~FNM, buffered, slave,  8086 */

/*
 * Memory locations same as in FreeBSD.
 */
.set	LOADADDR, 0x9000 # $LOADADDR:$0 where this program is copied
.set	NEXTLOAD, 0x2000
.set	MEM_SSTACK, 0x2000 # supervisor stack
.set	MEM_USTACK, 0xffff # user stack
.set	STACK, 0x9000 # $STACK:$0 beginning of stack
.set	MEM_TSS, 0x5f98 # $MEM_TSS:$0 beginning of task segment
.set	MEM_TSS_END, 0x7fff # $MEM_TSS:$0 end of task segment
.set	MEM_IDT, 0x5e00 # $MEM_IDT:$0 address of Interrupt Descritor Table
.set	MEM_MAP, 0x6000

/*
 * Relocate some interrupt handlers because their vectors will be updated.
 */
.set MST_IRQS_ADDR, 32 # start address of handlers to relocate
.set SLV_IRQS_ADDR, 448 # start address of handlers to relocate
.set MST_NEW_VECTOR, 0x20 # vector for IRQ0
.set MST_NEW_VECTOR_ADDR, 128 # start address to copy handlers
.set SLV_NEW_VECTOR_ADDR, 160 # start address to copy handlers
.set BYTES_RELOCATE, 32 # number of bytes to copy

.code16
.text
.globl init_themon, idtreg, key_scanco, key_asciico, ptr_str_term
init_themon:
	call a20_enable # enable addressing past 1MB
/*
 * Relocate IRQ's 0-7 from vectors 8-15 to vectors
 * 32-40 and IRQ's 8-15 from vectors 16-23 to vectors 40-48.
 */
	movw $MST_IRQS_ADDR, %si # initial master handlers address
	movw $MST_NEW_VECTOR_ADDR, %di # final master handlers address
	movw $BYTES_RELOCATE, %cx # number of bytes
	call copy_bytes # relocate vector
	movw $SLV_IRQS_ADDR, %si # initial slave handlers address
	movw $SLV_NEW_VECTOR_ADDR, %di # final slave handlers address
	movw $BYTES_RELOCATE, %cx # number of bytes
	call copy_bytes # relocate vector
/*
 * Initialize the 8259's.
 * This step is quite simple, the only thing to notice is the vector change
 * for the IRQ's.
 */
	call init_int # initialize 8259's

/*
 * Initialize the IDT.
 */
	call setidt
	ret

/*
 * Before switching to protected mode, this small step is essential:
 * In real-mode the processor has only an address space of (2^20)-1,
 * but we need space to securely load the kernel without affecting the
 * well known memory mappings at the first megabyte of physical memory.
 * Because we assume to be working with a modern microprocessor of the
 * i386 line, we can enable one more bit in the address space (this is
 * the popular A20 address-line) so we can access addresses above 1MB.
 * This will increase the range from 1MB-1B to (1MB+64KB)-1B, and allow 
 * using addresses above when in protected-mode, thus allowing us to load 
 * the kernel using a special BIOS call that switches to protected-mode and
 * copies to extended memory (memory 32-bit wide)(although it is possible that
 * BIOS would enable the line automatically before copying to extended memory,
 * doing it won't make harm).
 */
/* 
 * Try to enable the a20 line through System Control Port A.
 * This is most likely to work, but in case it doesn't, the implications of
 * not enabling a20 line are yet to be devised...
 */
a20_enable:
	movw $0x92, %dx # port to enable A20 line
	inb %dx, %al # read port
	orb $2, %al # write one to the second byte, masking the rest
	movw $0x92, %dx # port to enable A20 line
	outb %al, %dx # enable A20 line
	ret

/*
 * This routine initializes the interrupts controllers with the only rare
 * initialization done at ICW2's. This is because if we are going to switch
 * to protected-mode, the interrupt lines must provide a vector different from
 * the Intel reserved vectors. So, we relocate the irq's 0-7 to 0x20-0x28.
 * We also relocate the slave's irq's 8-15 to 0x28-0x30.
 */
init_int:
	inb $0x21, %al # read master's 8259 state and
	pushw %ax #  save it
	inb $0xa1, %al # read slave's 8259 state and
	pushw %ax #  save it
	movb $ICW1_LEVEL, %al # start init sequence at master
	outb %al, $0x20
	outb %al, $0xa0
	movb $ICW2_MST, %al # use MST_NEW_VECTOR_ADDR for irq's 0-7
	outb %al, $0x21
	movb $ICW2_SLV, %al # use SLV_NEW_VECTOR_ADDR for irq's 8-15
	outb %al, $0xa1
	movb $ICW3_MST, %al # mask out slave irq of slave controller
	outb %al, $0x21
	movb $ICW3_SLV, %al # write master controller
	outb %al, $0xa1
	movb $ICW4_MST, %al # other...
	outb %al, $0x21
	movb $ICW4_SLV, %al # other...
	outb %al, $0xa1
	popw %ax # restore slave's state
	outb %al, $0xa1 # restore it
	popw %ax # restore master's state
	outb %al, $0x21 # restore it
	ret

/*
 * In protected-mode, interrupts and exceptions that the processor receive
 * represent an entry in the Interrupt Descriptor Table. Each entry is
 * 8 bytes long, and ultimately transfers control to a interrupt/exception
 * handler.
 * The first 32 entries in the table are reserved by Intel for various
 * architecture-specific exceptions. Entries above are user-defined.
 * From the monitor program, we must recall that the IRQ lines of the
 * master interrupt controller were modified to start interrupts at vector
 * 32 (0x20), and from slave ineterrupt controller to start at vector
 * 38 (0x28) so when we receive those vectors, the processor will scale them
 * by 8 and find the entry in the IDT for the handler. These handlers save
 * the corresponding value of the interrupt and switch to real-mode, where
 * the correct IVT will be loaded and the correct call be issued by
 * software.
 */
setidt:
	movw $NUM_HANDLS, %cx # loop number of handlers times
	movw $NUM_EXCNS, %dx # loop number of exception handlers times
	movw $handls_ctrl, %si # control string
	movw $MEM_IDT, %di # IDT pointer
	xorw %bx, %bx # scale control register
next_handler:
	testw %dx, %dx # end of exceptions?
	jnz 2f # no
	movw end_handls, %ax # load end of handlers bool variable
	testw %ax, %ax # end of handlers?
	jnz end_setidt # yes
	incw %ax
	movw %ax, end_handls
	xorw %bx, %bx # reset scale control
	addw $8, %si # now point to interrupts
	movw $NUM_INTS, %dx # loop number of interrupt handlers times
2:
	movw (%si), %ax # load handler
	pushw %bx # save scale factor
	pushw %ax # save it
	movw $12, %ax # handler size
	mulw %bx # scale
	xchgw %bx, %ax # exchange values
	popw %ax # restore handler
	addw %bx, %ax # add offset
	popw %bx # restore scale factor
	movw %ax, (%di) # copy handler to IDT
	movw 2(%si), %ax # load segment selector
	movw %ax, 2(%di) # copy it to IDT
	movw 4(%si), %ax # load control information
	movw %ax, 4(%di) # copy it to IDT
	movw 6(%si), %ax # load offset continuation
	movw %ax, 6(%di) # copy it to IDT
	addw $8, %di # point to next entry in IDT
	decw %dx # one exception/interrupt less
	incw %bx # one scale factor more
	loop next_handler
end_setidt:
	ret

/*
 * When an interrupt or exception is generated in protected-mode, control
 * will ultimately lead here. We are already in real-mode and have the vector
 * number in the handl_arg variable.
 */
int16_excpns:
	movb handl_arg, %al # test if not really and exception
	cmpb $0x10, %al
	je 1f
	cmpb $0x13, %al
	je 2f
	cmpb $0x15, %al
	je 3f
	cmpb $25, %al
	je 4f
	cmpb $22, %al
	je pnp_bios
1:
	movw $REAL_STRING_BUF, %si
	int $0x10
	jmp end_int16
2:
	movw $ext_bios_regs, %si
	movl (%si), %eax
	movl 4(%si), %ebx
	movl 8(%si), %ecx
	movl 12(%si), %edx
	pushw %es # save segment
	pushw $0 # new segment
	popw %es # load it
	movw $ext_das_struct, %si # das packet
	movb drive_num, %dl # drive number
	int $0x13 # extended read or write
	jc error
	movb $0x87, %ah
	movw $0x100, %cx
	movw $ext_mem_gdt_real, %si	
	int $0x15
	popw %es # restore segment
	jmp end_int16
3:
	movw $ext_bios_regs, %di
	movl (%di), %eax
	movl 4(%di), %ebx
	movl 8(%di), %ecx
	movl 12(%di), %edx
	pushw %es
	pushw $0
	popw %es
	xorl %ebx, %ebx
	movl $0x534d4150, %edx # magic number
	movl $0xe820, %eax # function number
	movl $24, %ecx # number of bytes
	movw $ext_mem_ents, %di # buffer to store entries
	movl $1, 20(%di) # make it ACPI 3.0 compatible
mem_call:
	int $0x15 # return extended memory
	movl $0xe820, %edx # value will be exchanged
	xchgl %eax, %edx # exchange values
	cmpl $0x534d4150, %edx # must be this magic value
	jne error
	movl $0, 24(%di) # NULL pointer
	addw $28, %di # next entry (acpi 3.0)
	movl $24, %ecx
	testl %ebx, %ebx # last entry?
	jnz mem_call # no
	movl $0x12345, 20(%di)
end_mem_call:
	movb $0x87, %ah
	xorb %al, %al
	movw ext_mem_ents, %cx
	movw $ext_mem_gdt_real, %si	
	int $0x15
	popw %es # restore segment
	jmp end_int16
4:
	pushw %es
	pushw $0
	popw %es
	movb $0x87, %ah
	movw kargs_size, %cx
	movw $ext_mem_gdt_real, %si	
	int $0x15
	popw %es # restore segment
	jmp end_int16
/*
 * Cumbersome interface to PnP BIOS functions.
 */
pnp_bios:
	movw pnp_num, %ax # load number of arguments
	shlw $2, %ax # multiply by 4
	movw $pnp_args, %si # pointer to arguments
	addw %ax, %si # scale; point to last argument saved
	subw $4, %si # scale; point to last argument saved
	movw pnp_num, %bx # load number of arguments
1:
	movw (%si), %ax # save argument
	pushw %ax # push it
	decw %bx # one argument less
	subw $4, %si # next argument
	testw %bx, %bx # end of arguments?
	jnz 1b # no
	movw $pnp_call, %bx
	movw pnp_off, %ax
	movw %ax, (%bx)
	addw $2, %bx
	movw pnp_seg, %ax
	movw %ax, (%bx)
	movw pnp_num, %dx
	movl $5, %eax
	lcall *(pnp_call) # call BIOS pnp function
	movl %eax, pnp_tmp
	shlw $1, %dx # multiply by 2
	addw %dx, %sp # restore stack
/* copy real-mode buffers to protected-mode buffers */
	pushw %bp # save
	xorw %bp, %bp # counter
1:
	movw $pnp_16, %bx # real-mode buffers
	shlw $2, %bp # multiply by 4
	addw %bp, %bx # point to buffer
	shrw $2, %bp # divide by 4
	movw %bx, %si # pointer to buffer
	movw $(ext_mem_gdt_real+18), %di # point to source address in struct
	movw $3, %cx
	rep
	movsb
	movw $(ext_mem_gdt_real+26), %di # point to destination address in struct
	movw $pnp_32, %ax # protected-mode buffers
	shlw $2, %bp # multiply by 4
	addw %bp, %ax # point to buffer
	shrw $2, %bp # divide by 4
	movw %ax, %si # destination address
	movw $3, %cx
	rep
	movsb
	pushw %es
	pushw $0
	popw %es
	movl pnp_args_size, %ecx
	movw %bp, %ax
	jmp 3f
2:
	shrl $8, %ecx # shift byte right
	decw %ax
3:
	testw %ax, %ax
	jnz 2b
	andl $0xff, %ecx # mask high bytes
	movb $0x87, %ah
	movw $ext_mem_gdt_real, %si
	pusha
	int $0x15
	popa
	popw %es # restore segment
	incw %bp
	cmpw pnp_bufs, %bp
	jne 1b
	popw %bp
	jmp end_int16

int16_ints:
	xorl %eax, %eax
	xorl %ebx, %ebx
	movb handl_arg, %al
	subb $32, %al
	movb $4, %bl
	mull %ebx
	addw $int16_init.1, %ax
	jmp *%ax
int16_init.1:
	int $32 # IRQ0: 18.2hz timer tick
	jmp end_int16
	int $33 # IRQ1: keypress interrupt
	jmp end_int16
	int $34 # IRQ2: cascade to slave 8259
	jmp end_int16
	int $35 # IRQ3: COM 2
	jmp end_int16
	int $36 # IRQ4: COM 1
	jmp end_int16
	int $37 # IRQ5: LPT 2
	jmp end_int16
	int $38 # IRQ6: floppy
	jmp end_int16
	int $39 # IRQ7: LPT 1
	jmp end_int16
	int $40 # IRQ8: Real-time clock (1khz)
	jmp end_int16
	int $41 # IRQ9
	jmp end_int16
	int $42 # IRQ10
	jmp end_int16
	int $43 # IRQ11
	jmp end_int16
	int $44 # IRQ12: Mouse
	jmp end_int16
	int $45 # IRQ13: Math coprocessor
	jmp end_int16
	int $46 # IRQ14: IDE services
	jmp end_int16
	int $47 # IRQ15: APM suspend
end_int16:
	cli # disable interrupts while restoring protected-mode
end_int16_nocli:
	lgdt gdtreg
	lidt idtreg
	movl %cr0, %eax # load cr0
	orl $1, %eax # enable protection bit
	movl %eax, %cr0 # return to protected-mode
	ljmp $SCODE_SEGSEL, $reentry_eip

.code16
.data
excp_str:
	.ascii "|================================================|\n"
	.ascii "Unrecoverable Exception!\n"
	.asciz "|================================================|\n"
int_num:
	.byte 0x00
page_on:
	.byte 0x00
seg_on:
	.byte 0x00
int_or_excpn:
	.byte 0x00
handl_arg:
	.long 0x00
end_handls:
	.word 0x0000
irq_rel_str:
	.ascii "\r\nIRQ's from master interrupt controller were relocated to vector 0x20"
	.asciz "\r\nIRQ's from slave interrupt controller were relocated to vector 0x28\r\n"
string_string:
	.asciz "1"
idtreg:
	.word (MEM_TSS-MEM_IDT)-1, MEM_IDT, 0x0000
ivtreg:
	.word 0x400-1, 0, 0
handls_ctrl:
	.word exptn_handls
	.word SCODE_SEGSEL
	.byte 0x00, 0x8e, 0x00, 0x00
	.word int_handls
	.word SCODE_SEGSEL
	.byte 0x00, 0x8e, 0x00, 0x00
ext_das_struct:
	.fill 16, 1, 0
ext_rw_buf:
	.fill 512, 1, 0
ext_mem_gdt_real:
	.fill 16, 1, 0
	.word 0xffff # source segment lenght
	.byte 0x00, 0x00, 0x00 # source offset
	.byte 0x92 # must be
	.word 0x0000 # must be
	.word 0xffff # destination segment lenght
	.byte 0x00, 0x00, 0x00 # destination offset
	.byte 0x92 # must be
	.word 0x0000
	.fill 16, 1, 0
ext_bios_regs:
	.fill 16, 1, 0
addr_copy:
	.long 0x00000000
ext_num_ents:
	.long 0x00000000
ext_mem_ents:
	.fill 512, 1, 0
pnp_call:
	.word 0, 0
reb_mac:
	.byte 0x00

/*
 * This code IS NOT ACCESSIBLE FROM THIS FILE/PROGRAM.
 * We will land here when receiving an exception or interrupt while in
 * protected-mode. This code is used by the kernel to do IO from BIOS
 * while device-drivers are not loaded.
 * We use the stack to save the machine state when receiving a signal, but
 * because stack space is limited, only a finite number of nested interrupts
 * are allowed.
 */
.code32
.set PG_ENABLED, ~0x7fffffff
.set PG_DISABLE, 0x7fffffff
.set PROT_ENABLED, 0x1
.set PROT_DISABLE, ~0x1
.text
/*
 * Generic handler for exceptions.
 * Chaos start!
 */
exptn_handls:
	movb $0, handl_arg
	jmp go16excps
	movb $1, handl_arg
	jmp dbg_serv
	movb $2, handl_arg
	jmp go16excps
	movb $3, handl_arg
	jmp go16excps
	movb $4, handl_arg
	jmp go16excps
	movb $5, handl_arg
	jmp go16excps
	movb $6, handl_arg
	jmp go16excps
	movb $7, handl_arg
	jmp go16excps
	movb $8, handl_arg
	jmp go16excps
	movb $9, handl_arg
	jmp go16excps
	movb $10, handl_arg
	jmp go16excps
	movb $11, handl_arg
	jmp go16excps
	movb $12, handl_arg
	jmp go16excps
	movb $13, handl_arg
	jmp go16excps
	movb $14, handl_arg
	jmp page_serv
	movb $15, handl_arg
	jmp go16excps
	movb $16, handl_arg
	jmp vid_serv
	movb $17, handl_arg
	jmp go16excps
	movb $18, handl_arg
	jmp go16excps
	movb $19, handl_arg
	jmp disk_serv
	movb $20, handl_arg
	jmp go16excps
	movb $21, handl_arg
	jmp ext_serv
	movb $22, handl_arg
	jmp pnp_serv
	movb $23, handl_arg
	jmp reboot_serv
	movb $24, handl_arg
	jmp go16excps
	movb $25, handl_arg
	jmp temp_serv
	movb $26, handl_arg
	jmp go16excps
	movb $27, handl_arg
	jmp go16excps
	movb $28, handl_arg
	jmp go16excps
	movb $29, handl_arg
	jmp go16excps
	movb $30, handl_arg
	jmp go16excps
	movb $31, handl_arg
	jmp go16excps
/*
 * Generic handlers for interrupts.
 * Chaos start!
 */
int_handls:
	movb $32, handl_arg
	jmp clock_serv
	movb $33, handl_arg
	jmp key_serv
	movb $34, handl_arg
	jmp end_caos
	movb $35, handl_arg
	jmp end_caos
	movb $36, handl_arg
	jmp end_caos
	movb $37, handl_arg
	jmp end_caos
	movb $38, handl_arg
	jmp end_caos
	movb $39, handl_arg
	jmp end_caos
	movb $40, handl_arg
	jmp end_caos
	movb $41, handl_arg
	jmp end_caos
	movb $42, handl_arg
	jmp end_caos
	movb $43, handl_arg
	jmp end_caos
	movb $44, handl_arg
	jmp end_caos
	movb $45, handl_arg
	jmp end_caos
	movb $46, handl_arg
	jmp end_caos
	movb $47, handl_arg
	jmp end_caos

go16excps:
	popl %eax
	popl %ebx
	popl %ecx
	pushl $buffer
	pushl handl_arg
	pushl %eax
	pushl %ebx
	pushl %ecx
	call excpdump32
	pushl $buffer
	call low_putstr
	jmp .

dbg_serv:
	pushl %ebp
	movl %esp, %ebp
	subl $16, %ebp
	pushl %ebp # for saving the context's %esp
	movl %esp, %ebp
	subl $8, %esp # local variable
	cli # disable for manipulating heap list
	call save # save context on heap
	sti
	movl %eax, -4(%ebp) # save pointer to heap
	pushl %eax
	pushl 8(%ebp)
	pushl 12(%ebp)
	leal 16(%ebp), %eax
	pushl %eax
	call dbg_main
	pushl -4(%ebp) # pointer to heap space
	cli
	call restore
	sti
	addl $32, %esp
	popl %ebp
	jmp end_caos

/*
 * Temporary handler.
 */
page_serv:
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp # local variable
	cli # disable for manipulating heap list
	call save # save context on heap
	movl %eax, -4(%ebp) # save pointer to heap
	movl %cr2, %eax # faulting linear address
	pushl %eax # as argument
	call page_serv_high
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $8, %esp
	popl %ebp
	jmp end_caos

/*
 * Temporary handler.
 */
ext_serv:
	movl $ext_bios_regs, %esi # load new stack
	movl %eax, (%esi) # save registers buffer
	movl %ebx, 4(%esi) # save registers buffer
	movl %ecx, 8(%esi) # save registers buffer
	movl %edx, 12(%esi) # save registers buffer
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp # local variable
	cli # disable for manipulating heap list
	call save # save context on heap
	movl %eax, -4(%ebp) # save pointer to heap
	movl $ext_mem_ents, addr_copy # pointer to buffer
	movl $addr_copy, %esi
	movl $(ext_mem_gdt_real+18), %edi # point to source address in struct
	movl $3, %ecx
	rep
	movsb
	movl $(ext_mem_gdt_real+26), %edi # point to destination address in struct
	movl 20(%ebp), %eax
	movl %eax, addr_copy # destination address
	movl $addr_copy, %esi
	movl $3, %ecx
	rep
	movsb
	call go_real
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $12, %esp
	popl %ebp
	jmp end_caos

/*
 * Stack if we get here:
 * 4(%ebp): EIPlow, 8(%ebp): CS, 12(%ebp): EFLAGS, 16(%ebp): EIPtemp, 
 * 20(%ebp): pointer buffer, 24(%ebp): struct size.
 */
temp_serv:
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp # local variable
	cli # disable for manipulating heap list
	call save # save context on heap
	movl %eax, -4(%ebp) # save pointer to heap
	pushl 24(%ebp) # save...
	popl kargs_size #  ...and restore size of structure
	movl $kern_args, %eax
	movl %eax, addr_copy
	movl $addr_copy, %esi # pointer to buffer
	movl $(ext_mem_gdt_real+18), %edi # point to source address in struct
	movl $3, %ecx
	rep
	movsb
	movl $(ext_mem_gdt_real+26), %edi # point to destination address in struct
	movl 20(%ebp), %eax
	movl %eax, addr_copy
	movl $addr_copy, %esi # destination address
	movl $3, %ecx
	rep
	movsb
	call go_real
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $12, %esp
	popl %ebp
	jmp end_caos

clock_serv:
	pusha
	cli
	movb $0x20, %al # confirm interrupt
	movl $0x20, %edx #  to 8259 controller
	outb $0x20
	sti
	popa
	iret

/*
 * Protected-mode passes an extended disk packet through the stack and the
 * extended function through %ah.
 */
disk_serv:
	movl $ext_bios_regs, %esi # load new stack
	movl %eax, (%esi) # save registers buffer
	movl %ebx, 4(%esi) # save registers buffer
	movl %ecx, 8(%esi) # save registers buffer
	movl %edx, 12(%esi) # save registers buffer
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp # local variable
	cli # disable for manipulating heap list
	call save # save context on heap
	movl %eax, -4(%ebp) # save pointer to heap
	movl 28(%ebp), %esi # pointer to extended disk packet
	movl $ext_das_struct, %edi # copy it to
	movl $16, %ecx # copy 16 bytes
	rep
	movsb
	movl $ext_das_struct, %edi # pointer to extended disk packet
	movl $ext_rw_buf, 4(%edi) # copy segment:offset
	movl $(ext_mem_gdt_real+18), %edi # point to source address in struct
	movl $ext_rw_buf, addr_copy
	movl $addr_copy, %esi
	movl $3, %ecx
	rep
	movsb
	movl $0, addr_copy
	movl $(ext_mem_gdt_real+26), %edi # point to destination address in struct
	movl 24(%ebp), %eax
	movl %eax, addr_copy # destination address
	movl $addr_copy, %esi # destination address
	movl $3, %ecx
	rep
	movsb
	call go_real
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $12, %esp
	popl %ebp
	jmp end_caos

/*
 * Print to the screen.
 */
vid_serv:
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp
	cmpl $0, 20(%ebp) # is argument 0?
	je no_str
	movl 20(%ebp), %eax
	movl %eax, ptr_str_term # save pointer to string
	jmp str
no_str:
	movl $0, ptr_str_term # pointer is NULL
str:
	cli # disable for manipulating heap list
	call save
	movl %eax, -4(%ebp) # save pointer to local variable
	call term_handl_scr
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $12, %esp
	popl %ebp
	jmp end_caos

/*
 * Handle a keypress.
 */
key_serv:
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp
	cli # disable for manipulating heap list
	call save
	movl %eax, -4(%ebp) # save pointer to local variable
	call new_int09_32
	call term_handl_kbd
	pushl %eax # save character
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $4, %esp
	popl %eax # restore character
	addl $8, %esp
	popl %ebp
	jmp end_caos

/*
 * Temporary handler for PnP interface.
 * pnp_off, pnp_seg, num_args, buf_off, num_bufs, ...
 */
pnp_serv:
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp # local variables
	cli # disable for manipulating heap list
	call save # save context on heap
	movl %eax, -4(%ebp) # save pointer to heap
	movl $0, %eax
	movl $5, %ecx
	movl $pnp_32, %edi # clear variables
	rep
	stosl
	movl $5, %ecx
	movl $pnp_16, %edi # clear variables
	movl $24, %ecx
	movl $pnp_args, %edi # clear variables
	rep
	stosb
	movl $0, pnp_args_size
	movl 20(%ebp), %eax
	movl %eax, pnp_off # entry offset
	movl 24(%ebp), %eax
	movl %eax, pnp_seg # entry segment
	movl 36(%ebp), %eax
	movl %eax, pnp_bufs # number of buffer arguments
	movl 32(%ebp), %eax # offset where buffers begin from %esi
	movl %eax, pnp_bufs_off
	shll $2, %eax # multiply by 4
	leal 40(%ebp), %esi # start of arguments
	movl %esi, %edi # %edi=arguments before buffers
	addl %eax, %esi # %esi=arguments starting from buffers
	movl 28(%ebp), %edx # number of arguments
	movl %edx, pnp_num # save them
	movl $pnp_args, %ebx # where to store arguments
	jmp 2f
1:
	decl %edx # one argument less
	movl (%edi), %eax
	movl %eax, (%ebx) # push argument
	addl $4, %edi # next argument
	addl $4, %ebx # next argument
2:
	cmpl %esi, %edi # end of arguments before buffers?
	jne 1b # no
	movl pnp_bufs, %ecx # number of buffers arguments
	movl $pnp_args_size, %edi # where to store buffers' sizes
	testl %ecx, %ecx # do we have buffer arguments?
	jz 5f # no
	pushl %ebp # save
	xorl %ebp, %ebp # counter
	jmp 4f
3:
	addl $4, %esi # next argument
	addl $8, %ebx # next argument space
4:
	subl $2, %edx
	movl (%esi), %eax # copy
	movl %ebp, pnp_cur # for shifting
41:
	cmpl $0, pnp_cur # end of shifting?
	je 42f # yes
	shll $8, %eax # shift 1 byte
	decl pnp_cur
	jmp 41b
42:
	orl %eax, (%edi) # save size
	movl (%esi), %eax # copy
	addl $4, %esi # next argument
	pusha
	movl %eax, pnp_tmp
	movl $pnp_32, %eax # protected-mode buffers
	pushl %ebp # save
	shll $2, %ebp # multiply by 4
	addl %ebp, %eax
	movl (%esi), %ebp # store
	movl %ebp, (%eax) #  protected-mode buffer
	movl (%eax), %ebp # content of the buffer
	movl (%ebp), %eax
	movl %eax, pnp_cont
	popl %ebp # restore
	pushl %ebp # save
	shll $2, %ebp # multiply by 4
	movl $pnp_16, %eax # real-mode buffers
	addl %ebp, %eax # scale
	popl %ebp # restore
	pushl %eax # first argument to heap_alloc
	shrl $6, pnp_tmp # divide by 64
	incl pnp_tmp # add 1
	pushl pnp_tmp # second argument to heap_alloc
	movl %eax, pnp_tmp
	call heap_alloc
	movl pnp_tmp, %eax
	movl (%eax), %eax # address allocated
	movl pnp_cont, %edx
	movl %edx, (%eax) # copy 32-bit buffer content to 16-bit buffer
	movl %eax, %edx
	andl $0x0f, %edx
	movl %edx, (%ebx)
	addl $4, %ebx
	movl %eax, %edx
	shrl $4, %edx
	movl %edx, (%ebx) #  argument
	addl $8, %esp # restore stack
	popa
	incl %ebp
	decl %ecx
	testl %ecx, %ecx
	jnz 3b # next buffer argument
	addl $4, %esi # next argument
	addl $8, %ebx # next argument space
	popl %ebp # restore
5:
	testl %edx, %edx # end of arguments?
	jz end_args # yes
	jmp 7f # no
6:
	addl $4, %esi # next argument
	addl $4, %ebx # next argument
7:
	movl (%esi), %eax
	movl %eax, (%ebx)
	decl %edx
	testl %edx, %edx
	jnz 6b
end_args:
	movw $5, pnp_tmp
	call go_real # do whatever in real-mode
	cmpl $0, pnp_tmp
	jne pnp_error
	movl $pnp_16, %esi
8:
	cmpl $0, (%esi) # NULL pointer?
	je 9f # yes
	pushl (%esi) # address to free
	call heap_free # free address
	addl $4, %esp
	addl $4, %esi # next address
	jmp 8b
9:
	pushl -4(%ebp) # pointer to heap space
	call restore
	sti
	addl $12, %esp
	popl %ebp
	jmp end_caos

pnp_error:
	jmp .

reboot_serv:
	movb $1, reb_mac
	jmp go_real

end_caos:
	iret # return from interrupt

/*
 * Save context of previous executing flow.
 * Disable interrupts so while allocating from the heap an interrupt does not
 * potenitially corrupt the structure.
 */
save:
	pushl %ebp
	movl %esp, %ebp
	subl $8, %esp
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	leal -4(%ebp), %eax # pointer argument
	pushl %eax # argument
	pushl $1 # size argument
	call heap_alloc
	addl $8, %esp
	movl -4(%ebp), %ebp # point to heap space
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	movl %eax, (%ebp) # save
	movl %ebx, 4(%ebp) # save
	movl %ecx, 8(%ebp) # save
	movl %edx, 12(%ebp) # save
	movl %esi, 16(%ebp) # save
	movl %edi, 20(%ebp) # save
	movl %ebp, %eax # return address
	xchg %ebp, %esi
	addl $8, %esp
	popl %ebp
	movl (%ebp), %ebx
	movl %ebx, 24(%esi) # %esp
	movl 4(%ebp), %ebx
	movl %ebx, 28(%esi) # %ebp
	movl 8(%ebp), %ebx
	movl %ebx, 32(%esi) # %eip
	movl 16(%ebp), %ebx
	movl %ebx, 36(%esi) # eflags
	ret

/*
 * Restore context of previous executing flow.
 */
restore:
	pushl %ebp
	movl %esp, %ebp
	movl 8(%ebp), %ebp
	movl (%ebp), %eax # restore
	movl 4(%ebp), %ebx # restore
	movl 8(%ebp), %ecx # restore
	movl 12(%ebp), %edx # restore
	movl 16(%ebp), %esi # restore
	movl 20(%ebp), %edi # restore
	pusha # save registers
	pushl %ebp # pointer argument
	movl 24(%ebp), %ebp # restore
	call heap_free
	addl $4, %esp
	popa # restore registers
	popl %ebp
	ret

go_real:
	movl $MEM_TSS, %edi # pointer to task segment
	movl %cr3, %eax # load page directory address register
	movl %esp, 56(%edi) # save page directory address to TSS
	movl %eax, 32(%edi) # save stack pointer to TSS
	movl %cr0, %eax # load status register
	testl $PG_ENABLED, %eax
	jz no_pag
	movb $1, page_on
	andl $PG_DISABLE, %eax
	movl %eax, %cr0
no_pag:
	ljmp $RCODE_SEGSEL, $segtion # to 16-bit segment
.code16
segtion:
	movl $RDATA_SEGSEL, %edx
	movl %edx, %ss # data segment
	movl %edx, %ds # data segment
	movl %edx, %es # data segment
	movl %edx, %fs # data segment
	movl %edx, %gs # data segment
	lidt ivtreg # load the real-mode Interrupt Vector Table
	movl %cr0, %eax
	andl $PROT_DISABLE, %eax # disable segmentation
	movl %eax, %cr0
	ljmp $0, $end_go16
end_go16:
	xorw %ax, %ax # reset segment registers
	movw %ax, %ss
	movw %ax, %ds
	movw %ax, %es
	movw $STACK, %sp # load new stack pointer
	movb int_or_excpn, %al # load variable
	testb %al, %al # is it an interrupt?
	jnz 2f # yes
end_go16.1:
	cmpb $1, reb_mac
	je int_19
	jmp int16_excpns # it is an interrupt
2:
end_go16.2:
	jmp int16_ints # it is an interrupt
int_19:
	movw $0x1234, 0x472
	ljmp $0xffff, $0
.code32
reentry_eip:
	nop # just in case...
	movl $SDATA_SEGSEL, %eax # protected-mode segments
	movl %eax, %ss
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %gs
	movl %eax, %fs
	movl $MEM_TSS, %edi # point to TSS
	movl 56(%edi), %esp # restore stack pointer
	movl 32(%edi), %eax # load page directory address
	movl %eax, %cr3 # restore page directory address
	movb page_on, %al
	testb %al, %al
	jz no_pag1
	movl %cr0, %eax # load register
	orl $PG_ENABLE, %eax # PG on
	movl %eax, %cr0 # restore paging
no_pag1:
	ret

.data
key_scanco:
	.long 0x00000000
key_asciico:
	.long 0x00000000
kargs_size:
	.long 0x00000000
ptr_str_term:
	.long 0x00000000
pnp_args_size:
	.long 0x00000000
pnp_func:
	.long 0x00000000
pnp_ptrs:
	.long 0x00000000, 0x00000000
pnp_bufs:
	.long 0x00000000
pnp_bufs_off:
	.long 0x00000000
pnp_cur:
	.long 0x00000000
pnp_32:
	.long 0, 0, 0, 0, 0
pnp_16:
	.long 0, 0, 0, 0, 0
pnp_num:
	.long 0x00000000
pnp_off:
	.long 0
pnp_seg:
	.long 0
pnp_tmp:
	.long 0
pnp_cont:
	.long 0
pnp_args:
	.long 0, 0, 0, 0, 0, 0, 0, 0
