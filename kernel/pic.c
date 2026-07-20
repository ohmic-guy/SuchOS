#include "pic.h"
#include "port.h"

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

void pic_init(void) {
  outb(PIC1_CMD, 0x11);
  outb(PIC2_CMD, 0x11);
  outb(PIC1_DATA, 0x20);
  outb(PIC2_DATA, 0x28);
  outb(PIC1_DATA, 0x04);
  outb(PIC2_DATA, 0x02);
  outb(PIC1_DATA, 0x01);
  outb(PIC2_DATA, 0x01);
  /* IRQ0 (timer) + IRQ1 (keyboard) unmasked */
  outb(PIC1_DATA, 0xFC);
  outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
  if (irq >= 8)
    outb(PIC2_CMD, PIC_EOI);
  outb(PIC1_CMD, PIC_EOI);
}