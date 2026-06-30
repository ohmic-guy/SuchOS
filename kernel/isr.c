#include "isr.h"
#include "pic.h"
#include "syscall.h"
#include "vga.h"

static isr_t irq_handlers[16] = {0};

void register_irq_handler(uint8_t irq, isr_t handler) {
  irq_handlers[irq] = handler;
}

static const char *exceptions[] = {"Division By Zero",
                                   "Debug",
                                   "Non-Maskable Interrupt",
                                   "Breakpoint",
                                   "Overflow",
                                   "Bound Range Exceeded",
                                   "Invalid Opcode",
                                   "Device Not Available",
                                   "Double Fault",
                                   "Coprocessor Segment Overrun",
                                   "Invalid TSS",
                                   "Segment Not Present",
                                   "Stack-Segment Fault",
                                   "General Protection Fault",
                                   "Page Fault",
                                   "Reserved",
                                   "x87 FPU Error",
                                   "Alignment Check",
                                   "Machine Check",
                                   "SIMD Float Exception",
                                   "Virtualization Exception",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Reserved",
                                   "Security Exception",
                                   "Reserved"};

void isr_handler(registers_t *regs) {
  /* syscall — do NOT halt, return to user */
  if (regs->int_no == 128) {
    syscall_handler(regs);
    return;
  }

  uint8_t ring = regs->cs & 3;
  terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
  terminal_write("\n[EXCEPTION] ");
  if (regs->int_no < 32)
    terminal_write(exceptions[regs->int_no]);
  terminal_write(" | INT=");
  terminal_writedec(regs->int_no);
  terminal_write(" ERR=");
  terminal_writehex(regs->err_code);
  terminal_write(" | Ring=");
  terminal_writedec(ring);
  terminal_putchar('\n');

  if (regs->int_no == 13 && ring == 3) {
    terminal_setcolor(VGA_LIGHT_GREEN, VGA_BLACK);
    terminal_write("[OK] Privilege separation confirmed.\n");
  }

  terminal_setcolor(VGA_LIGHT_RED, VGA_BLACK);
  terminal_write("SYSTEM HALTED\n");
  __asm__ volatile("cli; hlt");
}

void irq_handler(registers_t *regs) {
  uint8_t irq = (uint8_t)(regs->int_no - 32);
  if (irq_handlers[irq])
    irq_handlers[irq](regs);
  pic_send_eoi(irq);
}