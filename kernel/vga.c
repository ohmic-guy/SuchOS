#include "port.h"
#include "vga.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

static uint16_t* const VGA = (uint16_t*)0xB8000;
static uint32_t col   = 0;
static uint32_t row   = 0;
static uint8_t  color = 0;

static inline uint8_t mk_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t mk_entry(char c, uint8_t col) {
    return (uint16_t)c | ((uint16_t)col << 8);
}

void terminal_init(void) {
    color = mk_color(VGA_LIGHT_CYAN, VGA_BLACK);
    col = row = 0;
    for (uint32_t y = 0; y < VGA_HEIGHT; y++)
        for (uint32_t x = 0; x < VGA_WIDTH; x++)
            VGA[y * VGA_WIDTH + x] = mk_entry(' ', color);
}

void terminal_setcolor(uint8_t fg, uint8_t bg) {
    color = mk_color(fg, bg);
}

static void scroll(void) {
    for (uint32_t y = 0; y < VGA_HEIGHT - 1; y++)
        for (uint32_t x = 0; x < VGA_WIDTH; x++)
            VGA[y * VGA_WIDTH + x] = VGA[(y + 1) * VGA_WIDTH + x];
    for (uint32_t x = 0; x < VGA_WIDTH; x++)
        VGA[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = mk_entry(' ', color);
    row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) { outb(0x3F8, c);
    if (c == '\n') {
        col = 0;
        if (++row == VGA_HEIGHT) scroll();
        return;
    }
    if (c == '\r') { col = 0; return; }
    if (c == '\b') {
        if (col > 0) {
            col--;
            VGA[row * VGA_WIDTH + col] = mk_entry(' ', color);
        }
        return;
    }
    VGA[row * VGA_WIDTH + col] = mk_entry(c, color);
    if (++col == VGA_WIDTH) {
        col = 0;
        if (++row == VGA_HEIGHT) scroll();
    }
}

void terminal_write(const char* s) {
    for (uint32_t i = 0; s[i]; i++) terminal_putchar(s[i]);
}

void terminal_writehex(uint32_t n) {
    terminal_write("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nib = (n >> i) & 0xF;
        terminal_putchar(nib < 10 ? '0' + nib : 'A' + nib - 10);
    }
}

void terminal_writedec(uint32_t n) {
    if (!n) { terminal_putchar('0'); return; }
    char buf[12]; int i = 0;
    while (n) { buf[i++] = '0' + (n % 10); n /= 10; }
    for (int j = i - 1; j >= 0; j--) terminal_putchar(buf[j]);
}