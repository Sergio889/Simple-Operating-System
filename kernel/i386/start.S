/*
 * Finally we reach the first part of the kernel.
 * The kernel is, to great extent, the operating system itself, although
 * it is modernly arguable.
 * There is a fundamental reason why, to this point, code has been written in
 * assembly language, a reason somewhat more subtle than machine independence.
 * When studying operating systems, generally assembly language code is 
 * minimized in favor of the most machine independent code. We are delaying
 * the switch to a high-level language to ease understanding of the initial
 * steps at boot time. NOT SUGESTING ASSEMBLY LANGUAGE IS EASIER THAN THE C
 * PROGRAMMING LANGUAGE, CHANCES ARE THE CONTRARY HOLDS, JUST THAT SOMETIMES 
 * IT IS MORE DIFFICULT TO UNDERSTAND HOW BOTH LANGUAGES CAN WORK TOGETHER,
 * THUS IT MAY BE ADDEQUATE TO PROPOSE THAT LATER.
 * It is highly recommended to ultimately (that is, after studying a simple one)
 * study the Multics Operating System (Organick, 1972) for a simple reason 
 * (not as simple as Unix was).
 * Unix has historically been worshiped due to its simplicity. This was
 * acceptable maybe until the late 70's. Naturally computer systems are far
 * from those time-shared mini-computers from the 70's, and Unix' simplicity
 * is now inaddecuate. The evolution of Unix, from AT&T's System V Release 4 to
 * 4.4BSD, back to the late 80's and early 90's ended up simulating in some
 * form what Multics had attempted (and undoubtly achived) decades ago, with
 * each possible operation being made available to each process' address space.
 * The IO memory-mapped interface first proposed for 4BSD virtual memory
 * system and later designed and implemented in Mach 2.0, Mach 2.5 
 * and Mach 3.0, that was latter implemented to 4.4BSD, is fundamentally
 * identical to the segmentation approach taken by Multics from which it was
 * possible to map almost everything to each process' virtual memory address
 * space, and operations on those segments were issued by normal memory
 * write/read. Later operating systems, such like FreeBSD and Linux, continued 
 * with the philosophy of memory-mapping and a 'complex' virtual memory
 * system.
 * The previous background probably would encourage Einstein's famous quote:
 * "Everything should be made as simple as possible, but not simpler".
 * Naturally we are not suggesting Unix was wrong, in fact, we encourage the
 * simplicity philosophy. We just want to point how modern Unix systems
 * approach to a sublime Multics system that, just perhaps, was to ambitious
 * for its time.
/* 
 * The first part of SOS's kernel is concerned with mapping its own image to
 * the page-related structures necessary for starting the virtual memory
 * system. The structures will allow us to map the kernel at every per-process
 * page table. Until then (the necessary page-related structures are built
 * and right afterwards paging enabled), kernel memory references are wrong,
 * because we loaded it to address 0x100000, but was linked to address
 * VIRT_ADDR. So, before mapping to the virtual memory system, each memory
 * reference must be previously calculated.
 */
.set PG_ENABLE, 0x80000000 # enable paging
.set CR3_VAL, 0x01400000 # address of page-directory structure
.set MEM_TSS, 0x5f98 # $MEM_TSS:$0 beginning of task segment
.set MEM_TSS_END, 0x7fff # $MEM_TSS:$0 end of task segment
.set MEM_MAP, 0x6000
.set TSS_SS0, 0x8 # SS0 of TSS
.set TSS_ESP0, 0x4 # ESP0 of TSS
.set TSS_SS2, 0x24 # SS0 of TSS
.set TSS_ESP2, 0x20 # ESP0 of TSS
.set KERN_STACK, 0x400000

/*
 * When in protected mode, i386 processors interpret segment registers as
 * segment selector index values inside the Global Descriptor Table (GDT).
 * The address of the GDT is stored in the Global Descriptor 
 * Table Register (GDTR).
 * When in real mode, any x86 processor directly maps segment registers to
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

.code32
.globl start, map_kernel, enable_paging, test_copy
.text
start:
/*
 * Start mapping virtual memory to physical memory.
 */
	cld # up we go for string ops
	movl $KERN_STACK, %esp # load high stack
	call init # go C to initialize user-space servers
	jmp .

test_copy:
	movl $0xb8000, %esi
	movl $256, %ecx
1:
	movb $'a', (%esi)
	addl $2, %esi
	loop 1b
	jmp .

enable_paging:
	cli
	movl $CR3_VAL, %eax
	movl %eax, %cr3 # save it
	movl %cr0, %eax # load %cr0
	orl $PG_ENABLE, %eax # PG bit on
	movl %eax, %cr0 # enable paging!
	sti # enable interrupts
	ret

.data
mon_pag:
	.long 0x0000000b, 0x00000000 # first page (0x0)
map_kernel_ctrl_str:
kern_args_ptr:
	.long 0x00000000
