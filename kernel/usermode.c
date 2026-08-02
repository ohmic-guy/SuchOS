#include "usermode.h"
#include "audit.h"
#include "elf.h"
#include "sched.h"
#include "smep.h"
#include "vga.h"
#include <stdint.h>

extern void usermode_enter(uint32_t eip, uint32_t esp);

extern uint8_t _binary_user_hello_elf_start[];
extern uint8_t _binary_user_hello_elf_end[];

#define USER_STACK_TOP 0x078000U
#define USER_CODE_ADDR 0x050000U

/*
 * Task template: ESI counter, message at offset 0x22.
 * ebx immediate (bytes 11-14) patched at load time with ASLR base + 0x22.
 */
static const uint8_t task_tmpl[] = {
    0xBE, 0xFF, 0xFF, 0xFF, 0x00, /* mov esi, 0xFFFFFF    offset 0  */
    0xB8, 0x01, 0x00, 0x00, 0x00, /* mov eax, 1         offset 5  */
    0xBB, 0x00, 0x00, 0x00, 0x00, /* mov ebx, <patched> offset 10 */
    0xB9, 0x02, 0x00, 0x00, 0x00, /* mov ecx, 2         offset 15 */
    0xCD, 0x80,                   /* int 0x80           offset 20 */
    0x4E,                         /* dec esi            offset 22 */
    0x75, 0xEC,                   /* jnz -20 → offset 5 offset 23 */
    0xB8, 0x3C, 0x00, 0x00, 0x00, /* mov eax, 60        offset 25 */
    0x31, 0xDB,                   /* xor ebx, ebx       offset 30 */
    0xCD, 0x80,                   /* int 0x80           offset 32 */
    /* message char + newline at offset 34 = 0x22 */
    0x00, 0x0A};

#define TMPL_MSG_PATCH 11 /* offset of ebx immediate in template */
#define TMPL_MSG_CHAR 34  /* offset of message char              */
#define TMPL_SIZE 36

static void load_task(uint32_t base, char ch) {
  uint8_t *dst = (uint8_t *)base;
  for (uint32_t i = 0; i < TMPL_SIZE; i++)
    dst[i] = task_tmpl[i];

  /* Patch message address */
  uint32_t msg = base + 0x22;
  dst[TMPL_MSG_PATCH + 0] = (uint8_t)(msg & 0xFF);
  dst[TMPL_MSG_PATCH + 1] = (uint8_t)((msg >> 8) & 0xFF);
  dst[TMPL_MSG_PATCH + 2] = (uint8_t)((msg >> 16) & 0xFF);
  dst[TMPL_MSG_PATCH + 3] = (uint8_t)((msg >> 24) & 0xFF);

  /* Patch character */
  dst[TMPL_MSG_CHAR] = (uint8_t)ch;
}

void sched_run_tasks(void) {
  /* ASLR: randomise task bases with RDTSC */
  uint32_t seed = rdtsc_seed();
  uint32_t seed2 = seed ^ (seed << 7) ^ 0xC0FFEE;

  /* Task A: random page in 0x040000-0x04F000 (16 pages) */
  uint32_t base_a = 0x040000 + ((seed % 16) << 12);
  /* Task B: random page in 0x050000-0x05F000 (16 pages) */
  uint32_t base_b = 0x050000 + ((seed2 % 16) << 12);

  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\n[ SCHEDULER — Round Robin + ASLR ]\n");
  terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
  terminal_write("Task A base : ");
  terminal_writehex(base_a);
  terminal_putchar('\n');
  terminal_write("Task B base : ");
  terminal_writehex(base_b);
  terminal_putchar('\n');
  terminal_write("IRQ0 timer preempts between tasks.\n\n");
  terminal_setcolor(VGA_WHITE, VGA_BLACK);
  terminal_write("Output:\n");

  load_task(base_a, 'A');
  load_task(base_b, 'B');

  audit_log(AUDIT_SCHED, 0, base_a, 0);
  audit_log(AUDIT_SCHED, 1, base_b, 0);
  audit_log(AUDIT_SECURITY, 0, (uint32_t)rdtsc_seed(), 0xA51);

  sched_init();
  sched_create("task_a", base_a, 0x06F000);
  sched_create("task_b", base_b, 0x07F000);
  sched_prepare_first();
  __asm__ volatile("cli");
  sched_enter_first();
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