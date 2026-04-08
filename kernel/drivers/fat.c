#include "fat.h"
#include "ide.h"
#include "string.h"
#include "vga.h"
#include <stdint.h>


#define ENOENT  2
#define ENOTDIR 20
#define EINVAL  22
#define ENOSPC  28
#define EEXIST  17

Fat16BootSector bs;
PartitionTable pt[4];
uint32_t fat_start_byte;
uint32_t root_dir_start_byte;
uint32_t data_start_sector;
uint32_t partition_start_byte;
uint32_t root_dir_sectors;


int read_sector(void *buf, uint32_t sector) {
    return ide_read_sector(sector, buf);
}

int write_sector(void *buf, uint32_t sector) {
    return ide_write_sector(sector, buf);
}


uint16_t get_fat_entry(uint16_t cluster) {
    uint32_t offset = fat_start_byte + (cluster * 2);
    uint32_t sector = offset / 512;
    uint32_t sec_offset = offset % 512;
    
    unsigned char buf[512];
    read_sector(buf, sector);
    
    return *((uint16_t*)(buf + sec_offset));
}

void set_fat_entry(uint16_t cluster, uint16_t value) {
    
    uint32_t offset = fat_start_byte + (cluster * 2);
    uint32_t sector = offset / 512;
    uint32_t sec_offset = offset % 512;
    
    unsigned char buf[512];
    read_sector(buf, sector);
    *((uint16_t*)(buf + sec_offset)) = value;
    write_sector(buf, sector);
    serial_print("Fat entry set: ");
    if (bs.number_of_fats > 1) {
        uint32_t fat2_offset = offset + (bs.fat_size_sectors * bs.sector_size);
        uint32_t fat2_sector = fat2_offset / 512;
        uint32_t fat2_sec_offset = fat2_offset % 512;
        read_sector(buf, fat2_sector);
     
        *((uint16_t*)(buf + fat2_sec_offset)) = value;
        write_sector(buf, fat2_sector);
    }
      serial_print("second Fat entry set: ");
}

uint32_t get_cluster_offset(uint16_t n) {
    if (n < 2) return root_dir_start_byte;
    return (data_start_sector + (n - 2) * bs.sectors_per_cluster) * bs.sector_size;
}

void format_fat_name(const Fat16Entry *entry, char *out) {
    int p = 0;
    for (int i = 0; i < 8 && entry->filename[i] != ' '; i++) out[p++] = entry->filename[i];
    if (entry->ext[0] != ' ') {
        out[p++] = '.';
        for (int i = 0; i < 3 && entry->ext[i] != ' '; i++) out[p++] = entry->ext[i];
    }
    out[p] = '\0';
}

int fat_init(const char *image_path) {
    unsigned char buf[512];
    

    if (read_sector(buf, 0) != 0) return -1;
    
    memcpy(pt, buf + 0x1BE, sizeof(PartitionTable) * 4);
    partition_start_byte = pt[0].start_sector * 512;
    
    if (read_sector(buf, pt[0].start_sector) != 0) return -1;
    memcpy(&bs, buf, sizeof(Fat16BootSector));
    
    fat_start_byte = partition_start_byte + (bs.reserved_sectors * bs.sector_size);
    root_dir_start_byte = fat_start_byte + (bs.number_of_fats * bs.fat_size_sectors * bs.sector_size);
    root_dir_sectors = ((bs.root_dir_entries * 32) + (bs.sector_size - 1)) / bs.sector_size;
    data_start_sector = pt[0].start_sector + bs.reserved_sectors + (bs.number_of_fats * bs.fat_size_sectors) + root_dir_sectors;
    
    return 0;
}

int find_in_directory(uint32_t dir_start_off, const char *target, Fat16Entry *result, uint32_t *out_sector, int *out_idx, int is_root) {
    unsigned char buf[512];
    uint32_t limit = is_root ? root_dir_sectors : bs.sectors_per_cluster;
    uint32_t start_sec = dir_start_off / bs.sector_size;

    for (uint32_t s = 0; s < limit; s++) {
        uint32_t sec_num = start_sec + s;
 
        if (read_sector(buf, sec_num) != 0) break;
        
        Fat16Entry *es = (Fat16Entry *)buf;
        for (int i = 0; i < bs.sector_size / 32; i++) {
            if (es[i].filename[0] == 0x00) return -ENOENT;
            if (es[i].filename[0] == 0xE5) continue;
            
            char name[13];
            format_fat_name(&es[i], name);
            if (strcasecmp(name, target) == 0) {
                if (result) *result = es[i];
                if (out_sector) *out_sector = sec_num;
                if (out_idx) *out_idx = i;
                return 0;
            }
        }
    }
    return -ENOENT;
}

int find_entry_by_path(const char *path, Fat16Entry *result, uint32_t *out_sector, int *out_idx) {
    if (strcmp(path, "/") == 0) {
        memset(result, 0, sizeof(Fat16Entry));
        result->attributes = 0x10;
        return 0;
    }

    char path_copy[256];
    strncpy(path_copy, path, 255);
    path_copy[255] = '\0';

    uint32_t current_dir_off = root_dir_start_byte;
    int is_root = 1;
    Fat16Entry current_entry;

    char *token = strtok(path_copy, "/");
    while (token != NULL) {
        uint32_t sec;
        int idx;
        
        int res = find_in_directory(current_dir_off, token, &current_entry, &sec, &idx, is_root);
        if (res != 0) return -ENOENT;

        char *next_token = strtok(NULL, "/");
        if (next_token == NULL) {
            if (result) *result = current_entry;
            if (out_sector) *out_sector = sec;
            if (out_idx) *out_idx = idx;
            return 0;
        }

        if (!(current_entry.attributes & 0x10)) return -ENOTDIR;

        current_dir_off = get_cluster_offset(current_entry.starting_cluster);
        is_root = (current_entry.starting_cluster == 0); 
        token = next_token;
    }

    return -ENOENT;
}

int fat_read_data(const Fat16Entry *entry, char *buf, size_t size, uint32_t offset) {
    if (offset >= entry->file_size) return 0;
    if (offset + size > entry->file_size) size = entry->file_size - offset;
    
    uint16_t cluster = entry->starting_cluster;
    uint32_t cluster_size = bs.sectors_per_cluster * bs.sector_size;
    
    while (offset >= cluster_size) {
        cluster = get_fat_entry(cluster);
        offset -= cluster_size;
        if (cluster >= 0xFFF8) return 0;
    }
    
    size_t total = 0;
    unsigned char tmp[512]; 
    
    while (total < size && cluster >= 2 && cluster < 0xFFF8) {
        uint32_t cluster_start_sec = get_cluster_offset(cluster) / bs.sector_size;
        
        for (int s = 0; s < bs.sectors_per_cluster; s++) {
            if (offset >= bs.sector_size) {
                offset -= bs.sector_size;
                continue;
            }
            
            read_sector(tmp, cluster_start_sec + s);
            size_t to_copy = bs.sector_size - offset;
            if (to_copy > (size - total)) to_copy = size - total;
            
            memcpy(buf + total, tmp + offset, to_copy);
            total += to_copy;
            offset = 0;
            
            if (total >= size) break;
        }
        if (total >= size) break;
        cluster = get_fat_entry(cluster);
    }
    return total;
}

int fat_delete_file(const char *path) {
    //serial_print("\nDeleting file: ");
    Fat16Entry entry;
    uint32_t sec;
    int idx;
    if (find_entry_by_path(path, &entry, &sec, &idx) != 0) return -ENOENT;
    
    uint16_t cluster = entry.starting_cluster;
    while (cluster != 0xFFFF && cluster >= 2 && cluster < 0xFFF0) {
        uint16_t next = get_fat_entry(cluster);
        set_fat_entry(cluster, 0x0000);
        cluster = next;
    }
    
    unsigned char buf[512];
    read_sector(buf, sec);
    ((Fat16Entry *)buf)[idx].filename[0] = 0xE5;
    write_sector(buf, sec);
    
    return 0;
}

void printTreeRecursive(uint32_t dir_off, int depth) {
    Fat16Entry *entry;
    unsigned char local_buffer[512];
    uint32_t start_sector = dir_off / bs.sector_size;
    
    for (uint32_t s = 0; s < bs.sectors_per_cluster; s++) {
        if (read_sector(local_buffer, start_sector + s) != 0) break;
        
        for (int i = 0; i < bs.sector_size / sizeof(Fat16Entry); i++) {
            entry = &((Fat16Entry *)local_buffer)[i];
            if (entry->filename[0] == 0x00) return;
            if (entry->filename[0] == 0xE5) continue;
            if (entry->filename[0] == '.' && (entry->filename[1] == ' ' || entry->filename[1] == '.')) continue;
            
            for (int d = 0; d < depth; d++) vga_print("  ");
            
            char namebuf[13];
            format_fat_name(entry, namebuf);
            
            if (entry->attributes & 0x10) {
                vga_print("["); vga_print(namebuf); vga_print("]\n");
                if (entry->starting_cluster != 0)
                    printTreeRecursive(get_cluster_offset(entry->starting_cluster), depth + 1);
            } else {
                vga_print(namebuf); vga_print("\n");
            }
        }
    }
}

void printTree() {
    printTreeRecursive(root_dir_start_byte, 0);
}

int find_free_cluster() {
    for (int i = 2; i < 65535; i++) {
        if (get_fat_entry(i) == 0x0000) return i;
    }
    return -1;
}

int find_empty_slot(uint32_t dir_off, uint32_t *out_sector, int *out_idx, int is_root) {
    unsigned char buf[512];
    uint32_t limit = is_root ? root_dir_sectors : bs.sectors_per_cluster;
    uint32_t start_sec = dir_off / bs.sector_size;

    for (uint32_t s = 0; s < limit; s++) {
        read_sector(buf, start_sec + s);
        Fat16Entry *es = (Fat16Entry *)buf;
        for (int i = 0; i < bs.sector_size / 32; i++) {
            if (es[i].filename[0] == 0x00 || es[i].filename[0] == 0xE5) {
                *out_sector = start_sec + s;
                *out_idx = i;
                return 0;
            }
        }
    }
    return -ENOSPC;
}

int fat_create_file(const char *path) {
    Fat16Entry dummy;
    if (find_entry_by_path(path, &dummy, NULL, NULL) == 0) return -EEXIST;

    char parent_path[256];
    char filename[13];
    
    const char *last_slash = strrchr(path, '/');
    if (!last_slash) return -EINVAL; 

    strncpy(filename, last_slash + 1, 12);
    filename[12] = '\0';

    size_t parent_len = last_slash - path;
    if (parent_len == 0) {
        strcpy(parent_path, "/");
    } else {
        strncpy(parent_path, path, parent_len);
        parent_path[parent_len] = '\0';
    }

    Fat16Entry parent_entry;
    uint32_t dir_off;
    int is_root_dir = 0;

    if (strcmp(parent_path, "/") == 0) {
        dir_off = root_dir_start_byte;
        is_root_dir = 1;
    } else {
        if (find_entry_by_path(parent_path, &parent_entry, NULL, NULL) != 0) {
            return -ENOENT; 
        }
        if (!(parent_entry.attributes & 0x10)) {
            return -ENOTDIR;
        }
        dir_off = get_cluster_offset(parent_entry.starting_cluster);
        is_root_dir = 0;
    }

    uint32_t dir_sector;
    int dir_idx;
    if (find_empty_slot(dir_off, &dir_sector, &dir_idx, is_root_dir) != 0) {
        return -ENOSPC; 
    }

    unsigned char buf[512];
    read_sector(buf, dir_sector);
    Fat16Entry *entry = &((Fat16Entry *)buf)[dir_idx];

    memset(entry, 0, sizeof(Fat16Entry));
    memset(entry->filename, ' ', 8);
    memset(entry->ext, ' ', 3);

    char *dot = strchr(filename, '.');
    int name_len = dot ? (dot - filename) : strlen(filename);
    for (int i = 0; i < name_len && i < 8; i++) entry->filename[i] = toupper(filename[i]);
    if (dot) {
        for (int i = 0; i < 3 && dot[i+1]; i++) entry->ext[i] = toupper(dot[i+1]);
    }

    entry->attributes = 0x20;
    entry->starting_cluster = 0; 
    entry->file_size = 0;
    
    entry->modify_date = 0;
    entry->modify_time = 0;

    write_sector(buf, dir_sector);
    return 0;
}

int fat_write_data(const char *path, const char *buf, size_t size, uint32_t offset) {
    Fat16Entry entry;
    uint32_t entry_sector;
    int entry_idx;
    
    if (find_entry_by_path(path, &entry, &entry_sector, &entry_idx) != 0) return -ENOENT;

    uint32_t cluster_size = bs.sectors_per_cluster * bs.sector_size;
    
    if (entry.starting_cluster == 0) {
        int new_c = find_free_cluster();
        if (new_c < 0) return -ENOSPC;
        set_fat_entry(new_c, 0xFFFF);
        entry.starting_cluster = new_c;
    }
    serial_print("\nstarting writin: ");
   
    uint16_t curr_c = entry.starting_cluster;
    uint32_t walk = offset;
    
    while (walk >= cluster_size) {
        
        uint16_t next = get_fat_entry(curr_c);
        if (next >= 0xFFF8) { 
            int new_c = find_free_cluster();
            if (new_c < 0) return -ENOSPC;
            set_fat_entry(curr_c, new_c);
            set_fat_entry(new_c, 0xFFFF);
            next = new_c;
        }
        curr_c = next;
        walk -= cluster_size;
          
    }

    unsigned char sector_buf[512];
    uint32_t target_sec = (get_cluster_offset(curr_c) / bs.sector_size) + (walk / bs.sector_size);
    uint32_t sec_off = walk % bs.sector_size;
    
    read_sector(sector_buf, target_sec);
    size_t to_write = (size < (512 - sec_off)) ? size : (512 - sec_off);
    memcpy(sector_buf + sec_off, buf, to_write);
    write_sector(sector_buf, target_sec);
    serial_print("\nstructural print:\n ");
    if (offset + size > entry.file_size) {
        entry.file_size = offset + size;
    }

    unsigned char dir_buf[512];
    read_sector(dir_buf, entry_sector);
    ((Fat16Entry *)dir_buf)[entry_idx] = entry;
    write_sector(dir_buf, entry_sector);

    return to_write;
}