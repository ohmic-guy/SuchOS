#pragma once
#include <stdint.h>

static inline void sys_write(const char *str, uint32_t len) {
  __asm__ volatile("int $0x80" : : "a"(1), "b"(str), "c"(len) : "memory");
}

static inline void sys_exit(int code) {
  __asm__ volatile("int $0x80" : : "a"(60), "b"(code));
  __builtin_unreachable();
}