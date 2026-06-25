#include "pmm.h"

#define BITMAP_SIZE (PMM_FRAMES / 32)

static uint32_t bitmap[BITMAP_SIZE];

static void set_bit(uint32_t f)   { bitmap[f/32] |=  (1u << (f%32)); }
static void clear_bit(uint32_t f) { bitmap[f/32] &= ~(1u << (f%32)); }
static int  test_bit(uint32_t f)  { return bitmap[f/32] &  (1u << (f%32)); }

void pmm_init(uint32_t kernel_end) {
    /* mark everything used */
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        bitmap[i] = 0xFFFFFFFF;

    /* free frames above kernel */
    uint32_t first_free = kernel_end / PMM_FRAME_SIZE + 1;
    for (uint32_t i = first_free; i < PMM_FRAMES; i++)
        clear_bit(i);
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < PMM_FRAMES; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            return i * PMM_FRAME_SIZE;
        }
    }
    return 0; /* OOM */
}

void pmm_free_frame(uint32_t addr) {
    clear_bit(addr / PMM_FRAME_SIZE);
}

uint32_t pmm_used_frames(void) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < PMM_FRAMES; i++)
        if (test_bit(i)) n++;
    return n;
}