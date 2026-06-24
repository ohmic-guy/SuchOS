#include "vga.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"

void kernel_main(void) {
    terminal_init();

    terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
    terminal_write("[ SuchOS v0.3 ]\n\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("VGA driver    ");
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("IDT           ");
    idt_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("PIC remap     ");
    pic_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("Keyboard IRQ  ");
    keyboard_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
    terminal_write("\n> ");
    terminal_setcolor(VGA_WHITE, VGA_BLACK);

    __asm__ volatile ("sti");

    for (;;) __asm__ volatile ("hlt");
}