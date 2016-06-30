.code32
.globl bios_ext_rd, int_on, int_off, low_memcpy, low_memcpy, low_putstr, low_tot_ram, set_single_step
.text
bios_ext_rd:
	movb $0x42, %ah # extended read function
	int $0x13 # interrupt in protected-mode
	ret # return from interrupt

low_putstr:
	int $0x10
	ret

low_tot_ram:
	xorl %eax, %eax
	xorl %ebx, %ebx
	movl $24, %ecx
	movl $0x534d4150, %edx
	movl $0xe820, %eax
	int $0x15
	ret

low_memcpy:
	int $25
	ret

int_on:
	sti
	ret

int_off:
	cli
	ret

set_single_step:
	pushf
	popl %eax
	orl $0x100, %eax
	pushl %eax
	popf
	ret

.data
temp_buf:
	.fill 1024, 1, 0
