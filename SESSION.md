Project: SuchOS
Day 1 complete.

State:
- Two-stage bootloader working
- Stage 1: 512B MBR, loads Stage 2 via BIOS int 0x13
- Stage 2: A20 enabled, GDT loaded, CR0 flipped, 32-bit protected mode
- VGA banner printing at top-left row 0 col 0
- Build: nasm + make + qemu-system-i386 on Fedora
- Image: floppy.img, raw format, 1.44MB

Files:
- boot/boot.asm   — Stage 1 MBR
- boot/stage2.asm — Stage 2 + protected mode + VGA
- Makefile        — build + run

Next session (Day 2):
- Write kernel/kernel.c — C entry point
- Write kernel/linker.ld — flat binary linker script
- Cross-compiler: i686-elf-gcc setup on Fedora
- Stage 2 loads kernel from disk sector 3 onwards
- Jump to kernel from Stage 2
