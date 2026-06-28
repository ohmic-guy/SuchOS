#include "tss.h"
#include "gdt.h"

static tss_entry_t tss;
static uint8_t kstack[4096] __attribute__((aligned(16)));

extern void tss_flush(void);

void tss_init(void) {
  uint32_t base = (uint32_t)&tss;
  uint32_t limit = sizeof(tss) - 1;

  /* GDT[5]: Present, DPL=0, Type=9 (32-bit TSS available) */
  gdt_set_gate(5, base, limit, 0x89, 0x00);

  uint8_t *p = (uint8_t *)&tss;
  for (uint32_t i = 0; i < sizeof(tss); i++)
    p[i] = 0;

  tss.ss0 = 0x10; /* kernel data segment */
  tss.esp0 = (uint32_t)kstack + sizeof(kstack);
  tss.iomap_base = sizeof(tss_entry_t);

  tss_flush();
}

void tss_set_kernel_stack(uint32_t stack) { tss.esp0 = stack; }