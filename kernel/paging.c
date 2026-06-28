#include "paging.h"
#include <stdint.h>

#define PAGE_PRESENT  (1ULL << 0)
#define PAGE_RW       (1ULL << 1)
#define PAGE_NX       (1ULL << 63)

#define MSR_EFER      0xC0000080U
#define EFER_NXE      (1U << 11)

/* PAE: all entries are 64-bit */
static uint64_t pdpt[4]    __attribute__((aligned(32)));
static uint64_t pd  [512]  __attribute__((aligned(4096)));
static uint64_t pt0 [512]  __attribute__((aligned(4096))); /* 0x000000-0x1FFFFF */
static uint64_t pt1 [512]  __attribute__((aligned(4096))); /* 0x200000-0x3FFFFF */

static void wrmsr(uint32_t msr, uint64_t val) {
    __asm__ volatile("wrmsr"
        : : "c"(msr),
            "a"((uint32_t)(val & 0xFFFFFFFFU)),
            "d"((uint32_t)(val >> 32)));
}

static uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

void paging_init(void) {
    /* Disable paging before switching to PAE */
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1u << 31);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    /* Enable NX in EFER — must happen before PAE enable */
    uint64_t efer = rdmsr(MSR_EFER);
    efer |= EFER_NXE;
    wrmsr(MSR_EFER, efer);

    /*
     * PT0: identity maps 0x000000-0x1FFFFF
     *   0x000000-0x0FFFFF: executable — kernel code lives at 0x10000
     *   0x100000-0x1FFFFF: NX — heap region
     */
    for (uint32_t i = 0; i < 512; i++) {
        uint64_t phys  = (uint64_t)i * 0x1000;
        uint64_t flags = PAGE_PRESENT | PAGE_RW;
        if (phys >= 0x100000ULL) flags |= PAGE_NX;
        pt0[i] = phys | flags;
    }

    /*
     * PT1: identity maps 0x200000-0x3FFFFF
     *   All NX — upper heap only
     */
    for (uint32_t i = 0; i < 512; i++) {
        uint64_t phys = 0x200000ULL + (uint64_t)i * 0x1000;
        pt1[i] = phys | PAGE_PRESENT | PAGE_RW | PAGE_NX;
    }

    /* PD: first two entries cover 4MB */
    pd[0] = (uint64_t)(uint32_t)pt0 | PAGE_PRESENT | PAGE_RW;
    pd[1] = (uint64_t)(uint32_t)pt1 | PAGE_PRESENT | PAGE_RW;
    for (uint32_t i = 2; i < 512; i++) pd[i] = 0;

    /* PDPT: entry 0 covers 0-1GB */
    pdpt[0] = (uint64_t)(uint32_t)pd | PAGE_PRESENT;
    pdpt[1] = pdpt[2] = pdpt[3] = 0;

    /* Enable PAE — CR4 bit 5 */
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1u << 5);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    /* Load CR3 with PDPT base */
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)pdpt));

    /* Re-enable paging + Write-Protect (CR0.WP = bit 16) */
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= (1u << 31) | (1u << 16);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}