#include "syscall.h"
#include "vga.h"

static void sys_write(registers_t *regs) {
  const char *str = (const char *)regs->ebx;
  uint32_t len = regs->ecx;
  for (uint32_t i = 0; i < len; i++)
    terminal_putchar(str[i]);
}

static void sys_exit(registers_t *regs) {
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("\n[syscall] sys_exit(");
  terminal_writedec(regs->ebx);
  terminal_write(") from ring 3\n");
  terminal_write("[OK] User process exited cleanly.\n");
  terminal_setcolor(VGA_WHITE, VGA_BLACK);
  /* No scheduler yet — halt. Day 10 returns here properly. */
  __asm__ volatile("cli; hlt");
}

void syscall_handler(registers_t *regs) {
  switch (regs->eax) {
  case SYS_WRITE:
    sys_write(regs);
    break;
  case SYS_EXIT:
    sys_exit(regs);
    break;
  default:
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("[syscall] unknown=");
    terminal_writedec(regs->eax);
    terminal_putchar('\n');
    break;
  }
}

void syscall_init(void) { /* gate registered in idt_init */ }