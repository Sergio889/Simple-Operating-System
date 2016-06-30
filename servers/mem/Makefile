second:
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/kstdio/kprintf.c -o kprintf.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/receive.c -o receive.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/string.c -o string.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.c -o dump32.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.S -o bin2bcd.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c bios.S -o bios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c init.c -o init.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c vm.c -o vm.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c alloc.c -o alloc.o
	cc -m32 -fno-strict-aliasing -ffreestanding -N -e init -Ttext=0x00800080 -nostdlib -o vm_serv init.o string.o dump32.o receive.o alloc.o vm.o bios.o kprintf.o bin2bcd.o
clean:
	rm vm_serv *.o
-Ttext=0xc0476740
