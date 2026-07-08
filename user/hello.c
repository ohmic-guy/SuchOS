#include "syscall.h"

static void write_str(const char *s, uint32_t len) { sys_write(s, len); }

int main(void) {
  write_str("Hello from ELF!\n", 16);
  write_str("Ring 3 via SuchOS ELF loader.\n", 30);
  sys_exit(0);
  return 0;
}