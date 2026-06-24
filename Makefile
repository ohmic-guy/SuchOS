ASM    = nasm
CC     = gcc
LD     = ld
QEMU   = qemu-system-i386

CFLAGS = -m32 -ffreestanding -fno-pic -nostdlib -fno-builtin \
         -fno-stack-protector -Wall -Wextra -Ikernel

KERNEL_CSRC  = kernel/kernel.c kernel/vga.c kernel/idt.c \
               kernel/isr.c kernel/pic.c kernel/keyboard.c
KERNEL_COBJS = $(KERNEL_CSRC:.c=.o)
KERNEL_OBJS  = $(KERNEL_COBJS) kernel/interrupts.o

all: floppy.img

boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

stage2.bin: boot/stage2.asm
	$(ASM) -f bin $< -o $@

kernel/interrupts.o: kernel/isr.asm
	$(ASM) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	$(LD) -T kernel/linker.ld -m elf_i386 -o $@ $(KERNEL_OBJS)

kernel.bin: kernel.elf
	objcopy -O binary $< $@

floppy.img: boot.bin stage2.bin kernel.bin
	dd if=/dev/zero  bs=512 count=2880 of=$@
	dd if=boot.bin   of=$@ conv=notrunc
	dd if=stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	dd if=kernel.bin of=$@ bs=512 seek=2 conv=notrunc

run: floppy.img
	$(QEMU) -drive format=raw,file=floppy.img,if=floppy,index=0

clean:
	rm -f *.bin *.elf floppy.img kernel/*.o

.PHONY: all run clean