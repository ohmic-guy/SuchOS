#include <stdint.h>

/*
 * Stack canary. In production this should be seeded from RDTSC or RDRAND.
 * Fixed value sufficient for Day 6 — randomisation comes with RNG support.
 */
uintptr_t __stack_chk_guard = 0xDEADC0DE;

/*
 * Called by gcc when canary mismatch is detected.
 * Stack may be corrupt — bypass terminal, write directly to VGA.
 */
__attribute__((noreturn))
void __stack_chk_fail(void) {
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    const char* msg = "!! STACK SMASH DETECTED - SYSTEM HALTED !!";
    for (uint32_t i = 0; msg[i]; i++)
        vga[i] = (uint16_t)((uint8_t)msg[i]) | 0x4F00U; /* white on red */
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}