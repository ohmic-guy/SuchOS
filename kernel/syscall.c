#include "syscall.h"
#include "audit.h"
#include "sched.h"
#include "smep.h"
#include "vga.h"

static void sys_write(registers_t *regs) {
  const char *str = (const char *)regs->ebx;
  uint32_t len = regs->ecx;
  uint32_t pid = sched_active() ? sched_current()->pid : 0;

  audit_log(AUDIT_SYSCALL, pid, 1, regs->ebx);

  smap_allow(); /* CLAC — read user buffer */
  for (uint32_t i = 0; i < len; i++)
    terminal_putchar(str[i]);
  smap_deny(); /* STAC — re-enable SMAP   */
}

static void sys_exit(registers_t *regs) {
  uint32_t pid = sched_active() ? sched_current()->pid : 0;
  audit_log(AUDIT_SYSCALL, pid, 60, regs->ebx);

  if (sched_active()) {
    audit_log(AUDIT_SCHED, pid, 0, 0);
    sched_exit();
    return;
  }
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("\n[syscall] sys_exit(");
  terminal_writedec(regs->ebx);
  terminal_write(") from ring 3\n");
  terminal_write("[OK] User process exited cleanly.\n");
  terminal_setcolor(VGA_WHITE, VGA_BLACK);
  __asm__ volatile("cli; hlt");
}

void syscall_handler(registers_t *regs) {
  switch (regs->eax) {
  case 1:
    sys_write(regs);
    break;
  case 60:
    sys_exit(regs);
    break;
  default:
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("[syscall] unknown=");
    terminal_writedec(regs->eax);
    terminal_putchar('\n');
    audit_log(AUDIT_SYSCALL, 0, regs->eax, 0xFFFFFFFF);
    break;
  }
}

void syscall_init(void) {}