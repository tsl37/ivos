#include "cli.h"
#include "vga.h"
#include "serial.h"
#include "keyboard.h"
#include "ide.h"
#include "string.h"
#include "fat.h"


unsigned int atoi(char *str) {
    unsigned int res = 0;
    while (*str) {
        if (*str >= '0' && *str <= '9') res = res * 10 + (*str - '0');
        else break;
        str++;
    }
    return res;
}

unsigned int atoh(char *str) {
    unsigned int res = 0;
    if (str[0] == '0' && str[1] == 'x') str += 2;
    while (*str) {
        unsigned char byte = *str;
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
        res = (res << 4) | (byte & 0xF);
        str++;
    }
    return res;
}

void cli_readline(char *buffer, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            vga_putchar('\n');
            break;
        } else if (c == '\b' && i > 0) {
            i--;
            int cursor = get_cursor() / 2;
            set_cursor(cursor - 1);
            vga_putchar(' ');
            set_cursor(cursor - 1);
        } else if (c >= 32 && c <= 126) {
            buffer[i++] = c;
            vga_putchar(c);
        }
    }
    buffer[i] = '\0';
}

int cli_parse(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *p = line;
    while (*p && argc < max_args) {
        while (*p == ' ') *p++ = '\0';
        if (*p == '\0') break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
    }
    return argc;
}

void hex_dump(void *addr, int len) {
    unsigned char *p = (unsigned char *)addr;
    for (int i = 0; i < len; i++) {
        if (i % 16 == 0) {
            if (i != 0) vga_putchar('\n');
            vga_print_hex32((unsigned int)p + i);
            vga_print(": ");
        }
        vga_print_hex8(p[i]);
        vga_putchar(' ');
        if ((i + 1) % 8 == 0 && (i + 1) % 16 != 0) vga_putchar(' ');
    }
    vga_putchar('\n');
}


void make_abs_path(const char *cwd, const char *input, char *out) {
    if (input[0] == '/') {
        strcpy(out, input);
    } else {
        strcpy(out, cwd);
        int len = strlen(out);
        if (len > 0 && out[len - 1] != '/') {
            out[len] = '/';
            out[len + 1] = '\0';
        }
        strcpy(out + strlen(out), input);
    }
}


void do_ls(const char *path) {
    Fat16Entry entry;
    uint32_t dir_off;
    int is_root = 0;

    if (strcmp(path, "/") == 0) {
        dir_off = root_dir_start_byte;
        is_root = 1;
    } else {
        if (find_entry_by_path(path, &entry, NULL, NULL) != 0) {
            vga_print("Directory not found.\n");
            return;
        }
        if (!(entry.attributes & 0x10)) {
            vga_print("Not a directory.\n");
            return;
        }
        dir_off = get_cluster_offset(entry.starting_cluster);
    }

    unsigned char buf[512];
    uint32_t limit = is_root ? root_dir_sectors : bs.sectors_per_cluster;
    uint32_t start_sec = dir_off / bs.sector_size;

    for (uint32_t s = 0; s < limit; s++) {
        if (read_sector(buf, start_sec + s) != 0) break;
        Fat16Entry *es = (Fat16Entry *)buf;
        for (int i = 0; i < bs.sector_size / 32; i++) {
            if (es[i].filename[0] == 0x00) {
                vga_print("\n");
                return;
            }
            if (es[i].filename[0] == 0xE5) continue;

            char name[13];
            format_fat_name(&es[i], name);
            if (es[i].attributes & 0x10) {
                vga_print("["); vga_print(name); vga_print("]  ");
            } else {
                vga_print(name); vga_print("  ");
            }
        }
    }
    vga_print("\n");
}


void cli_loop() {
    char line[128];
    char *argv[10];
    char current_path[256] = "/";
    char abs_path[256];

    vga_print("Initializing FAT16...\n");
    if (fat_init("") != 0) {
        vga_print("Warning: FAT16 init failed. Check disk image.\n");
    } else {
        vga_print("FAT16 initialized successfully.\n");
    }

    while (1) {
        vga_print("root@chaos:");
        vga_print(current_path);
        vga_print("# ");
        
        cli_readline(line, 128);
        int argc = cli_parse(line, argv, 10);

        if (argc == 0) continue;

        if (strcmp(argv[0], "help") == 0) {
            vga_print("Commands: clear, read, dump, load, run\n");
            vga_print("FS Commands: ls, cat, cd, tree, stat, exec, touch, rm, write\n");
        } 
        else if (strcmp(argv[0], "clear") == 0) {
            for (int i = 0; i < 25; i++) vga_print("\n");
        } 
       
        else if (strcmp(argv[0], "tree") == 0) {
            printTree();
        } 
        else if (strcmp(argv[0], "ls") == 0) {
            if (argc > 1) {
                make_abs_path(current_path, argv[1], abs_path);
                do_ls(abs_path);
            } else {
                do_ls(current_path);
            }
        } 
        else if (strcmp(argv[0], "cd") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            
            if (strcmp(abs_path, "/") == 0) {
                strcpy(current_path, "/");
            } else {
                Fat16Entry entry;
                if (find_entry_by_path(abs_path, &entry, NULL, NULL) == 0 && (entry.attributes & 0x10)) {
                    strcpy(current_path, abs_path);
                } else {
                    vga_print("Directory not found or not a directory.\n");
                }
            }
        } 
        else if (strcmp(argv[0], "stat") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            Fat16Entry entry;
            if (find_entry_by_path(abs_path, &entry, NULL, NULL) == 0) {
                vga_print("Size: "); vga_print_hex32(entry.file_size); vga_print(" Bytes\n");
                vga_print("Cluster: "); vga_print_hex32(entry.starting_cluster); vga_print("\n");
                vga_print("Attributes: "); vga_print_hex8(entry.attributes); vga_print("\n");
            } else {
                vga_print("File not found.\n");
            }
        } 
        else if (strcmp(argv[0], "cat") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            Fat16Entry entry;
            if (find_entry_by_path(abs_path, &entry, NULL, NULL) == 0) {
                if (entry.attributes & 0x10) {
                    vga_print("Is a directory.\n");
                } else {
                    char buf[512];
                    uint32_t offset = 0;
                    while (offset < entry.file_size) {
                        size_t to_read = (entry.file_size - offset > 512) ? 512 : (entry.file_size - offset);
                        int bytes_read = fat_read_data(&entry, buf, to_read, offset);
                        if (bytes_read <= 0) break;
                        for (int i = 0; i < bytes_read; i++) vga_putchar(buf[i]);
                        offset += bytes_read;
                    }
                    vga_putchar('\n');
                }
            } else {
                vga_print("File not found.\n");
            }
        } 
        else if (strcmp(argv[0], "exec") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            Fat16Entry entry;
            if (find_entry_by_path(abs_path, &entry, NULL, NULL) == 0) {
                vga_print("Loading binary to 0x2000...\n");
                void *load_addr = (void *)0x2000;
                fat_read_data(&entry, load_addr, entry.file_size, 0);
                vga_print("Jumping to 0x2000...\n");
                
                void (*func)() = (void (*)())load_addr;
                func(); 
                
                vga_print("\nProgram finished.\n");
            } else {
                vga_print("File not found.\n");
            }
        }
        else if (strcmp(argv[0], "touch") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            if (fat_create_file(abs_path) == 0) {
                vga_print("File created.\n");
            } else {
                vga_print("Failed to create file.\n");
            }
        }
        else if (strcmp(argv[0], "rm") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            if (fat_delete_file(abs_path) == 0) {
                vga_print("File deleted.\n");
            } else {
                vga_print("Failed to delete file.\n");
            }
        }
        else if (strcmp(argv[0], "write") == 0 && argc > 1) {
            make_abs_path(current_path, argv[1], abs_path);
            fat_create_file(abs_path); 
            
            vga_print("Enter text. Press Ctrl+D to save and exit.\n");
            char wbuf[512];
            int wpos = 0;
            
            while (wpos < 512) {
                char c = keyboard_getchar();
            
                if (c == 4) {
                    break;
                }
            
                if (c == '\b' && wpos > 0) {
                    wpos--;
                    int cursor = get_cursor() / 2;
                    set_cursor(cursor - 1); 
                    vga_putchar(' '); 
                    set_cursor(cursor - 1);
                } 
               
                else if ((c >= 32 && c <= 126) || c == '\n') {
                    wbuf[wpos++] = c;
                    vga_putchar(c);
                }
            }
            
            vga_putchar('\n');
           
            fat_write_data(abs_path, wbuf, wpos, 0);
            
            vga_print("File saved.\n");
        }
       
        else if (strcmp(argv[0], "read") == 0 && argc > 1) {
            unsigned char disk_buffer[512];
            unsigned int lba = atoh(argv[1]);
            ide_read_sector(lba, disk_buffer);
            vga_print("Sector loaded to internal buffer.\n");
        }
        else if (strcmp(argv[0], "dump") == 0 && argc > 1) {
            unsigned int addr = atoh(argv[1]);
            vga_print("Dumping 128 bytes at ");
            vga_print_hex32(addr);
            hex_dump((void *)addr, 128);
        }
        else {
            vga_print("Unknown command. Type 'help'.\n");
        }
    }
}