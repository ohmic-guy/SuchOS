#include "syscall.h"

static void write_str(const char *s) {
  uint32_t len = 0;
  while (s[len])
    len++;
  sys_write(s, len);
}

int main(void) {
  write_str("Hello from ELF user program!\n");
  write_str("Running in ring 3 via SuchOS ELF loader.\n");
  sys_exit(0);
  return 0;
}