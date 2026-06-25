#pragma once
#include <stdint.h>

#define PMM_FRAME_SIZE 4096
#define PMM_MEM_SIZE   (32 * 1024 * 1024)
#define PMM_FRAMES     (PMM_MEM_SIZE / PMM_FRAME_SIZE)

void     pmm_init(uint32_t kernel_end);
uint32_t pmm_alloc_frame(void);
void     pmm_free_frame(uint32_t addr);
uint32_t pmm_used_frames(void);