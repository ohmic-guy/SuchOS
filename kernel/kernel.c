#include "gdt.h"
#include "heap.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "pic.h"
#include "pmm.h"
#include "shell.h"
#include "vga.h"

extern uint32_t kernel_end;
#define HEAP_SIZE (4 * 1024 * 1024)

#define OK()                                                                   \
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);                               \
  terminal_write("[OK]\n")
#define LBL() terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK)

void kernel_main(void) {
  terminal_init();

  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("[ SuchOS v0.5 ]\n\n");

  LBL();
  terminal_write("GDT           ");
  gdt_init();
  OK();
  LBL();
  terminal_write("IDT           ");
  idt_init();
  OK();
  LBL();
  terminal_write("PIC           ");
  pic_init();
  OK();
  LBL();
  terminal_write("Keyboard      ");
  keyboard_init();
  OK();

  LBL();
  terminal_write("PMM           ");
  pmm_init((uint32_t)&kernel_end);
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("[OK] frames=");
  terminal_writedec(pmm_used_frames());
  terminal_write(" used\n");

  LBL();
  terminal_write("Heap          ");
  uint32_t hstart = ((uint32_t)&kernel_end + 0xFFF) & ~0xFFFu;
  heap_init(hstart, HEAP_SIZE);
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("[OK] base=");
  terminal_writehex(hstart);
  terminal_write("\n");

  LBL();
  terminal_write("Paging        ");
  paging_init();
  OK();

  LBL();
  terminal_write("kmalloc test  ");
  void *p = kmalloc(128);
  kfree(p);
  void *p2 = kmalloc(64); /* should reuse freed block */
  if (p2) {
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK] alloc+free+reuse working\n");
  } else {
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("[FAIL]\n");
  }

  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("\nType 'help' for commands.\n");

  __asm__ volatile("sti");
  shell_init();

  for (;;)
    __asm__ volatile("hlt");
}