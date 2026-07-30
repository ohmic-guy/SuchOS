#pragma once
#include <stdint.h>

#define AUDIT_BUF_SIZE 64

typedef enum {
  AUDIT_SYSCALL = 0,
  AUDIT_EXCEPTION = 1,
  AUDIT_SCHED = 2,
  AUDIT_SECURITY = 3
} audit_type_t;

typedef struct {
  audit_type_t type;
  uint32_t pid;
  uint32_t value;
  uint32_t addr;
} audit_entry_t;

void audit_init(void);
void audit_log(audit_type_t type, uint32_t pid, uint32_t value, uint32_t addr);
void audit_print(void);
uint32_t audit_count(void);