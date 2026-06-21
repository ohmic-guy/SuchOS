Project: SuchOS
Day 2 complete.

State:
- Two-stage bootloader working
- Stage 1: 512B MBR, loads Stage 2 via BIOS int 0x13
- Stage 2: Loads kernel from disk (sector 3 onwards), enables A20, loads GDT, enters 32-bit protected mode, prints VGA banner, jumps to C kernel at 0x10000
- Kernel: C entry point running in protected mode, placed at 0x10000, writes directly to VGA memory
- Build: nasm + gcc (32-bit freestanding) + ld + make + qemu-system-i386 on Linux
- Image: floppy.img, raw format, 1.44MB

Files:
- boot/boot.asm    — Stage 1 MBR
- boot/stage2.asm  — Stage 2 + protected mode + VGA + jump to kernel
- kernel/kernel.c  — C kernel entry point
- kernel/linker.ld — Flat binary linker script
- Makefile         — build + run

Next session (Day 3):
- Create a text mode VGA driver (cursor control, scrolling, string/hex printing)
- Setup the Interrupt Descriptor Table (IDT)
- Implement Interrupt Service Routines (ISRs) and IRQ handlers
- Setup the Programmable Interrupt Controller (PIC)
- Handle basic keyboard input (IRQ 1)
