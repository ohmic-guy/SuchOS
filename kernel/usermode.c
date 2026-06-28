#include "usermode.h"
#include "vga.h"

/*
 * User program: cli → #GP(0) in ring 3.
 * Proves privilege separation — kernel catches the fault.
 */
static uint8_t user_prog[] = {
    0xFA,      /* cli  — GPF in ring 3 */
    0xEB, 0xFE /* jmp $ — dead fallback */
};

#define USER_CODE_ADDR 0x050000U
#define USER_STACK_TOP 0x078000U

extern void usermode_enter(uint32_t eip, uint32_t esp);

void usermode_test(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\n[ RING 3 TEST ]\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("Copying user code to 0x050000...\n");

  uint8_t *dst = (uint8_t *)USER_CODE_ADDR;
  for (uint32_t i = 0; i < sizeof(user_prog); i++)
    dst[i] = user_prog[i];

  terminal_write("CS=0x1B  SS=0x23  EIP=0x050000\n");
  terminal_write("User code will execute 'cli' -> expect #GP.\n\n");

  usermode_enter(USER_CODE_ADDR, USER_STACK_TOP);
  /* never returns */
}