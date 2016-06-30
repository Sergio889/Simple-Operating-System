/*
 * Some simple routines to access the file system.
 */

.set PART_TABLE_OFF, 0x1be
.set PART_TYPE_ENT_OFF, 4
.set SB_IVEC_OFF, 20
.set DIR_ENT_SIZE, 16 # Directory entry in bytes
.set KERNEL_ADDR, 0x2000 # kernel segment address
.set GDT_DEST_OFF, 26 # offset to destination offset

.code16
.globl ls_root, load_kernel, kernel_size
.text
/*
 * It must be copied to /boot/kernel
 */
load_kernel:
	movb $1, ld_kern
ls_root:
	call read_magic # read magic block
	call read_sb # read super block
	movw $empty_buffer, %si # point to super block in main memory
	addw $SB_IVEC_OFF, %si # point to super block size
	movl (%si), %eax # %eax = relative-to-partition sector of ivector blocks
	movl part_sector_abs, %ebx
	addl %ebx, %eax
	movl %eax, ivec_sector_abs # store it
	call read_ivec
	movw $empty_buffer, %si # point to ivector in main memory
	xorl %eax, %eax
	movw (%si), %ax # extract inode's sector of root directory (relative)
	movl ivec_sector_abs, %ebx # absolute sector of first ivector
	addl %ebx, %eax # %eax = absolute sector of root's inode
	movl %eax, root_inode_sector_abs # save it
	incl %eax # absolute sector of root directory's data
	movl %eax, root_data_sector_abs
	call read_ino
	movw $empty_buffer, %si
	movl 4(%si), %eax # extract size of directory's data in sectors
	movl %eax, root_data_size # save it
	movl 16(%si), %eax # extract number of entries in directory
	movl %eax, root_num_entries # save it
	movb ld_kern, %al # test if load kernel
	testb %al, %al
	jnz load_it
	movl root_data_size, %ecx # load number of data sectors (ie. number of loops)
	movl root_data_sector_abs, %eax
	jmp 2f
1:
	incw %ax # read next sector of data
2:
	pushl %ecx # save it
	pushl %eax # save sector to read
	call read_data
	pushw $0xb800
	popw %es
	movw $0, %di
	movb $'a', %es:(%di)
	addw $2, %di
	movb $'a', %es:(%di)
	jmp .
	call parse_data_block
	popl %eax
	popl %ecx
	decl %ecx
	jnz 1b
	ret

/*
 * We will traverse the necessary path through the file system
 * (i.e /boot/kernel) to get the size of the kernel stored in its inode.
 */
load_it:
	movl root_data_sector_abs, %eax # load root's data
	call read_block # read root's data (first sector)
	movw $empty_buffer, %si # pointer to root's first sector of data
	call find_boot # find boot's directory entry in data
	movb found_bool, %al
	testb %al, %al
	#jz error
	movl 12(%si), %eax # save entry number in ivector
	movl %eax, boot_entry_ivec # save it
	movl ivec_sector_abs, %eax # load ivector's first sector
	call read_block # read ivector's first sector
	movw $empty_buffer, %si # pointer to beginning of ivector
	movl $2, %ebx # multiply to get entry number
	movl boot_entry_ivec, %eax # load boot's entry in ivector
	mull %ebx # scale it
	addw %ax, %si # point to boot's entry in ivector
	movw (%si), %ax # save
	movw %ax, boot_inode_sec #  boot's entry in ivector (i.e inode's sector)
	movl ivec_sector_abs, %ebx # load absolute sector to partition
	xorl %eax, %eax # make sure
	movw boot_inode_sec, %ax # load relative sector of boot's inode
	addl %ebx, %eax # absolute sector to boot's inode
	movl %eax, boot_inode_sec_abs # save it
	movl boot_inode_sec_abs, %eax # load it
	incl %eax # absolute sector of boot's data; we know kernel is only entry!
	movl %eax, boot_data_sec_abs # save absolute sector of boot's data
	call read_block # read first sector of boot's data
	movw $empty_buffer, %si # pointer to root's first sector of data
	addw $32, %si # point to kernel entry (XXX must be generalized!)
	movw 12(%si), %ax
	movw %ax, kernel_entry_ivec # save kernel's entry in ivector
	movl ivec_sector_abs, %eax # read ivector
	call read_block
	movw $empty_buffer, %si # pointer to beginning of ivector
	movl $2, %ebx # multiply to get entry number
	movl kernel_entry_ivec, %eax
	mull %ebx
	addw %ax, %si # point to kernel's entry in ivector
	movw (%si), %ax
	movw %ax, kernel_inode_sec
	movl ivec_sector_abs, %ebx
	addl %ebx, %eax
	movl %eax, kernel_inode_sec_abs
	call read_block
	movw $empty_buffer, %si # pointer to kernel inode
	movl 4(%si), %eax # size in sectors of kernel
	movl %eax, kernel_size
	movl kernel_inode_sec_abs, %eax # absolute sector of inode
	incl %eax # absolute sector of data
	movl %eax, kernel_data_sector_abs
	call read_kernel
	ret

/*
 * Print out the name of the entries.
 */
parse_data_block:
	movl root_num_entries, %ecx
	movw $empty_buffer, %si # pointer to entry
	jmp 2f
1:
	addw $DIR_ENT_SIZE, %si # next entry in directory
2:
	decl %ecx
	jnz 1b
	ret

/*
 * Read root's inode.
 */
read_ino:
	movl root_inode_sector_abs, %eax
	call read_block
	ret

/*
 * Read root's data.
 */
read_data:
	call read_block
	ret

/*
 * Read blocks until magic block. The point is to get the offset in sectors.
 */
read_magic:
	call find_entry
	movl (%si), %eax # beginning of partition in sectors
	movl %eax, part_sector_abs
1:
	pushl %eax # save sector
	call read_block
	movw $512, %cx # number of bytes in sector
	call strncmp
	testb %cl, %cl
	jz got_magic
	popl %eax
	incl %eax
	jmp 1b
got_magic:
	popl %eax
	movl %eax, magic_sector_abs
	ret

/*
 * Read the first block of ivector.
 */
read_ivec:
	movl ivec_sector_abs, %eax
	call read_block
	ret

/*
 * Read the super block.
 */
read_sb:
	movl magic_sector_abs, %eax # sector number of magic block
	incl %eax # sector number of super block
	movl %eax, sb_sector_abs
	call read_block
	ret

/*
 * Actually do the reading.
 */
read_block:
	movw %sp, %di # save stack pointer	
	movw $empty_buffer, %bx
	pushl $0 #  construct 
	pushl %eax # LBA address
	pushw %ds #  to
	pushw %bx #  form	
	push $1 #  packet
	push $0x10 #  for
	movw %sp, %si #  LBA
	movb $0x42, %ah #  access	
	movb drive_num, %dl # drive number
	int	$0x13 #  through BIOS
	jc error
	movw %di, %sp # restore stack pointer
	ret

/*
 * This is a special routine to load a kernel of arbitrary size.
 * The strategy is as follows:
 * Because it would be inflexible to constrain the kernel to a fixed size, a
 * strategy must be thought to load a file of arbitrary size. The fundamental
 * problems are addressing in real-mode, BIOS's Interrupt Vector Table and
 * the current lack of control we have on the disk device. Because we have not
 * yet loaded SOS's devices drivers, and because these are stored in the
 * kernel, the BIOS must be trusted the task of reading from disk. BIOS's
 * interrupt routines work only in real-mode, but here are contrained to a 1MB
 * address space, excluding also the space used by the BIOS for storing the
 * IVT, data structures, extensions, etc
 * WE CANNOT LOAD THE KERNEL AT A ADDRESS WITHIN THIS ADDRESS SPACE THAT IS
 * LARGE ENOUGH TO ACCEPT THAT IT CAN BE OF A VIRTUALLY ARBITRARY SIZE.
 * There is, however, an option using the BIOS interrupt 15 for copying to
 * extended memory, that is, memory above 1MB. Thus, we will load the kernel
 * by small units (sectors for now), and copy them to extended memory using
 * the BIOS facility, thus remaining in real-mode.
 */
read_kernel:
	pushw %es
	pushw $0
	popw %es
	movl kernel_size, %ecx # loop kernel's size in sectors
	movl kernel_data_sector_abs, %eax # kernel's first sector
1:
	pushl %ecx # save loop number
	pushl %eax # save sector number
	movw %sp, %di # save stack pointer	
	movw $0, %bx # 0x2000:0
	pushl $0 #  construct 
	pushl %eax # LBA address
	pushw $KERNEL_ADDR #  to
	pushw %bx #  form	
	pushw $1 #  packet
	pushw $0x10 #  for
	movw %sp, %si #  LBA
	movb $0x42, %ah #  access	
	movb drive_num, %dl # drive number
	sti
	int $0x13 #  through BIOS
	jc error
	cli
	movw %di, %sp # restore stack
	movb $0x87, %ah
	movw $0x100, %cx
	movw $ext_mem_gdt, %si	
	sti
	int $0x15
	jc error
	cli
	popl %eax # restore sector number
	incl %eax # read next sector
	movw GDT_DEST_OFF(%si), %bx
	addw $512, %bx
	movw %bx, GDT_DEST_OFF(%si)
	popl %ecx
	loop 1b
	popw %es
	ret

/*
 * Compare block read with magic block.
 */
strncmp:
	pushw %es # save segment
	pushw $0 # load
	popw %es # new segment
	dec %cx
	movw $empty_buffer, %si #  sector read
1:
	movw $'a', %ax
	cmpb (%si), %al
	jne 2f
	dec %cx
	inc %si
	testw %cx, %cx
	jnz 1b
2:
	popw %es # restore segment
	ret

/*
 * Find the sector address of the boot directory inside root's data.
 */
find_boot:
	movl root_num_entries, %ecx # loop
	jmp 2f
1:
	popw %es # restore segment
	popw %si # restore pointer
	addw $16, %si # point to next entry
2:
	pushw %si # save pointer
	pushw %es # save segment
	pushl %ecx # save loop
	pushw $0 # new segment
	popw %es # load it	
	movl $4, %ecx # boot string lenght
	movw $boot_str, %di # compare with
	repe
	scasb
	testl %ecx, %ecx
	jz found_boot
	popl %ecx # restore loop
	loop 1b
	jmp 1f
found_boot:
	movb $1, found_bool
	popl %ecx
1:
	popw %es # restore segment
	popw %si # restore pointer
	ret

/*
 * Find the corresponding entry in the MBR.
 */
find_entry:
	movl $0, %eax # sector number of MBR
	call read_block # read MBR
	movw $empty_buffer, %si # point to beginning of MBR
	addw $PART_TABLE_OFF, %si # point to partition table
	addw $4, %si # point to file system type
	movw $4, %cx # number of entries
	jmp 2f
1:
	addw $16, %si # next entry
2:
	movb (%si), %al # copy the file system entry
	cmpb $0x50, %al # found our file system?
	loopne 1b # no
	addw $4, %si # point to absolute beginning in sectors
	ret

error:
	movw $error_str, %si
	call putstr
	jmp .

error_copy:
	movw $read_str, %si
	call putstr
	jmp .

.data
/* Data in the magic block */
empty_buffer:
	.fill 512, 1, 0
part_number:
	.byte 0x00
part_sector_abs:
	.long 0x00000000
magic_sector_abs:
	.long 0x00000000
sb_sector_abs:
	.long 0x00000000
ivec_sector_abs:
	.long 0x00000000
root_inode_sector_abs:
	.long 0x00000000
root_data_sector_abs:
	.long 0x00000000
root_num_entries:
	.long 0x00000000
root_data_size:
	.long 0x00000000
error_str:
	.asciz "Error"
read_str:
	.asciz "Read"
boot_str:
	.asciz "Boot"
ld_kern:
	.byte 0x00
found_bool:
	.byte 0x00
boot_entry_ivec:
	.long 0x00000000
boot_inode_sec:
	.word 0x0000
boot_inode_sec_abs:
	.long 0x00000000
boot_data_sec_abs:
	.long 0x00000000
kernel_entry_ivec:
	.word 0x0000
kernel_inode_sec:
	.word 0x0000
kernel_inode_sec_abs:
	.long 0x00000000
kernel_size:
	.long 0x00000000
kernel_data_sector_abs:
	.long 0x00000000
ext_mem_gdt:
	.fill 16, 1, 0
	.word 0xffff # source segment lenght
	.byte 0x00, 0x00, 0x02 # source offset
	.byte 0x92 # must be
	.word 0x0000 # must be
	.word 0xffff # destination segment lenght
	.byte 0x00, 0x00, 0x20 # destination offset
	.byte 0x92 # must be
	.word 0x0000
	.fill 16, 1, 0
