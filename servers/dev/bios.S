.code32
.text
.globl low_putstr
low_putstr:
	int $0x10
	ret
