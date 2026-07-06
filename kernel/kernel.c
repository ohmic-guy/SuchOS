#include "elf.h"
#include "gdt.h"
#include "heap.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "pic.h"
#include "pmm.h"
#include "shell.h"
#include "syscall.h"
#include "tss.h"
#include "vga.h"

extern uint32_t kernel_end;

#define HEAP_START 0x100000U
#define HEAP_SIZE (3 * 1024 * 1024)

#define OK()                                                                   \
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);                               \
  terminal_write("[OK]\n")
#define LBL() terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK)

void kernel_main(void) {
  terminal_init();

  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("[ SuchOS v0.9 ]\n\n");

  LBL();
  terminal_write("GDT              ");
  gdt_init();
  OK();
  LBL();
  terminal_write("TSS              ");
  tss_init();
  OK();
  LBL();
  terminal_write("IDT              ");
  idt_init();
  OK();
  LBL();
  terminal_write("PIC              ");
  pic_init();
  OK();
  LBL();
  terminal_write("Keyboard         ");
  keyboard_init();
  OK();

  LBL();
  terminal_write("PMM              ");
  pmm_init((uint32_t)&kernel_end);
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("[OK] frames=");
  terminal_writedec(pmm_used_frames());
  terminal_write(" used\n");

  LBL();
  terminal_write("Heap             ");
  heap_init(HEAP_START, HEAP_SIZE);
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("[OK] base=0x100000\n");

  LBL();
  terminal_write("PAE + NX + Ring3 ");
  paging_init();
  OK();
  LBL();
  terminal_write("Syscall int 0x80 ");
  syscall_init();
  OK();
  LBL();
  terminal_write("ELF loader       ");
  OK();
  LBL();
  terminal_write("Stack canaries   ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("[OK] guard=0xDEADC0DE\n");

  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("\nType 'help'. Type 'elf' to run user program.\n");

  __asm__ volatile("sti");
  shell_init();
  for (;;)
    __asm__ volatile("hlt");
}