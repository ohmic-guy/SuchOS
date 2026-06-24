#include "pic.h"
#include "port.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

void pic_init(void) {
    /* ICW1 — start init sequence */
    outb(PIC1_CMD,  0x11);
    outb(PIC2_CMD,  0x11);
    /* ICW2 — remap IRQ vectors */
    outb(PIC1_DATA, 0x20);   /* IRQ 0-7  → INT 32-39 */
    outb(PIC2_DATA, 0x28);   /* IRQ 8-15 → INT 40-47 */
    /* ICW3 — cascade identity */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    /* ICW4 — 8086 mode */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    /* Mask all IRQs except IRQ1 (keyboard) */
    outb(PIC1_DATA, 0xFD);   /* 1111 1101 */
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}