second:
	cc -m32 -fno-strict-aliasing -ffreestanding -c second.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c support.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c term.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c 6845.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c 8042.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c SOSfs.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c themon.S
	cc -m32 -fno-strict-aliasing -ffreestanding -c term.c -o term_high.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c interp.c -o interp.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c heap.c -o heap.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c parse.c -o parse.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ddb/dbg.c -o ddb/dbg.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ddb/dbg_disasm.c -o ddb/dbg_disasm.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ddb/dbg_coms.c -o ddb/dbg_coms.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ddb/dbg_break.S -o ddb/dbg_break.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../../libcommon/dump32.c
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../../libcommon/kstdio/kprintf.c
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../../libcommon/dump32.S -o bin2bcd.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../../libcommon/string.c
	cc -m32 -fno-strict-aliasing -ffreestanding -N -e entry -Ttext=0x9074 -nostdlib -Wl,-S -o second second.o SOSfs.o themon.o support.o 6845.o 8042.o heap.o term.o dump32.o string.o ddb/dbg.o ddb/dbg_disasm.o ddb/dbg_coms.o ddb/dbg_break.o term_high.o parse.o interp.o bin2bcd.o kprintf.o
install:
	gcc -m32 -g install.c -o install
clean:
	rm install second *.o
