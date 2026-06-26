#include "paging.h"

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_SIZE 4096
#define TABLE_ENTRIES 1024

static uint32_t page_dir[TABLE_ENTRIES] __attribute__((aligned(4096)));
static uint32_t page_table[TABLE_ENTRIES] __attribute__((aligned(4096)));

void paging_init(void) {
  /* Identity map first 4MB: virt == phys */
  for (uint32_t i = 0; i < TABLE_ENTRIES; i++)
    page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;

  /* Point page dir entry 0 at our page table */
  page_dir[0] = (uint32_t)page_table | PAGE_PRESENT | PAGE_RW;

  /* All other entries not present */
  for (uint32_t i = 1; i < TABLE_ENTRIES; i++)
    page_dir[i] = 0;

  /* Load CR3 with page directory base */
  __asm__ volatile("mov %0, %%cr3" : : "r"(page_dir));

  /* Set CR0 bit 31 — enable paging */
  uint32_t cr0;
  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= (1u << 31);
  __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}