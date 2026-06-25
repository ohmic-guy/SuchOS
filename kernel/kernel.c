#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "pmm.h"
#include "heap.h"

extern uint32_t kernel_end;

#define HEAP_SIZE (4 * 1024 * 1024)

void kernel_main(void) {
    terminal_init();

    terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
    terminal_write("[ SuchOS v0.4 ]\n\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("GDT           ");
    gdt_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("IDT           ");
    idt_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("PIC           ");
    pic_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("Keyboard      ");
    keyboard_init();
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK]\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("PMM           ");
    pmm_init((uint32_t)&kernel_end);
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK] used=");
    terminal_writedec(pmm_used_frames());
    terminal_write(" frames\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("Heap          ");
    uint32_t hstart = ((uint32_t)&kernel_end + 0xFFF) & ~0xFFFu;
    heap_init(hstart, HEAP_SIZE);
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK] base=");
    terminal_writehex(hstart);
    terminal_write("\n");

    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("kmalloc test  ");
    void* p = kmalloc(256);
    if (p) {
        terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
        terminal_write("[OK] ptr=");
        terminal_writehex((uint32_t)p);
        terminal_write("\n");
    } else {
        terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
        terminal_write("[FAIL]\n");
    }

    terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
    terminal_write("\n> ");
    terminal_setcolor(VGA_WHITE, VGA_BLACK);

    __asm__ volatile ("sti");
    for (;;) __asm__ volatile ("hlt");
}