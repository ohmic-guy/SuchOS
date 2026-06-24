#include "keyboard.h"
#include "isr.h"
#include "vga.h"
#include "port.h"

static const char sc_map[] = {
    0, 0, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' '
};

static void keyboard_handler(registers_t* regs) {
    (void)regs;
    uint8_t sc = inb(0x60);
    if (sc & 0x80) return;          /* key release — ignore */
    if (sc < sizeof(sc_map) && sc_map[sc])
        terminal_putchar(sc_map[sc]);
}

void keyboard_init(void) {
    register_irq_handler(1, keyboard_handler);
}