second:
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/kstdio/kprintf.c -o kprintf.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/pools.c -o pools.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/receive.c -o receive.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/string.c -o string.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.c -o dump32.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.S -o bin2bcd.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c bios.S -o bios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c init.c -o init.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c fs.c -o fs.o
	cc -m32 -fno-strict-aliasing -ffreestanding -N -e init -Ttext=0x00c00080 -nostdlib -o fs_serv init.o string.o dump32.o receive.o pools.o fs.o bios.o kprintf.o bin2bcd.o
clean:
	rm fs_serv *.o
-Ttext=0xc0476740
