.code32
.text
.globl low_putstr
.type	low_putstr, @function
low_putstr:
	int $0x10
	ret
