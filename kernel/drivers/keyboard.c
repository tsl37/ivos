#include "keyboard.h"
#include "io.h"
#include "serial.h"
#define SCAN_LCTRL 0x1D
#define SCAN_RCTRL 0xE0 // Right Ctrl is often a multi-byte sequence; simple handling below
#define RELEASE_BIT 0x80

static int ctrl_pressed = 0; // Global or static state tracker

const char scancode_to_ascii[128] = {

    [0x01] = 27, // ESC
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',

    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',

    [0x37] = '*',
    [0x39] = ' '};

int keyboard_getchar()
{
    unsigned char scancode;

    while (1)
    {
        while ((inb(0x64) & 1) == 0)
            ;
        scancode = inb(0x60);
        serial_print("Scancode: ");
        serial_print_hex(scancode);
        serial_print("\n");
        if (scancode & RELEASE_BIT)
        {
            unsigned char released_scancode = scancode & ~RELEASE_BIT;
            if (released_scancode == SCAN_LCTRL)
            {
                ctrl_pressed = 0;
            }
            continue;
        }
        if (scancode == SCAN_LCTRL)
        {
            ctrl_pressed = 1;
            continue;
        }

        if (scancode < 128)
        {
            char ascii = scancode_to_ascii[scancode];

            if (ascii != 0)
            {

                if (ctrl_pressed)
                {

                    if (ascii == 'c' || ascii == 'C')
                        return 3;

                    if (ascii == 'd' || ascii == 'D')
                        return 4;
                }
                return ascii;
            }
        }
    }
}