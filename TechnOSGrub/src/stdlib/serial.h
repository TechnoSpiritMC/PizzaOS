#pragma once

#include <stdbool.h>

#include "../include/stdint.h"

inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline void serial_init() {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

inline void serial_putc(char c) {
    while (!(inb(0x3F8 + 5) & 0x20)) {}
    outb(0x3F8, c);
}

inline void serial_puts(const char* s) { while (*s) serial_putc(*s++); }

int* serial_printf_number(int* argp, int length, bool sign, int radix);
void serial_printf(const char* fmt, ...);