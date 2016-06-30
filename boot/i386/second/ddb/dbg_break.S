/*
 * Funciones de bajo nivel para aplicar puntos de interrupción.
 */
.code32
.text
.globl dbg_break, dbg_unbreak
dbg_break:
	pushl %ebp
	movl %esp, %ebp
	pusha
	movl 8(%ebp), %eax
	movl %eax, %dr0
	movl %dr7, %eax
	orl $1, %eax
	andl $~0x30000, %eax
	movl %eax, %dr7 # %dr7 es inaccesible desde C
	popa
	popl %ebp
	ret

dbg_unbreak:
	pushl %ebp
	movl %esp, %ebp
	pusha
	movl %dr7, %eax
	andl $~1, %eax
	movl %eax, %dr7 # %dr7 es inaccesible desde C
	popa
	popl %ebp
	ret
