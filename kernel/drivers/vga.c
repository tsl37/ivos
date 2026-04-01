#include "vga.h"
#define VIDEO_ADDRESS 0xB8000
#define WHITE_ON_BLACK 0x0F
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_CONTROLLER_REGISTER 0x3d4
#define VGA_DATA_REGISTER 0x3d5
#define VGA_CURSOR_LOW_REG 0x0f
#define VGA_CURSOR_HIGH_REG 0x0e

void vga_scroll()
{
    unsigned char *vidmem = (unsigned char *)VIDEO_ADDRESS;

    for (int i = 0; i < VGA_COLS * (VGA_ROWS - 1) * 2; i++)
    {
        vidmem[i] = vidmem[i + VGA_COLS * 2];
    }

    int last_row_offset = VGA_COLS * (VGA_ROWS - 1) * 2;
    for (int i = 0; i < VGA_COLS * 2; i += 2)
    {
        vidmem[last_row_offset + i] = ' ';
        vidmem[last_row_offset + i + 1] = WHITE_ON_BLACK;
    }
}

void set_cursor(int offset)
{
    outb(VGA_CONTROLLER_REGISTER, VGA_CURSOR_HIGH_REG);
    outb(VGA_DATA_REGISTER, (unsigned char)(offset >> 8) & 0xFF);
    outb(VGA_CONTROLLER_REGISTER, VGA_CURSOR_LOW_REG);
    outb(VGA_DATA_REGISTER, (unsigned char)(offset & 0xFF));
}

int get_cursor()
{
    outb(VGA_CONTROLLER_REGISTER, VGA_CURSOR_HIGH_REG);
    int offset = inb(VGA_DATA_REGISTER) << 8;
    outb(VGA_CONTROLLER_REGISTER, VGA_CURSOR_LOW_REG);
    offset |= inb(VGA_DATA_REGISTER);
    return offset * 2;
}
void set_char_at(char c, int offset)
{
    unsigned char *vidmem = (unsigned char *)VIDEO_ADDRESS;
    vidmem[offset] = c;
    vidmem[offset + 1] = WHITE_ON_BLACK;
}

void print_string(char *string)
{
    int offset = get_cursor();
    int i = 0;
    while (string[i] != '\0')
    {
        set_char_at(string[i], offset);
        i++;
        offset += 2;
    }
}

void vga_putchar(char c)
{
    int offset = get_cursor();

    if (c == '\n')
    {

        int current_row = offset / (2 * VGA_COLS);
        offset = (current_row + 1) * 2 * VGA_COLS;
    }
    else
    {

        set_char_at(c, offset);
        offset += 2;
    }

    if (offset >= VGA_COLS * VGA_ROWS * 2)
    {
        vga_scroll();

        offset = VGA_COLS * (VGA_ROWS - 1) * 2;
    }
    set_cursor(offset / 2);
}

void vga_print(const char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        vga_putchar(str[i]);
        i++;
    }
}


void vga_print_nibble(unsigned char nibble) {
    if (nibble <= 9) vga_putchar(nibble + '0');
    else vga_putchar(nibble - 10 + 'A');
}

void vga_print_hex8(unsigned char value) {
    vga_print_nibble((value >> 4) & 0x0F);
    vga_print_nibble(value & 0x0F);
}


void vga_print_hex32(unsigned int value) {
    for (int i = 28; i >= 0; i -= 4) {
        vga_print_nibble((value >> i) & 0x0F);
    }
}