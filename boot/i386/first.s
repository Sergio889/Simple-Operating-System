/*
 * Este programa es cargado por el MBR y luego el control de ejecución es
 * pasado a él.
 * Su función es simplemente cargar un programa aún más grande del disco.
 * Los argumentos esperados son:
 * %dl == número de disco de la BIOS
 * %es:%si == apuntador a la entrada correcta en la tabla de particiones
 * Programa de dos pasadas:
 * 1. Cargar el primer sector del siguiente programa y determinar su tamaño.
 *  (el siguiente programa tiene formato ELF).
 * 2. Teniendo el tamaño total en bytes, cargar la cantidad de sectores que
 *  ocupa el siguiente programa.
 */
/*dirección donde este programa reside*/
.set LOADSTART, 0x7c00
/* dirección donde cargar el siguiente programa */
.set NEXTLOAD, 0x9000
/* tamaño en bytes del resto de campos en el encabezado ELF */
.set ELF_RES_STR_LEN, 8 # 
/*desplazamiento hacia tamaño de sección/segmento en la entrada en la tabla*/
.set ELF_SEGMENT_SIZE_OFF, 16
/*desplazamiento hacia tamaño de sección/segmento en la entrada en la tabla*/
.set ELF_SECTION_SIZE_OFF, 20
/*tamaño en bytes del identificador en el encabezado ELF*/
.set ELF_ID_STR_LEN, 16
/* 
 * Desplazamiento hacia la dirección del punto de entrada del siguiente 
 * programa.
 */
.set ELF_ENTRY_POINT, 24
/* desplazamiento hacia el encabezado de la tabla de programa */
.set ELF_PH_OFF, 28
/* desplazamiento hacia el encabezado de la tabla de sección */
.set ELF_SH_OFF, 32
/* desplazamiento hacia el tamaño del encabezado ELF */
.set ELF_ELF_SIZ_OFF, 40
/* desplazamiento hacia el tamaño de cada encabezado de programa */
.set ELF_PH_ENT_SIZ_OFF, 42
/* desplazamiento hacia el número de entradas en el encabezado de programa */
.set ELF_PH_ENT_NUM, 44
/* desplazamiento hacia el tamaño de cada encabezado de sección */
.set ELF_SH_ENT_SIZ_OFF, 46
/* desplazamiento hacia el número de entradas en el encabezado de sección */
.set ELF_SH_ENT_NUM, 48

.globl entry
.code16
entry:
	cli # deshabilitar interrupciones mientras pila inestable
	cld # instrucciones que involucran arreglos aumentan %si y %di
	xorw %ax, %ax # cero
	movw %ax, %ds # inicializar (eliminar) segmentación; segmento de datos
	movw %ax, %ss # segmento de pila
	movw $LOADSTART-1, %sp # pila inicia justo debajo de este programa
	sti # pila estable; habilitar interrupciones
	jmp main # saltar las estructuras de datos que siguen
/* espacio para guardar entrada de la partición pasada por MBR */
ptent:
	.fill 0x10, 0x1, 0x0
/* 
 * Variable para guardar la cantidad de sectores que conforman el siguiente 
 * programa.
 */
loader_size:
	.word 0x0001
/* espacio para guardar número de disco de la BIOS */
drive_num:
	.byte 0x0
/* booleano estático para guardar soporte EDD */
lba_support:
	.byte 0x00
/* cadena de error */
error_str:
	.asciz "First"
/*
 * Bytes que definen el inicio encabezado ELF: 32-bits, little-endian, versión actual, osabi, y
 * versión abi. 9 bytes en total.
 */
e_ident:
	.byte 0x7f
	.ascii "ELF"
	.byte 0x01
	.byte 0x01
	.byte 0x01
	.byte 0x09
	.fill 8, 1, 0
/*
 * Bytes que definen el tipo mágico, máquina y versión
 * 8 bytes en total.
 */
e_rest:
	.word 0x0002
	.word 0x0003
	.long 0x00000001

main:
	movb %dl, drive_num # guardar el número de disco de la BIOS en memoria
	movw $ptent, %di # copiar entrada en la tabla de partición desde %es:%si
	movw $0x10, %cx # tamaño de la entrada en la tabla de partición
	rep
	movsb
	movb $1, lba_support # guardar soporte para LBA en memoria
	call read_blk # leer del disco hacia RAM 512 bytes del siguiente programa
	movw $NEXTLOAD, %bp # %bp apunta al inicio del siguiente programa
	movw %bp, %di # copiar apuntador a %di
	movw $e_ident, %si # %si apunta a e_ident
	movw $ELF_ID_STR_LEN+1, %cx # número de bytes que debemos verificar
	call parse_elf # verificar si formato ELF está correcto
	leaw 16(%bp), %di # copiar apuntador a %di
	movw $e_rest, %si # %si apunta a e_rest
	movw $ELF_RES_STR_LEN+1, %cx # número de bytes a verificar
	call parse_elf # verificar si formato ELF está correcto
	call parse_elf_min_blocks # cantidad de sectores que debemos cargar
	xorw %dx, %dx # usar %dx en la función parse_elf_size
	call parse_elf_size # extraer bytes totales del siguiente programa
	call read_blk
	movw $ptent, %si # apuntar a la entrada en la tabla de particiones
	movb drive_num, %dl # cargar número de disco de BIOS
	movw loader_size, %ax # cargar cantidad de sectores antes del superbloque
	incw %ax # aumentar en uno
	pushw %ax # guardarlo en la pila; pasará al siguiente programa
	pushw lba_support # guardar soporte para LBA en la pila
	pushw %si # guardar apuntador a entrada en tabla de particiones
	ljmp $0, $0x9074 # saltar al siguiente programa

/*
 * ELF object file-format can be identified from the ELF header, located
 * at the beginning of the file.
 * This function parses the first bytes that identifies the file as ELF, and
 * the next bytes that identifies the file as being compiled in little endian
 * format.
 */
parse_elf:
	repe
	cmpsb
	testb %cl, %cl # verificar resultado
	jz ok1 # será cero si está todo bien
	call error_elf # encabezado incorrecto

/*
 * From the standard structures from the ELF file format extract the total
 * size of the file in bytes.
 */
parse_elf_size:
	movw $NEXTLOAD, %bp # %bp apunta al principio del siguiente programa
	movw ELF_PH_ENT_SIZ_OFF(%bp), %bx # extraer variable a %bx
	movw ELF_PH_ENT_NUM(%bp), %cx # cargar variable a %cx
	movw %bp, %di
	addw ELF_PH_OFF(%bp), %di # extraer variable a %ax
	addw ELF_SEGMENT_SIZE_OFF(%bp), %di # %di apunta a inicio de la tabla
1:
	addw (%di), %dx # aumentar tamaño total por la variable de tamaño
	addw %bx, %di # siguiente entrada
	loop 1b # iterar
	movw ELF_SH_ENT_SIZ_OFF(%bp), %bx # extraer variable a %bx
	movw ELF_SH_ENT_NUM(%bp), %cx # cargar variable a %cx
	movw %bp, %di
	addw ELF_SH_OFF(%bp), %di # extraer variable a %ax
	addw ELF_SECTION_SIZE_OFF(%bp), %di # %di apunta a inicio de la tabla
2:
	addw (%di), %dx # aumentar tamaño total por la variable de tamaño
	addw %bx, %di # siguiente entrada
	loop 2b # iterar
	addw ELF_ELF_SIZ_OFF(%bp), %dx # agregar tamaño de encabezado ELF
	shrw $9, %dx
	incw %dx
	movw %dx, loader_size # total bytes / 512 = number of sectors to load
	ret # retornar a la función que llamó

/*
 * Extract from the ELF header the greater offset between the offset to the
 * section header and the offset to the segment header.
 */
parse_elf_min_blocks:
	movw ELF_PH_OFF(%bp), %ax # desplazamiento en bytes a tabla de programa
	movw ELF_SH_OFF(%bp), %bx # desplazamiento en bytes a tabla de sección
	cmp %ax, %bx # qué desplazamiento es mayor?
	jb 1f # tabla de programa
	movw %bx, %ax # tabla de sección
1:
	shrw $9, %ax
	incw %ax # load one more sector
	movw %ax, loader_size

read_blk:
	movw $ptent, %bp # %bp apunta a ptent
	movw $NEXTLOAD, %bx # sector leído del disco se copiará a NEXTLOAD
	movl 8(%bp), %eax # primer bloque de la partición (LBA)
	movl 12(%bp), %edx # primer bloque de la partición (LBA)
	incl %eax # incrementar LBA al segundo sector de la partición
	pushw %si # guardar apuntador a entrada correcta en la tabla de particiones
	mov %sp, %di # guardar apuntador de pila
	pushl $0x0 # construír paquete
	pushl %eax
	pushw %es
	pushw %bx
	movw loader_size, %dx
	pushw %dx
	pushw $0x0010
	movw %sp, %si
	movb $0x42, %ah
	movb drive_num, %dl
	int $0x13 # leer sector usando la BIOS
	jc error # ir a error si algo anduvo mal
	movw %di, %sp # restaurar pila
	popw %si # restaurar apuntador a la entrada correcta en la tabla de particiones
	ret # retornar a función previa

ok1:
ok2:
	ret

error:
	movw $error_str, %si
	call putstr
	jmp .

error_elf:
	movw $error_str, %si
	call putstr
	jmp .

/*
 * Prints string of characters by calling bios_int()
 * on each character.
 * Note that printing a single character corresponds
 * to printing a one character string.
 */
/* void putstr(char *) */
putstr:
	lodsb /* load next byte */
	testb %al, %al
	jz 2f
1:
	movb $0xe, %ah
	movb $7, %bh
	int $0x10
	jmp putstr
2:
	ret

/* bytes left */
	.fill 206, 1, 0
