#include "shell.h"
#include "heap.h"
#include "pmm.h"
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
  terminal_write("  help     show this message\n");
  terminal_write("  clear    clear the screen\n");
  terminal_write("  mem      memory usage info\n");
  terminal_write("  version  SuchOS version info\n");
}

static void cmd_clear(void) { terminal_init(); }

static void cmd_mem(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\nMemory info:\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  PMM frames used : ");
  terminal_writedec(pmm_used_frames());
  terminal_write(" / 8192\n");
  terminal_write("  Heap used       : ");
  terminal_writedec(heap_used());
  terminal_write(" bytes\n");
}

static void cmd_version(void) {
  terminal_setcolor(VGA_LIGHT_CYAN, VGA_BLACK);
  terminal_write("\nSuchOS v0.5\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("  Arch  : x86 32-bit protected mode\n");
  terminal_write("  Build : NASM + GCC -m32 + LD\n");
  terminal_write("  Paging: identity mapped 0x00000000-0x003FFFFF\n");
}

static void shell_execute(const char *cmd) {
  if (streq(cmd, "help")) {
    cmd_help();
  } else if (streq(cmd, "clear")) {
    cmd_clear();
  } else if (streq(cmd, "mem")) {
    cmd_mem();
  } else if (streq(cmd, "version")) {
    cmd_version();
  } else {
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("Unknown command: ");
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