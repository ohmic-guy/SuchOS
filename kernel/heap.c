#include "heap.h"

static uint32_t heap_start = 0;
static uint32_t heap_end   = 0;
static uint32_t heap_curr  = 0;

void heap_init(uint32_t start, uint32_t size) {
    heap_start = start;
    heap_end   = start + size;
    heap_curr  = start;
}

void* kmalloc(uint32_t size) {
    size = (size + 3) & ~3u; /* 4-byte align */
    if (heap_curr + size > heap_end) return 0;
    void* ptr = (void*)heap_curr;
    heap_curr += size;
    return ptr;
}

void kfree(void* ptr) {
    (void)ptr;
    /* bump allocator — proper free list comes Day 5 */
}

uint32_t heap_used(void) {
    return heap_curr - heap_start;
}