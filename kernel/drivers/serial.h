#include "io.h"
void serial_init();
void serial_putchar(char c);
void serial_print(const char *str);
void serial_print_hex(unsigned char val);