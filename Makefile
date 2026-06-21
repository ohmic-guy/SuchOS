CC      = gcc
CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra
LD      = ld
LDFLAGS = -m elf_i386
ASM     = nasm
QEMU    = qemu-system-i386

all: floppy.img

boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

stage2.bin: boot/stage2.asm
	$(ASM) -f bin $< -o $@

kernel.bin: kernel/kernel.c kernel/linker.ld
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel/kernel.o
	$(LD) $(LDFLAGS) -T kernel/linker.ld kernel/kernel.o -o kernel.bin --oformat binary

floppy.img: boot.bin stage2.bin kernel.bin
	dd if=/dev/zero  bs=512 count=2880 of=$@
	dd if=boot.bin   of=$@ conv=notrunc
	dd if=stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	dd if=kernel.bin of=$@ bs=512 seek=2 conv=notrunc

run: floppy.img
	$(QEMU) -drive format=raw,file=floppy.img,if=floppy,index=0

clean:
	rm -f *.bin kernel/*.o floppy.img
