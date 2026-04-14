#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include "string.h" 

// MBR Partition Table (16 Bytů)
typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    uint32_t start_sector;
    uint32_t length_sectors;
} __attribute((packed)) PartitionTable;

// Boot Sektor pro FAT16
typedef struct {
    unsigned char jmp[3];
    char oem[8];
    uint16_t sector_size;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_short;
    uint8_t media_descriptor;
    uint16_t fat_size_sectors;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_int;
    uint8_t drive_number;
    uint8_t current_head;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    uint16_t boot_sector_signature;
} __attribute((packed)) Fat16BootSector;

// Záznam adresáře FAT16 (32 Bytů)
typedef struct {
    unsigned char filename[8];
    unsigned char ext[3];
    uint8_t attributes;
    uint8_t reserved[10];
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t starting_cluster;
    uint32_t file_size;
} __attribute((packed)) Fat16Entry;

// Globální proměnné
extern Fat16BootSector bs;
extern uint32_t root_dir_sectors;
extern uint32_t root_dir_start_byte;


int fat_init(const char *image_path);
void fat_close(void);

// Nízkoúrovňové operace sektorů
int read_sector(void *buf, uint32_t sector);
int write_sector(void *buf, uint32_t sector);
uint32_t get_cluster_offset(uint16_t n);

// Operace s položkami (cesty, jména, výpisy)
void format_fat_name(const Fat16Entry *entry, char *out);
int find_entry_by_path(const char *path, Fat16Entry *result, uint32_t *out_sector, int *out_idx);
void printTree(void);

// V/V operace souborů (nahrazeno off_t za uint32_t)
int fat_create_file(const char *path);
int fat_delete_file(const char *path);
int fat_read_data(const Fat16Entry *entry, char *buf, size_t size, uint32_t offset);
int fat_write_data(const char *path, const char *buf, size_t size, uint32_t offset);

#endif