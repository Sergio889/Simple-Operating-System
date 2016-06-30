.set PCI_ADDRESS, 0xcf8
.set PCI_DATA, 0xcfc

.globl low_pci_conf_rd, low_pci_conf_wr, low_pnp
.type	low_pci_conf_rd, @function
.type low_pci_conf_wr, @function
.type low_pnp, @function
.code32
.text
low_pnp:
	int $22
	ret

low_pci_conf_rd:
	pushl %ebp
	movl %esp, %ebp
	movl 8(%ebp), %eax # configuration-read word
	movl $PCI_ADDRESS, %edx	# io address
	outl %eax, %dx # write word
	movl $PCI_DATA, %edx	# io address
	inl %dx, %eax # read register
	popl %ebp
	ret

low_pci_conf_wr:
	pushl %ebp
	movl %esp, %ebp
	movl 8(%ebp), %eax # configuration-read word
	movl $PCI_ADDRESS, %edx	# io address
	outl %eax, %dx # write word
	movl $PCI_DATA, %edx	# io address
	movl 12(%ebp), %eax
	outl %eax, %dx # write register
	popl %ebp
	ret
