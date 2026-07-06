ASM      = nasm
CC       = gcc
LD       = ld
QEMU     = qemu-system-i386
CROSS_CC = $(HOME)/opt/cross/bin/i686-elf-gcc
CROSS_LD = $(HOME)/opt/cross/bin/i686-elf-ld

CFLAGS_BASE = -m32 -ffreestanding -fno-pic -nostdlib -fno-builtin \
              -Wall -Wextra -Ikernel
CFLAGS      = $(CFLAGS_BASE) -fno-stack-protector
SP_CFLAGS   = $(CFLAGS_BASE) -fstack-protector-strong

KERNEL_CSRC  = kernel/kernel.c kernel/vga.c kernel/gdt.c \
               kernel/tss.c kernel/idt.c kernel/isr.c \
               kernel/pic.c kernel/keyboard.c \
               kernel/pmm.c kernel/heap.c kernel/paging.c \
               kernel/syscall.c kernel/elf.c \
               kernel/shell.c kernel/usermode.c
KERNEL_COBJS = $(KERNEL_CSRC:.c=.o)
KERNEL_OBJS  = $(KERNEL_COBJS) kernel/stack_guard.o \
               kernel/interrupts.o kernel/gdt_flush.o \
               kernel/tss_flush.o kernel/usermode_asm.o \
               kernel/hello_elf.o

all: floppy.img

# ── Bootloader ────────────────────────────────────────────
boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

stage2.bin: boot/stage2.asm
	$(ASM) -f bin $< -o $@

# ── ASM kernel objects ────────────────────────────────────
kernel/interrupts.o: kernel/isr.asm
	$(ASM) -f elf32 $< -o $@

kernel/gdt_flush.o: kernel/gdt_flush.asm
	$(ASM) -f elf32 $< -o $@

kernel/tss_flush.o: kernel/tss_flush.asm
	$(ASM) -f elf32 $< -o $@

kernel/usermode_asm.o: kernel/usermode.asm
	$(ASM) -f elf32 $< -o $@

# ── Stack guard — no stack protector ─────────────────────
kernel/stack_guard.o: kernel/stack_guard.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Kernel C objects ──────────────────────────────────────
%.o: %.c
	$(CC) $(SP_CFLAGS) -c $< -o $@

# ── User program ──────────────────────────────────────────
user/start.o: user/start.asm
	$(ASM) -f elf32 $< -o $@

user/hello.o: user/hello.c user/syscall.h
	$(CROSS_CC) -m32 -ffreestanding -nostdlib -fno-builtin \
	    -fno-stack-protector -Wall -c $< -o $@

user/hello.elf: user/start.o user/hello.o user/user.ld
	$(CROSS_LD) -T user/user.ld -e _start \
	    -o $@ user/start.o user/hello.o

# Embed ELF binary into kernel as raw bytes
kernel/hello_elf.o: user/hello.elf
	objcopy -I binary -O elf32-i386 -B i386 $< $@

# ── Kernel link ───────────────────────────────────────────
kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	$(LD) -T kernel/linker.ld -m elf_i386 -o $@ $(KERNEL_OBJS)

kernel.bin: kernel.elf
	objcopy -O binary $< $@

# ── Floppy image ──────────────────────────────────────────
floppy.img: boot.bin stage2.bin kernel.bin
	dd if=/dev/zero  bs=512 count=2880 of=$@
	dd if=boot.bin   of=$@ conv=notrunc
	dd if=stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	dd if=kernel.bin of=$@ bs=512 seek=2 conv=notrunc

run: floppy.img
	$(QEMU) -drive format=raw,file=floppy.img,if=floppy,index=0

clean:
	rm -f *.bin *.elf floppy.img kernel/*.o user/*.o user/*.elf

.PHONY: all run clean