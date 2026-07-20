#include "usermode.h"
#include "elf.h"
#include "sched.h"
#include "vga.h"
#include <stdint.h>

extern void usermode_enter(uint32_t eip, uint32_t esp);

extern uint8_t _binary_user_hello_elf_start[];
extern uint8_t _binary_user_hello_elf_end[];

#define USER_STACK_TOP 0x078000U
#define USER_CODE_ADDR 0x050000U

/*
 * Task A — placed at 0x040000
 * Prints "A\n" 5 times via sys_write, then sys_exit(0).
 * msg_a at 0x040024 (offset 0x24 in this array).
 *
 * Disassembly:
 *   0x00: mov ecx, 5
 *   0x05: push ecx
 *   0x06: mov eax, 1          ; SYS_WRITE
 *   0x0B: mov ebx, 0x040024   ; msg_a
 *   0x10: mov ecx, 2
 *   0x15: int 0x80
 *   0x17: pop ecx
 *   0x18: dec ecx
 *   0x19: jnz 0x05  (offset 0xEA)
 *   0x1B: mov eax, 60         ; SYS_EXIT
 *   0x20: xor ebx, ebx
 *   0x22: int 0x80
 *   0x24: 'A', 0x0A
 */
static uint8_t task_a[] = {
    0xB9, 0x05, 0x00, 0x00, 0x00, /* mov ecx, 5           */
    0x51,                         /* push ecx             */
    0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 1           */
    0xBB, 0x24, 0x00, 0x04, 0x00, /* mov ebx, 0x040024    */
    0xB9, 0x02, 0x00, 0x00, 0x00, /* mov ecx, 2           */
    0xCD, 0x80,                   /* int 0x80             */
    0x59,                         /* pop ecx              */
    0x49,                         /* dec ecx              */
    0x75, 0xEA,                   /* jnz -22 → 0x05      */
    0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 60          */
    0x31, 0xDB,                   /* xor ebx, ebx         */
    0xCD, 0x80,                   /* int 0x80             */
    0x41, 0x0A                    /* 'A', '\n'            */
};

/*
 * Task B — placed at 0x041000
 * Prints "B\n" 5 times. msg_b at 0x041024.
 * Identical layout to task_a, different msg address and char.
 */
static uint8_t task_b[] = {
    0xB9, 0x05, 0x00, 0x00, 0x00, /* mov ecx, 5           */
    0x51,                         /* push ecx             */
    0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 1           */
    0xBB, 0x24, 0x10, 0x04, 0x00, /* mov ebx, 0x041024    */
    0xB9, 0x02, 0x00, 0x00, 0x00, /* mov ecx, 2           */
    0xCD, 0x80,                   /* int 0x80             */
    0x59,                         /* pop ecx              */
    0x49,                         /* dec ecx              */
    0x75, 0xEA,                   /* jnz -22 → 0x05      */
    0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 60          */
    0x31, 0xDB,                   /* xor ebx, ebx         */
    0xCD, 0x80,                   /* int 0x80             */
    0x42, 0x0A                    /* 'B', '\n'            */
};

void sched_run_tasks(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\n[ SCHEDULER — Round Robin ]\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("Task A  0x040000  prints 'A' x5\n");
  terminal_write("Task B  0x041000  prints 'B' x5\n");
  terminal_write("IRQ0 timer preempts between tasks.\n\n");
  terminal_setcolor(VGA_WHITE, VGA_BLACK);
  terminal_write("Output:\n");

  /* Copy task code to user-executable region */
  uint8_t *dst = (uint8_t *)0x040000;
  for (uint32_t i = 0; i < sizeof(task_a); i++)
    dst[i] = task_a[i];

  dst = (uint8_t *)0x041000;
  for (uint32_t i = 0; i < sizeof(task_b); i++)
    dst[i] = task_b[i];

  sched_init();
  sched_create("task_a", 0x040000, 0x06F000);
  sched_create("task_b", 0x041000, 0x07F000);
  sched_prepare_first(); /* active=1, TSS, sched_esp_ptr set */
  sched_enter_first();   /* iret into task_a — never returns */
}

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