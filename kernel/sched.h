#pragma once
#include <stdint.h>

#define MAX_PROCS 4
#define KSTACK_SIZE 4096

typedef enum { PROC_DEAD = 0, PROC_READY = 1, PROC_RUNNING = 2 } proc_state_t;

typedef struct {
  uint32_t pid;
  proc_state_t state;
  uint32_t esp;
  uint32_t kstack_top;
  char name[16];
  uint8_t kstack[KSTACK_SIZE] __attribute__((aligned(16)));
} process_t;

/* Set by sched_tick/sched_exit, consumed by common_exit in isr.asm */
extern uint32_t sched_esp_ptr;
/* Defined in isr.asm — enters first process */
extern void sched_enter_first(void);

void sched_init(void);
void sched_create(const char *name, uint32_t entry, uint32_t user_esp);
void sched_tick(void *regs_ptr);
void sched_exit(void);
void sched_prepare_first(void);
void sched_print_status(void);
process_t *sched_current(void);
uint32_t sched_count(void);
int sched_active(void);