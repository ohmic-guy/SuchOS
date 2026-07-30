#include "audit.h"
#include "vga.h"

static audit_entry_t buf[AUDIT_BUF_SIZE];
static uint32_t head = 0;
static uint32_t count = 0;

void audit_init(void) { head = count = 0; }

void audit_log(audit_type_t type, uint32_t pid, uint32_t value, uint32_t addr) {
  buf[head].type = type;
  buf[head].pid = pid;
  buf[head].value = value;
  buf[head].addr = addr;
  head = (head + 1) % AUDIT_BUF_SIZE;
  if (count < AUDIT_BUF_SIZE)
    count++;
}

static const char *type_str(audit_type_t t) {
  switch (t) {
  case AUDIT_SYSCALL:
    return "SYSCALL  ";
  case AUDIT_EXCEPTION:
    return "EXCEPTION";
  case AUDIT_SCHED:
    return "SCHED    ";
  case AUDIT_SECURITY:
    return "SECURITY ";
  default:
    return "UNKNOWN  ";
  }
}

void audit_print(void) {
  if (!count) {
    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("  No entries.\n");
    return;
  }
  uint32_t start = count < AUDIT_BUF_SIZE ? 0 : head;
  uint32_t show = count < 16 ? count : 16;
  for (uint32_t i = 0; i < show; i++) {
    uint32_t idx = (start + i) % AUDIT_BUF_SIZE;
    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("  [");
    terminal_writedec(i);
    terminal_write("] ");
    terminal_setcolor(VGA_YELLOW, VGA_BLACK);
    terminal_write(type_str(buf[idx].type));
    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write(" pid=");
    terminal_writedec(buf[idx].pid);
    terminal_write(" val=");
    terminal_writehex(buf[idx].value);
    terminal_write(" addr=");
    terminal_writehex(buf[idx].addr);
    terminal_putchar('\n');
  }
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  Total: ");
  terminal_writedec(count);
  terminal_write(" entries\n");
}

uint32_t audit_count(void) { return count; }