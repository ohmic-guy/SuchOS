#include "heap.h"

#define ALIGN4(x) (((x) + 3) & ~3u)
#define HEADER_SIZE ALIGN4(sizeof(block_t))

typedef struct block {
  uint32_t size;
  uint8_t free;
  struct block *next;
} block_t;

static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static uint32_t heap_curr = 0;
static block_t *free_list = 0;

void heap_init(uint32_t start, uint32_t size) {
  heap_start = start;
  heap_end = start + size;
  heap_curr = start;
  free_list = 0;
}

void *kmalloc(uint32_t size) {
  size = ALIGN4(size);

  /* Search free list first */
  block_t *prev = 0;
  block_t *cur = free_list;
  while (cur) {
    if (cur->free && cur->size >= size) {
      cur->free = 0;
      /* Unlink from free list */
      if (prev)
        prev->next = cur->next;
      else
        free_list = cur->next;
      return (void *)((uint32_t)cur + HEADER_SIZE);
    }
    prev = cur;
    cur = cur->next;
  }

  /* Bump allocate */
  if (heap_curr + HEADER_SIZE + size > heap_end)
    return 0;
  block_t *blk = (block_t *)heap_curr;
  blk->size = size;
  blk->free = 0;
  blk->next = 0;
  heap_curr += HEADER_SIZE + size;
  return (void *)((uint32_t)blk + HEADER_SIZE);
}

void kfree(void *ptr) {
  if (!ptr)
    return;
  block_t *blk = (block_t *)((uint32_t)ptr - HEADER_SIZE);
  blk->free = 1;
  blk->next = free_list;
  free_list = blk;
}

uint32_t heap_used(void) { return heap_curr - heap_start; }