#include "serial.h"



#define COM1_PORT 0x3f8

void serial_init() {
    outb(COM1_PORT + 1, 0x00);    // Vypne všechna přerušení sériového portu
    outb(COM1_PORT + 3, 0x80);    // Povolí DLAB (Divisor Latch Access Bit) pro nastavení rychlosti
    outb(COM1_PORT + 0, 0x03);    // Nastaví dělitel rychlosti na 3 (low byte) -> 38400 baudů
    outb(COM1_PORT + 1, 0x00);    // (high byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bitů, žádná parita, jeden stop bit
    outb(COM1_PORT + 2, 0xC7);    // Povolí FIFO fronty, vyčistí je a nastaví práh na 14 bytů
    outb(COM1_PORT + 4, 0x0B);    // Povolí IRQ, nastaví signály RTS/DSR
}

void serial_putchar(char c) {
    while ((inb(COM1_PORT + 5) & 0x20) == 0) {}
    outb(COM1_PORT, c);
}

void serial_print(const char *str) {
    int i = 0;
    while (str[i] != '\0') {
        serial_putchar(str[i]);
        i++;
    }
}

void serial_print_hex(unsigned char val) {
    const char *hex = "0123456789ABCDEF";
    serial_putchar('0');
    serial_putchar('x');
    serial_putchar(hex[(val >> 4) & 0x0F]); // První znak
    serial_putchar(hex[val & 0x0F]);        // Druhý znak
    serial_putchar('\n');
}