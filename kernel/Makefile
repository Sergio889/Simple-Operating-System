second:
	cc -m32 -fno-strict-aliasing -ffreestanding -c i386/mmap.c -o i386/mmap.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c i386/start.S -o i386/start.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c i386/bios.S -o i386/bios.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/receive.c -o ../libcommon/receive.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/dump32.c -o ../libcommon/dump32.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/dump32.S -o ../libcommon/bin2bcd.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/string.c -o ../libcommon/string.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/elf32.c -o ../libcommon/elf32.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/SOSfs.c -o ../libcommon/SOSfs.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c ../libcommon/kstdio/kprintf.c -o ../libcommon/kstdio/kprintf.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c SOS_init.c -o SOS_init.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c main.c -o main.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c reqs.c -o reqs.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c reply.c -o reply.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c send.c -o send.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c vm.c -o vm.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c alloc.c -o alloc.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c proc.c -o proc.o
	cc -m32 -fno-strict-aliasing -ffreestanding -c i386/init.c -o i386/init.o
	cc -m32 -fno-strict-aliasing -ffreestanding -N -e start -Ttext=0x00200080 -nostdlib -o kernel i386/start.o i386/bios.o i386/mmap.o ../libcommon/kstdio/kprintf.o ../libcommon/string.o ../libcommon/elf32.o ../libcommon/SOSfs.o ../libcommon/receive.o ../libcommon/dump32.o ../libcommon/bin2bcd.o reqs.o reply.o send.o vm.o proc.o main.o alloc.o SOS_init.o i386/init.o 
clean:
	rm kernel *.o rm ../libcommon/*.o
-Ttext=0xc0476740
