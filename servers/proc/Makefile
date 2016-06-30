second:
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/kstdio/kprintf.c -o kprintf.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/pools.c -o pools.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/receive.c -o receive.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/string.c -o string.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.c -o dump32.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.S -o bin2bcd.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c bios.S -o bios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c init.c -o init.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c proc.c -o proc.o
	cc -m32 -fno-strict-aliasing -ffreestanding -N -e init -Ttext=0x00400080 -nostdlib -o proc_serv init.o string.o dump32.o pools.o receive.o proc.o bios.o kprintf.o bin2bcd.o
clean:
	rm proc_serv *.o
-Ttext=0xc0476740
