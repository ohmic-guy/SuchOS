#pragma once
#include <stdint.h>

void gdt_init(void);
void gdt_set_gate(int i, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t gran);