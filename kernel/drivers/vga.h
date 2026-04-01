#include "io.h"

#define VIDEO_ADDRESS 0xB8000
#define WHITE_ON_BLACK 0x0F
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_CONTROLLER_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_CURSOR_LOW_REG 0x0f
#define VGA_CURSOR_HIGH_REG 0x0e

void vga_scroll() ;
void set_cursor(int offset);
int get_cursor();
void set_char_at(char c, int offset);
void print_string(char *string);
void vga_putchar(char c);
void vga_print(const char *str) ;