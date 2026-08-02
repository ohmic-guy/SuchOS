#include "sched.h"
#include "tss.h"
#include "vga.h"

static process_t procs[MAX_PROCS];
static uint32_t cur_pid = 0;
static uint32_t n_procs = 0;
static int active = 0;

uint32_t sched_esp_ptr = 0;

process_t *sched_current(void) { return &procs[cur_pid]; }
uint32_t sched_count(void) { return n_procs; }
int sched_active(void) { return active; }

static void k_memset(uint8_t *p, uint8_t v, uint32_t n) {
  for (uint32_t i = 0; i < n; i++)
    p[i] = v;
}

static void k_strcpy(char *dst, const char *src, uint32_t max) {
  uint32_t i = 0;
  while (i < max - 1 && src[i]) {
    dst[i] = src[i];
    i++;
  }
  dst[i] = 0;
}

void sched_init(void) {
  for (int i = 0; i < MAX_PROCS; i++) {
    procs[i].state = PROC_DEAD;
    procs[i].esp = 0;
    k_memset(procs[i].kstack, 0, KSTACK_SIZE);
  }
  active = 0;
  n_procs = 0;
  cur_pid = 0;
  sched_esp_ptr = 0;
}

void sched_create(const char *name, uint32_t entry, uint32_t user_esp) {
  if (n_procs >= MAX_PROCS)
    return;
  process_t *p = &procs[n_procs];
  p->pid = n_procs;
  p->state = PROC_READY;
  p->kstack_top = (uint32_t)p->kstack + KSTACK_SIZE;
  k_strcpy(p->name, name, 16);

  /*
   * Build fake IRQ interrupt frame on kernel stack.
   * common_exit will pop these in sequence and iret into user mode.
   *
   * Frame layout (low addr = top of stack):
   *   GS, FS, ES, DS      ← isr_common pushed last
   *   EDI..EAX            ← pusha
   *   int_no, err_code    ← ISR macro
   *   EIP, CS, EFLAGS     ← CPU pushed
   *   user_ESP, SS        ← CPU pushed (privilege change)
   */
  uint32_t *sp = (uint32_t *)p->kstack_top;

  *--sp = 0x23;     /* SS  (user data)   */
  *--sp = user_esp; /* ESP (user stack)  */
  *--sp = 0x202;    /* EFLAGS: IF=1      */
  *--sp = 0x1B;     /* CS  (user code)   */
  *--sp = entry;    /* EIP               */
  *--sp = 0;        /* err_code          */
  *--sp = 32;       /* int_no (IRQ0=32)  */
  /* pusha: EAX ECX EDX EBX ESP EBP ESI EDI */
  *--sp = 0; /* EAX */
  *--sp = 0; /* ECX */
  *--sp = 0; /* EDX */
  *--sp = 0; /* EBX */
  *--sp = 0; /* ESP (ignored by popa) */
  *--sp = 0; /* EBP */
  *--sp = 0; /* ESI */
  *--sp = 0; /* EDI */
  /* isr_common: push ds, push es, push fs, push gs */
  *--sp = 0x23; /* DS */
  *--sp = 0x23; /* ES */
  *--sp = 0x23; /* FS */
  *--sp = 0x23; /* GS ← proc->esp */

  p->esp = (uint32_t)sp;
  n_procs++;
}

static uint32_t find_next(void) {
  for (uint32_t i = 1; i < n_procs; i++) {
    uint32_t nxt = (cur_pid + i) % n_procs;
    if (procs[nxt].state == PROC_READY || procs[nxt].state == PROC_RUNNING)
      return nxt;
  }
  return cur_pid;
}

void sched_tick(void *regs_ptr) {
  if (!active || n_procs < 2)
    return;

  process_t *cur = &procs[cur_pid];
  if (cur->state == PROC_RUNNING)
    cur->state = PROC_READY;
  cur->esp = (uint32_t)regs_ptr; /* save GS pointer */

  uint32_t nxt = find_next();
  if (nxt == cur_pid) {
    cur->state = PROC_RUNNING;
    return;
  }

  cur_pid = nxt;
  process_t *next = &procs[nxt];
  next->state = PROC_RUNNING;
  tss_set_kernel_stack(next->kstack_top);
  sched_esp_ptr = (uint32_t)&next->esp;
}

void sched_exit(void) {
  procs[cur_pid].state = PROC_DEAD;

  for (uint32_t i = 1; i <= n_procs; i++) {
    uint32_t nxt = (cur_pid + i) % n_procs;
    if (procs[nxt].state == PROC_READY) {
      cur_pid = nxt;
      procs[nxt].state = PROC_RUNNING;
      tss_set_kernel_stack(procs[nxt].kstack_top);
      sched_esp_ptr = (uint32_t)&procs[nxt].esp;
      return;
    }
  }
  active = 0;
  terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
  terminal_write("\n[sched] All processes done.\n");
  terminal_setcolor(VGA_WHITE, VGA_BLACK);
  terminal_write("[sched] Halting. Restart QEMU.\n");
  __asm__ volatile("cli");
  for (;;)
    __asm__ volatile("hlt");
}

void sched_prepare_first(void) {
  active = 1;
  cur_pid = 0;
  procs[0].state = PROC_RUNNING;
  tss_set_kernel_stack(procs[0].kstack_top);
  sched_esp_ptr = (uint32_t)&procs[0].esp;
}

void sched_print_status(void) {
  terminal_setcolor(VGA_YELLOW, VGA_BLACK);
  terminal_write("\nProcesses:\n");
  static const char *st[] = {"DEAD", "READY", "RUNNING"};
  for (uint32_t i = 0; i < n_procs; i++) {
    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("  [");
    terminal_writedec(procs[i].pid);
    terminal_write("] ");
    terminal_write(procs[i].name);
    terminal_write("  ");
    uint8_t col = procs[i].state == PROC_RUNNING ? VGA_LIGHT_GREEN
                  : procs[i].state == PROC_READY ? VGA_YELLOW
                                                 : VGA_LIGHT_RED;
    terminal_setcolor(col, VGA_BLACK);
    terminal_write(st[procs[i].state]);
    terminal_putchar('\n');
  }
  if (!n_procs) {
    terminal_setcolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_write("  No processes.\n");
  }
}