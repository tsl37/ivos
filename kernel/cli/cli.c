#include "cli.h"
#include "vga.h"
#include "serial.h"
#include "keyboard.h"
#include "ide.h"
#include "string.h"

unsigned int atoi(char *str)
{
    unsigned int res = 0;
    while (*str)
    {
        if (*str >= '0' && *str <= '9')
        {
            res = res * 10 + (*str - '0');
        }
        else
        {
            break;
        }
        str++;
    }
    return res;
}

unsigned int atoh(char *str)
{
    unsigned int res = 0;
    if (str[0] == '0' && str[1] == 'x')
        str += 2;
    while (*str)
    {
        unsigned char byte = *str;
        if (byte >= '0' && byte <= '9')
            byte = byte - '0';
        else if (byte >= 'a' && byte <= 'f')
            byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <= 'F')
            byte = byte - 'A' + 10;
        res = (res << 4) | (byte & 0xF);
        str++;
    }
    return res;
}

void cli_readline(char *buffer, int max_len)
{
    int i = 0;
    while (i < max_len - 1)
    {
        serial_print("Waiting for input...\n");
        char c = keyboard_getchar();
        if (c == '\n')
        {
            vga_putchar('\n');
            break;
        }
        else if (c == '\b' && i > 0)
        {
            i--;
            int cursor = get_cursor() / 2;
            set_cursor(cursor - 1);
            vga_putchar(' ');
            set_cursor(cursor - 1);
        }
        else if (c >= 32 && c <= 126)
        {
            buffer[i++] = c;
            vga_putchar(c);
        }
    }
    buffer[i] = '\0';
}

int cli_parse(char *line, char *argv[], int max_args)
{
    int argc = 0;
    char *p = line;
    while (*p && argc < max_args)
    {
        while (*p == ' ')
            *p++ = '\0';
        if (*p == '\0')
            break;
        argv[argc++] = p;
        while (*p && *p != ' ')
            p++;
    }
    return argc;
}

void hex_dump(void *addr, int len)
{
    unsigned char *p = (unsigned char *)addr;
    for (int i = 0; i < len; i++)
    {
        if (i % 16 == 0)
        {
            if (i != 0)
                vga_putchar('\n');
            vga_print_hex32((unsigned int)p + i);
            vga_print(": ");
        }

        vga_print_hex8(p[i]);
        vga_putchar(' ');

        if ((i + 1) % 8 == 0 && (i + 1) % 16 != 0)
        {
            vga_putchar(' ');
        }
    }
    vga_putchar('\n');
}

void cli_loop()
{
    char line[64];
    char *argv[5];
    unsigned char disk_buffer[512];

    while (1)
    {
        vga_print("ChurchOS> ");
        cli_readline(line, 64);
        int argc = cli_parse(line, argv, 5);

        if (argc == 0)
            continue;

        if (strcmp(argv[0], "help") == 0)
        {
            vga_print("Commands: help, clear, read, dump, load, run\n");
        }
        else if (strcmp(argv[0], "clear") == 0)
        {
            for (int i = 0; i < 25; i++)
                vga_print("\n");
        }
        else if (strcmp(argv[0], "read") == 0 && argc > 1)
        {
            unsigned int lba = atoh(argv[1]);
            ide_read_sector(lba, disk_buffer);
            vga_print("Sector loaded to internal buffer.\n");
        }
        else if (strcmp(argv[0], "dump") == 0 && argc > 1)
        {
            unsigned int addr = atoh(argv[1]);
            vga_print("Dumping 128 bytes at ");
            vga_print_hex32(addr);
            hex_dump((void *)addr, 128);
        }
        else if (strcmp(argv[0], "load") == 0 && argc > 2)
        {
            unsigned int lba = atoi(argv[1]);
            unsigned int addr = atoh(argv[2]);
            ide_read_sector(lba, (void *)addr);
            vga_print("Loaded LBA to memory.\n");
        }
        else if (strcmp(argv[0], "run") == 0 && argc > 1)
        {
            unsigned int addr = atoh(argv[1]);
            vga_print("Executing code...\n");
            void (*func)() = (void (*)())addr;
            func();
        }
        else
        {
            vga_print("Unknown command.\n");
        }
    }
}