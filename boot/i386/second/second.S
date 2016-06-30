/*
 * Este puede ser pensado como un programa monitor que maneja todos los
 * aspectos de la inicialización de bajo nivel de los sistemas IBM PC/AT y
 * compatibles.
 * La mayoría del programa está codificado en C, aunque el principio y
 * algunas rutinas de soporte están codificadas en ensamblador.
 * El programa es compilado y enlazado con el formato objeto ELF
 * It does the following (in order):
 * - Save important variables passed by bootloader
 * - Place drivers for keyboard and invalid opcode exception
 * - Probe BIOS for relevant information and display them through BIOS 0x10
 * - Kindly ask the user to "kill BIOS" by pressing a key
 * - Place driver for displaying to the screen
 * - Start the command-interpreter and terminal handling logic
 * - Make sure not running on ancient hardware
 * - Call TheMon (Daemon) for further initialization (programming 8259, etc)
 * - Call procedure to load kernel through file-system
 * - Create data structures for real and protected mode
 * - Start virtual-memory system mapping low-memory and kernel accordingly
 * - Jump to the kernel
 * Arguments expected:
 * ds:si = pointer to partition table entry
 * dl = drive number
 * stack:
 * -2 number of blocks before super block
 *  0 if BIOS supports for LBA
 * 
 * There are a few conventions taken from FreeBSD relative to memory addresses.
 * No actual code taken.
 */

/*
 * Definiciones de control.
 * Esto sólo es usado en probe_bios().
 */
.set EOI, -1 # marca de fin de interrupciones
.set EOF, -2 # marca de fin de funciones
.set EOP, -3 # marca de fin de parámetros
.set IO_BASE_OFF, 0x63 # dirección base en entrada/salida para la tabla de la pantalla
.set KERN_ENT_OFF, 128 # desplazamiento hacia el inicio del kernel (necesario para sobrepasar el ELF)

/*
 * Las direcciones de memoria escogidas son iguales a las escogidas en FreeBSD.
 */
.set LOADADDR, 0x9000 # $LOADADDR:$0 dirección donde reside este programa
.set MEM_TSS, 0x5f98 # $MEM_TSS:$0 inicio del segmento de tarea
.set MEM_TSS_END, 0x7fff # $MEM_TSS:$0 fin de segmento de tarea

/*
 * Segmentos para modo protegido.
 */
.set SCODE_SEGSEL, 0x8 # segmento para código de supervisor
.set SDATA_SEGSEL, 0x10 # segmento para datos y pila de supervisor
.set RCODE_SEGSEL, 0x18 # segmento para código de 16 bits
.set RDATA_SEGSEL, 0x20 # segmento para datos y pila de 16 bits
.set UCODE_SEGSEL, 0x28|3 # segmento para código de usuario
.set UDATA_SEGSEL, 0x30|3 # segmento para datos y pila de usuario
.set TSS_SEGSEL, 0x38 # segmento para tarea

/*
 * Objetos visibles desde otros archivos enlazados.
 */
.globl entry, int_nums, function_nums, params_nums, bios_death, one_char_str, shift, regs_strs16, test2, test2_end, temp_str, taking_str, taking_str_end, crl, columns, display_mode, page_number, minor_version, major_version, extended_memory, max_head, max_sector, max_sector, max_track, num_drives, hours, minutes, seconds, num_columns, num_rows, mode, io_offsets, base_io, test1, part_table_ptr, drive_num, gdtreg, error
.code16
.text
entry:
	jmp start # saltar estructuras de datos

.data
/* Variable para guardar la dirección de la tabla de particiones */
part_table_ptr:
	.word 0x0000
welcome_SOS:
	.asciz "Welcome to the Simple Operating System (SOS)\r\n\r\n"
/* Cadena para cuando instalamos un manejador en el vector de interrupciones */
taking_str:
	.asciz "Took over interrupt % \r\n"
taking_str_end:

/* Cadena para cuando saquemos información de la BIOS*/
from_bios_str:
	.asciz "From BIOS:\r\n"
/* Cadena para cuando saquemos información de la BIOS*/
bios_str:
	.asciz "0x%% -> "
bios_str_end:

/* Cadena para cuando mostremos información del sistema */
first_str:
extended_mem_size_str:
	.asciz "times 1K extended memory\r\nHere is geometry if you need translation:\r\n"
max_track_str:
	.asciz "maximum track number\r\n"
max_sector_str:
	.asciz "maximum sector number\r\n"
max_head_str:
	.asciz "maximum head number\r\n"
num_drives_str:
	.asciz "number of drives\r\nTime is:\r\n"
hours_str:
	.asciz "hour\r\n"
minutes_str:
	.asciz "minutes\r\n"
seconds_str:
	.asciz "seconds\r\n"
base_io_str:
	.asciz "base of IO ports for video\r\n"
/* Cadena para cuando presentemos al usuario generosamente la posibilidad de matar a la BIOS */
kill_bios_str:
	.asciz "Hit enter to kill BIOS!\r\n"

/* Vector de números de interrupciones que usaremos para sacar información de la BIOS*/
int_nums:
	.byte 0x15, 0x13, 0x1a, 0x1d, EOI

/* Vector que describe las funciones que utiliza cada interrupción de la BIOS */
function_nums:
	.byte 0x88, EOF, 0x08, EOF, 0x02, EOF, 0x00, EOF

/* Vector que describe los parámetros que se debe pasar a las funciones */
params_nums:
	.byte EOP, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, EOP, EOP, EOP

/* Cadena para cuando imprimamos los registros de la CPU */
regs_strs16:
	.asciz "di=0x", " si=0x", " bp=0x", " sp=0x", " bx=0x", " dx=0x", " cx=0x", " ax=0x"

/* Variable booleana para decidir sobre desplazamiento de bits */
shift:
	.byte 0x01

/* Cadena de un solo caracter */
one_char_str:
	.word 0x0000

/* Inicio absoluto de la partición en sectores */
lba_first:
	.word 0x0000, 0x0000

/* Desplazamiento hacia el superbloque desde el inicio de la partición, en sectores */
sb_lba_off:
	.word 0x0000, 0x0000

/* Variable booleana que determina soporte para LBA */
lba_support:
	.byte 0x00

/* Variable para guardar el número de disco de la BIOS */
drive_num:
	.byte 0x00

/* Búfer para guardar el superbloque de la partición */
super_block:
	.fill 0x200, 0x1, 0x0

/* Variable booleana para determinar si tenemos un microprocesador demasiado viejo */
is_8088:
	.byte 0x00

/* Cadena de fin de línea */
crl:
	.asciz "\r\n"
io_offsets:
	.byte 0x4a, 0x63, 0x84, 0x85, 0x9a

/* Búfer para guardar temporalmente una cadena */
temp_str:
	.fill 256, 1, 0
variables_from_bios:
memory:
	extended_memory:
		.word 0x0000
disk:
	max_track:
		.word 0x0000
	max_sector:
		.word 0x0000
	max_head:
		.word 0x0000
	num_drives:
		.word 0x0000
time:
	hours:
		.word 0x0000
	minutes:
		.word 0x0000
	seconds:
		.word 0x0000
video_info:
	num_columns:
		.word 0x0000
	base_io:
		.word 0x0000
	num_rows:
		.word 0x0000
	fonts:
		.word 0x0000
	mode:
		.word 0x0000
	
.text
start:
	popw %ax # restaurar variable booleana para soporte LBA
	movb %al, lba_support # guardarla
	popw %ax # restaurar desplazamiento hacia el superbloque
	movw %ax, sb_lba_off # guardarlo
	popw %ax # restaurar apuntador a la tabla de particiones
	movw %ax, part_table_ptr # guardarlo
	movb %dl, drive_num # guardar número de disco de la BIOS

	movw $welcome_SOS, %si # %si apunta a cadena de bienvenida al sistema operativo
	call putstr # imprimirla
	movw $0x9, %si # instalar manejador para interrupción de teclado
	movw $new_int09, %ax # dirección del nuevo manejador
	movw $old_int09, %di # dirección del antiguo manejador
	call hook_intX # instalar manejador
	movw $0x6, %si # instalar manejador para excepción de instrucción inválida
	movw $new_int06, %ax # dirección del nuevo manejador
	movw $old_int06, %di # dirección del antiguo manejador
	call hook_intX # instalar manejador
	call probe_bios # tomar lo que podamos de la BIOS
	movw $crl, %si # %si apunta a cadena de fin de línea
	call putstr # 'imprimirla'
	movw $from_bios_str, %si # %si apunta a cadena de BIOS
	call putstr # imprimirla
	movw $crl, %si # imprimir
	call putstr #  cadena de fin de línea
	movw $8, %cx # cantidad de iteraciones para impresión de cadenas
	movw $variables_from_bios, %si # %si apunta a variables a imprimir
	movw $bios_str, %di # %di apunta a las cadenas a imprimir
	movw $bios_str_end, %bx # %bx apunta a fin de cadenas
	movw $first_str, %bp # %bp apunta a cadenas a imprimir
	subw %di, %bx # restar para obtener tamaño de la cadena
loop_from_bios:
	movw $2, %ax # números a imprimir
	pushw %bx # guardar tamaño de cadena
	pushw %si # guardar apuntador a variables a imprimir
	pushw %di # guardar apuntador a cadenas a imprimir
	pushw %cx # guardar variable de iteración
	pushw %bp # guardar apuntador a la primera cadena
	movw (%si), %dx # extraer dos números
	movb %dl, %cl # extraer byte bajo
	andw $0x00ff, %cx # eliminar bits de órdenes altos
	pushw %cx # push it for argument
	movb %dh, %cl # extract high byte
	andw $0x00ff, %cx # mask low order bits
	pushw $'h' # push argument
	pushw %cx # push argument
	pushw $'h' # push argument
	pushw %di # string pointer
	pushw %bx # string lenght
	call low_printf # print string
	addw $12, %sp # bypass stack info we don't need
	popw %bp # restore pointer to first string
	movw %bp, %si # save it to print
	call putstr # print it
	movw %si, %bp # save
	popw %cx # restore loop number
	popw %di # restore pointer to strings to print
	popw %si # restore pointer to variables to print
	popw %bx # restore string lenght
	addw $2, %si # to next entry
	loop loop_from_bios # loop
	movw $kill_bios_str, %si # copy string
	call putstr # print it
	xorb %al, %al # zero where key is read
	call read_key # read key
	call read_key # read key
	movb $1, no_bios # stop using BIOS interrupt handler for keyboard
	movw $0x10, %si # lets take care of the display
	movw $new_int10, %ax # new handler's address
	movw $old_int10, %di # old handler's backup address
	call hook_intX # take over interrupt
	int $0x10
/*
 * Discard running on proc <= 80486 (NO CPUID OP)
 * Interrupt Vector Table created by BIOS starts at
 * $0:$0. Every entry in the table consists of a valid
 * 16-bit real-mode address, that is, an address formed
 * by a base address, which comes from a half word 
 * segment register shifted 4 bits to the left, and a 
 * half word offset from that base address.
 * Because the base address and offset values are 1 word wide,
 * each entry is 4 bytes, and because an invalid opcode generates
 * an exception with a number of 6, meaning the 6th entry in
 * the table, we place at 0:24 our own address for handling the
 * exception. The idea is both to detect ancient hardware and
 * not halt the CPU because of an invalid opcode.
 */
cpu_id:
	cpuid
return_handler:
	movb is_8088, %al # anyway, this will be executed
	testb %al, %al # zero if Pentium or greater
	jnz error
/*
 * An early necessary operation before switching to protected-mode is
 * reprogramming the 8259 interrupt controller. The fundamental problem is
 * that Intel reserves some interrupt vectors in protected-mode for internal
 * or future use. The BIOS creates the Interrupt Vector Table ignoring this
 * issue and, therefore, using reserved vectors. For this reason, the vectors
 * used by the BIOS but that are reserved by Intel must be relocated.
 */
init.1:
/*
 * GO 32-bits!.
 * Load necessary registers for segmentation.
 * Before switching to protected-mode we call load_kernel to load the file
 * at /boot/kernel to a known address.
 */
bits32:
	cli # this part of the monitor needs no interrupts
	call init_themon # call the monitor themon <-> daemon
	call load_kernel # load a program called kernel
	lgdt gdtreg # load GDTR
	lidt idtreg # load IDTR
	movl %cr0, %eax # go
	orl $1, %eax #  protected mode
	movl %eax, %cr0 #  32-bits!
	ljmp $SCODE_SEGSEL, $init.2

 error:
	pushw $0xb800
	popw %es
	movw $0, %di
	movb $'a', %es:(%di)
	jmp .
.code32
/*
 * Continue execution here in 32-bits protected mode!
 * Load the task register with TSS before launching kernel.
 * Still no interrupts.
 */
init.2:
/*
 * Start mapping virtual memory to physical memory.
 * Before jumping to the kernel just loaded, map the low memory (1MB) to
 * the same physical memory and map the kernel to physical address 0x100000.
 */
	movl $SDATA_SEGSEL, %eax
	movl %eax, %ss # new segment
	movl %eax, %es # new segment
	movl %eax, %ds # new segment
	movl %eax, %fs # new segment
	movl %eax, %gs # new segment
	cli
	call heap_init
	call term_init
	call interpreter_init
	ljmp $SCODE_SEGSEL, $(0x00200000+KERN_ENT_OFF)

/*
 * If running on a 80486 or less, exception generated by cpuid
 * instruction leads here, where we set a variable indicating
 * that perhaps we are running a 16-bit processor.
 * Although invalid opcode exception was introduced in the 80386.
 */
new_int06:
	movb $1, is_8088 # surely we have a proc < Pentium
	popw %ax # restore offset
	popw %bx # restore segment
	pushw $0 # save new segment
	pushw $return_handler # save new offset
	iret # return to instruction following cpuid

bios_death:
	jmp .


.data
.p2align 3 # force 8-byte alignment
gdt:
	.word 0x0, 0x0, 0x0, 0x0 # first entry must be NULL
	.word 0xffff, 0x0000, 0x9a00, 0x00cf # supervisor code
	.word 0xffff, 0x0000, 0x9200, 0x00cf # supervisor data/stack
	.word 0xffff, 0x0000, 0x9a00, 0x0000 # real-mode code segment
	.word 0xffff, 0x0000, 0x9200, 0x0000 # real-mode data segment
	.word 0xffff, 0xa000, 0xfa00, 0x00cf # user code
	.word 0xffff, 0xa000, 0xf200, 0x00cf # user data/stack
	.word (MEM_TSS_END-MEM_TSS), MEM_TSS, 0x8900, 0x0000 # obligated task segment
gdt.1:

gdtreg:
	.word gdt.1-gdt-1, gdt, 0x00
old_int09:
	.word 0x0000, 0x0000
old_int06:
	.word 0x0000, 0x0000
old_int10:
	.word 0x0000, 0x0000
old_int13:
	.word 0x0000, 0x0000
cr0_buf:
	.fill 32, 1, 0
