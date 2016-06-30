mbr:
	cc -fno-strict-aliasing -pipe  -ffreestanding -mpreferred-stack-boundary=2  -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3  -c mbr.s
	cc -fno-strict-aliasing -pipe  -ffreestanding -mpreferred-stack-boundary=2  -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3  -N -e entry -Ttext=0x600 -Wl,-S,--oformat,binary -nostdlib -o mbr mbr.o
first:
	cc -pipe -fno-strict-aliasing -ffreestanding -mpreferred-stack-boundary=2 -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3  -c first.s
	cc -pipe -fno-strict-aliasing -ffreestanding -mpreferred-stack-boundary=2 -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3  -N -e entry -Ttext=0x7c00 -Wl,-S,--oformat,binary -nostdlib -o first first.o
clean:
	rm *.o mbr first
