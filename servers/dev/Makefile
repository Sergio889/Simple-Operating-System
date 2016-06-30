second:
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/receive.c -o receive.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/pools.c -o pools.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/string.c -o string.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.c -o dump32.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/dump32.S -o bin2bcd.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../../libcommon/kstdio/kprintf.c -o kprintf.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c low.S -o low.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c bios.S -o bios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c i386/cpuid.S -o cpuid.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c init.c -o init.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c pci.c -o pci.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c pnpbios.c -o pnpbios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c smbios.c -o smbios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c dev.c -o dev.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c irq.c -o irq.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c 8259.c -o 8259.o
	cc -m32 -fno-strict-aliasing -ffreestanding -N -e init -Ttext=0x01000080 -nostdlib -o dev_serv init.o string.o dump32.o receive.o pools.o dev.o bios.o cpuid.o low.o 8259.o irq.o smbios.o pnpbios.o pci.o kprintf.o bin2bcd.o
clean:
	rm dev_serv *.o
-Ttext=0xc0476740
