#include "shell.h"
#include "heap.h"
#include "pmm.h"
#include "usermode.h"
#include "vga.h"

#define CMD_BUF_SIZE 256

static char cmd_buf[CMD_BUF_SIZE];
static uint32_t cmd_len = 0;

static int streq(const char *a, const char *b) {
  while (*a && *b) {
    if (*a != *b)
      return 0;
    a++;
    b++;
  }
  return *a == *b;
}

static void print_prompt(void) {
  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("> ");
  terminal_setcolor(VGA_WHITE, VGA_BLACK);
}

static void cmd_help(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\nCommands:\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  help      this message\n");
  terminal_write("  clear     clear screen\n");
  terminal_write("  mem       memory stats\n");
  terminal_write("  security  security status\n");
  terminal_write("  version   version info\n");
  terminal_write("  ring3     enter ring 3 (WARNING: halts after)\n");
}

static void cmd_clear(void) { terminal_init(); }

static void cmd_mem(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\nMemory:\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  PMM frames used : ");
  terminal_writedec(pmm_used_frames());
  terminal_write(" / 8192\n");
  terminal_write("  Heap used       : ");
  terminal_writedec(heap_used());
  terminal_write(" bytes\n");
}

static void cmd_security(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\nSecurity:\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  PAE paging    ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("enabled\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  NX bit        ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("enabled\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  CR0.WP        ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("enabled\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  Stack canary  ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("0xDEADC0DE\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  TSS           ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("loaded (GDT[5])\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  Ring 3 pages  ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("0x040000-0x07FFFF\n");
}

static void cmd_version(void) {
  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("\nSuchOS v0.7\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  Arch   : x86 32-bit\n");
  terminal_write("  Rings  : 0 (kernel) + 3 (user)\n");
  terminal_write("  Paging : PAE + NX + user pages\n");
  terminal_write("  GDT    : 6 descriptors + TSS\n");
}

static void shell_execute(const char *cmd) {
  if (streq(cmd, "help"))
    cmd_help();
  else if (streq(cmd, "clear"))
    cmd_clear();
  else if (streq(cmd, "mem"))
    cmd_mem();
  else if (streq(cmd, "security"))
    cmd_security();
  else if (streq(cmd, "version"))
    cmd_version();
  else if (streq(cmd, "ring3"))
    usermode_test(); /* halts after */
  else {
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("Unknown: ");
    terminal_setcolor(VGA_WHITE, VGA_BLACK);
    terminal_write(cmd);
    terminal_putchar('\n');
  }
}

void shell_handle_char(char c) {
  if (c == '\n') {
    terminal_putchar('\n');
    cmd_buf[cmd_len] = '\0';
    if (cmd_len > 0)
      shell_execute(cmd_buf);
    cmd_len = 0;
    print_prompt();
  } else if (c == '\b') {
    if (cmd_len > 0) {
      cmd_len--;
      terminal_putchar('\b');
    }
  } else if (cmd_len < CMD_BUF_SIZE - 1) {
    cmd_buf[cmd_len++] = c;
    terminal_putchar(c);
  }
}

void shell_init(void) {
  cmd_len = 0;
  print_prompt();
}