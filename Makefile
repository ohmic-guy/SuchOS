ASM    = nasm
CC     = gcc
LD     = ld
QEMU   = qemu-system-i386

CFLAGS_BASE = -m32 -ffreestanding -fno-pic -nostdlib -fno-builtin \
              -Wall -Wextra -Ikernel
CFLAGS      = $(CFLAGS_BASE) -fno-stack-protector
SP_CFLAGS   = $(CFLAGS_BASE) -fstack-protector-strong

KERNEL_CSRC  = kernel/kernel.c kernel/vga.c kernel/gdt.c \
               kernel/tss.c kernel/idt.c kernel/isr.c \
               kernel/pic.c kernel/keyboard.c \
               kernel/pmm.c kernel/heap.c kernel/paging.c \
               kernel/syscall.c kernel/shell.c kernel/usermode.c
KERNEL_COBJS = $(KERNEL_CSRC:.c=.o)
KERNEL_OBJS  = $(KERNEL_COBJS) kernel/stack_guard.o \
               kernel/interrupts.o kernel/gdt_flush.o \
               kernel/tss_flush.o kernel/usermode_asm.o

all: floppy.img

boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

stage2.bin: boot/stage2.asm
	$(ASM) -f bin $< -o $@

kernel/interrupts.o: kernel/isr.asm
	$(ASM) -f elf32 $< -o $@

kernel/gdt_flush.o: kernel/gdt_flush.asm
	$(ASM) -f elf32 $< -o $@

kernel/tss_flush.o: kernel/tss_flush.asm
	$(ASM) -f elf32 $< -o $@

kernel/usermode_asm.o: kernel/usermode.asm
	$(ASM) -f elf32 $< -o $@

kernel/stack_guard.o: kernel/stack_guard.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(SP_CFLAGS) -c $< -o $@

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