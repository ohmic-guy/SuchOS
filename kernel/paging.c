#include "paging.h"
#include <stdint.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_RW (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_NX (1ULL << 63)

#define MSR_EFER 0xC0000080U
#define EFER_NXE (1U << 11)

static uint64_t pdpt[4] __attribute__((aligned(32)));
static uint64_t pd[512] __attribute__((aligned(4096)));
static uint64_t pt0[512] __attribute__((aligned(4096)));
static uint64_t pt1[512] __attribute__((aligned(4096)));

static void wrmsr(uint32_t msr, uint64_t val) {
  __asm__ volatile("wrmsr"
                   :
                   : "c"(msr), "a"((uint32_t)(val & 0xFFFFFFFFU)),
                     "d"((uint32_t)(val >> 32)));
}

static uint64_t rdmsr(uint32_t msr) {
  uint32_t lo, hi;
  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return ((uint64_t)hi << 32) | lo;
}

void paging_init(void) {
  uint32_t cr0;

  /* Disable paging before switching to PAE */
  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 &= ~(1u << 31);
  __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

  /* Enable NX in EFER */
  wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_NXE);

  /* PT0: identity map 0x000000-0x1FFFFF */
  for (uint32_t i = 0; i < 512; i++) {
    uint64_t phys = (uint64_t)i << 12;
    uint64_t flags = PAGE_PRESENT | PAGE_RW;
    if (phys >= 0x100000ULL)
      flags |= PAGE_NX; /* heap NX */
    pt0[i] = phys | flags;
  }

  /* User code region: 0x040000-0x05FFFF — executable, user */
  for (uint32_t i = 0x40; i < 0x60; i++)
    pt0[i] = ((uint64_t)i << 12) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

  /* User stack region: 0x060000-0x07FFFF — NX, writable, user */
  for (uint32_t i = 0x60; i < 0x80; i++)
    pt0[i] = ((uint64_t)i << 12) | PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_NX;

  /* PT1: 0x200000-0x3FFFFF — all NX kernel heap */
  for (uint32_t i = 0; i < 512; i++) {
    uint64_t phys = 0x200000ULL + ((uint64_t)i << 12);
    pt1[i] = phys | PAGE_PRESENT | PAGE_RW | PAGE_NX;
  }

  pd[0] = (uint64_t)(uint32_t)pt0 | PAGE_PRESENT | PAGE_RW;
  pd[1] = (uint64_t)(uint32_t)pt1 | PAGE_PRESENT | PAGE_RW;
  for (uint32_t i = 2; i < 512; i++)
    pd[i] = 0;

  pdpt[0] = (uint64_t)(uint32_t)pd | PAGE_PRESENT;
  pdpt[1] = pdpt[2] = pdpt[3] = 0;

  /* Enable PAE */
  uint32_t cr4;
  __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
  cr4 |= (1u << 5);
  __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

  __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)pdpt));

  /* Re-enable paging + CR0.WP */
  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= (1u << 31) | (1u << 16);
  __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}