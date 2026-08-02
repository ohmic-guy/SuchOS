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

QEMU_FLAGS  = -drive format=raw,file=floppy.img,if=floppy,index=0 \
              -display gtk,gl=off \
              -vga std \
              -m 32M

KERNEL_CSRC  = kernel/kernel.c kernel/vga.c kernel/gdt.c \
               kernel/tss.c kernel/idt.c kernel/isr.c \
               kernel/pic.c kernel/keyboard.c \
               kernel/pmm.c kernel/heap.c kernel/paging.c \
               kernel/syscall.c kernel/elf.c kernel/sched.c \
               kernel/smep.c kernel/audit.c \
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
	    -fno-stack-protector -fno-pic -fno-pie -Wall -c $< -o $@

user/hello.elf: user/start.o user/hello.o user/user.ld
	$(CROSS_LD) -T user/user.ld -e _start \
	    -o $@ user/start.o user/hello.o

kernel/hello_elf.o: user/hello.elf
	objcopy -I binary -O elf32-i386 -B i386 $< $@

# ── Kernel link ───────────────────────────────────────────
kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	$(LD) -T kernel/linker.ld -m elf_i386 -o $@ $(KERNEL_OBJS)

kernel.bin: kernel.elf
	objcopy -O binary $< $@

# ── Floppy image ──────────────────────────────────────────
floppy.img: boot.bin stage2.bin kernel.bin
	dd if=/dev/zero  bs=512 count=2880 of=$@ 2>/dev/null
	dd if=boot.bin   of=$@ conv=notrunc    2>/dev/null
	dd if=stage2.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	dd if=kernel.bin of=$@ bs=512 seek=2 conv=notrunc 2>/dev/null
	@echo "Image size: $$(wc -c < kernel.bin) bytes"

# ── Run targets ───────────────────────────────────────────
run: floppy.img
	$(QEMU) $(QEMU_FLAGS)

# Fallback if gtk fails
run-sdl: floppy.img
	$(QEMU) -drive format=raw,file=floppy.img,if=floppy,index=0 \
	        -display sdl -vga std -m 32M

# Debug: GDB stub on port 1234
debug: floppy.img
	$(QEMU) $(QEMU_FLAGS) -s -S &
	gdb -ex "target remote :1234" \
	    -ex "symbol-file kernel.elf"

# ── Clean ─────────────────────────────────────────────────
clean:
	@rm -f *.bin *.elf floppy.img kernel/*.o user/*.o user/*.elf
	@echo "Clean done."

info: kernel.bin
	@echo "Kernel size : $$(wc -c < kernel.bin) bytes"
	@echo "User ELF    : $$(wc -c < user/hello.elf) bytes"

.PHONY: all run run-sdl debug clean info