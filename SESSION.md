Project: SuchOS
Day 3 complete.

State:
- VGA driver: scrolling, color, putchar, writehex, writedec, backspace
- IDT: 256 gates, all 32 CPU exceptions + 16 IRQs handled via NASM stubs
- PIC: remapped IRQ 0-7 → INT 32-39, IRQ 8-15 → INT 40-47
- Keyboard: IRQ1 handler, scancode→ASCII, live input at prompt
- Kernel boots to "> " prompt, accepts keystrokes
- ASM stubs in kernel/isr.asm → compiled as kernel/interrupts.o (not isr.o)
- Build: nasm + gcc -m32 + ld + objcopy + make + qemu

Files:
- boot/boot.asm       — Stage 1 MBR
- boot/stage2.asm     — Stage 2, loads 32 sectors, protected mode, jumps to 0x10000
- kernel/kernel.c     — entry point, init sequence
- kernel/vga.h/c      — full text driver
- kernel/idt.h/c      — IDT setup
- kernel/isr.h/c      — exception + IRQ dispatch
- kernel/isr.asm      — NASM interrupt stubs → interrupts.o
- kernel/pic.h/c      — PIC remap + EOI
- kernel/keyboard.h/c — IRQ1 scancode handler
- kernel/linker.ld    — flat binary at 0x10000
- Makefile

Next session (Day 4):
- GDT in C (move out of stage2.asm)
- Physical memory manager (bitmap allocator)
- Assume flat 32MB RAM
- kmalloc / kfree basics