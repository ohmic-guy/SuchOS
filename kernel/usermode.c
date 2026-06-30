#include "usermode.h"
#include "vga.h"

/*
 * User program layout at USER_CODE_ADDR:
 * Bytes 0-28: instructions
 * Bytes 29+:  message string
 *
 * Message address = USER_CODE_ADDR + 29 = 0x05001D
 * Little-endian:  0x1D 0x00 0x05 0x00
 */
static uint8_t user_prog[] = {
    /* mov eax, 1 (SYS_WRITE) */
    0xB8, 0x01, 0x00, 0x00, 0x00,
    /* mov ebx, 0x05001D (message address) */
    0xBB, 0x1D, 0x00, 0x05, 0x00,
    /* mov ecx, 18 (message length) */
    0xB9, 0x12, 0x00, 0x00, 0x00,
    /* int 0x80 */
    0xCD, 0x80,
    /* mov eax, 60 (SYS_EXIT) */
    0xB8, 0x3C, 0x00, 0x00, 0x00,
    /* mov ebx, 0 (exit code) */
    0xBB, 0x00, 0x00, 0x00, 0x00,
    /* int 0x80 */
    0xCD, 0x80,
    /* message: "Hello from Ring 3!" (18 bytes, no null needed) */
    'H', 'e', 'l', 'l', 'o', ' ', 'f', 'r', 'o', 'm', ' ', 'R', 'i', 'n', 'g',
    ' ', '3', '!'};

#define USER_CODE_ADDR 0x050000U
#define USER_STACK_TOP 0x078000U

extern void usermode_enter(uint32_t eip, uint32_t esp);

void usermode_test(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\n[ SYSCALL TEST ]\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("Loading user program at 0x050000...\n");
  terminal_write("Program calls: sys_write(1) then sys_exit(60)\n\n");

  uint8_t *dst = (uint8_t *)USER_CODE_ADDR;
  for (uint32_t i = 0; i < sizeof(user_prog); i++)
    dst[i] = user_prog[i];

  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("Output from ring 3: ");
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);

  usermode_enter(USER_CODE_ADDR, USER_STACK_TOP);
}