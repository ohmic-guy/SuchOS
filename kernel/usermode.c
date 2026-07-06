#include "usermode.h"
#include "elf.h"
#include "vga.h"
#include <stdint.h>

extern void usermode_enter(uint32_t eip, uint32_t esp);

/* Embedded ELF binary — injected by objcopy */
extern uint8_t _binary_user_hello_elf_start[];
extern uint8_t _binary_user_hello_elf_end[];

#define USER_STACK_TOP 0x078000U
#define USER_CODE_ADDR 0x050000U

void usermode_run_elf(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\n[ ELF LOADER ]\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);

  uint32_t elf_size =
      (uint32_t)(_binary_user_hello_elf_end - _binary_user_hello_elf_start);
  terminal_write("ELF size : ");
  terminal_writedec(elf_size);
  terminal_write(" bytes\n");

  uint32_t entry = elf_load(_binary_user_hello_elf_start, elf_size);
  if (!entry) {
    terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
    terminal_write("[ELF] Load failed\n");
    return;
  }

  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("[ELF] Entering ring 3 at ");
  terminal_writehex(entry);
  terminal_write("\n\nUser output:\n");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);

  usermode_enter(entry, USER_STACK_TOP);
}

/* Day 8 raw bytes test — kept for reference */
static uint8_t user_prog[] = {
    0xB8, 0x01, 0x00, 0x00, 0x00, 0xBB, 0x1D, 0x00, 0x05, 0x00, 0xB9, 0x12,
    0x00, 0x00, 0x00, 0xCD, 0x80, 0xB8, 0x3C, 0x00, 0x00, 0x00, 0xBB, 0x00,
    0x00, 0x00, 0x00, 0xCD, 0x80, 'H',  'e',  'l',  'l',  'o',  ' ',  'f',
    'r',  'o',  'm',  ' ',  'R',  'i',  'n',  'g',  ' ',  '3',  '!'};

void usermode_test(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\n[ RAW BYTES TEST ]\n");
  uint8_t *dst = (uint8_t *)USER_CODE_ADDR;
  for (uint32_t i = 0; i < sizeof(user_prog); i++)
    dst[i] = user_prog[i];
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  usermode_enter(USER_CODE_ADDR, USER_STACK_TOP);
}