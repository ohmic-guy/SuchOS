#include "smep.h"
#include <stdint.h>

static int _smep = 0;
static int _smap = 0;

static void cpuid_leaf7(uint32_t *ebx_out) {
  uint32_t eax, ebx, ecx, edx;
  __asm__ volatile("mov $7, %%eax\n"
                   "xor %%ecx, %%ecx\n"
                   "cpuid\n"
                   : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));
  *ebx_out = ebx;
}

void smep_smap_init(void) {
  uint32_t ebx;
  cpuid_leaf7(&ebx);

  uint32_t cr4;
  __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));

  /* SMEP: CPUID.07H:EBX bit 7 → CR4 bit 20 */
  if (ebx & (1u << 7)) {
    cr4 |= (1u << 20);
    _smep = 1;
  }

  /* SMAP: CPUID.07H:EBX bit 20 → CR4 bit 21 */
  if (ebx & (1u << 20)) {
    cr4 |= (1u << 21);
    _smap = 1;
  }

  __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

int smep_enabled(void) { return _smep; }
int smap_enabled(void) { return _smap; }

void smap_allow(void) {
  if (_smap)
    __asm__ volatile("clac");
}

void smap_deny(void) {
  if (_smap)
    __asm__ volatile("stac");
}

uint32_t rdtsc_seed(void) {
  uint32_t lo, hi;
  __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
  (void)hi;
  return lo;
}