#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "fat.h"

static int fat_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));

    Fat16Entry entry;
    int res = find_entry_by_path(path, &entry, NULL, NULL);
    if (res != 0) return res;

    if (entry.attributes & 0x10) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = entry.file_size;
    }
    return 0;
}

static int fat_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;

    Fat16Entry dir_entry;
    if (find_entry_by_path(path, &dir_entry, NULL, NULL) != 0) return -ENOENT;
    if (!(dir_entry.attributes & 0x10)) return -ENOTDIR;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    uint32_t dir_off;
    uint32_t limit;
    int is_root = 0;

    if (strcmp(path, "/") == 0) {
        dir_off = root_dir_start_byte;
        limit = root_dir_sectors;
        is_root = 1;
    } else {
        dir_off = get_cluster_offset(dir_entry.starting_cluster);
        limit = bs.sectors_per_cluster;
    }

    unsigned char sector_buffer[512];
    for (uint32_t s = 0; s < limit; s++) {
        if (read_sector(sector_buffer, (dir_off / bs.sector_size) + s) <= 0) break;
        Fat16Entry *entries = (Fat16Entry *)sector_buffer;
        
        for (int i = 0; i < bs.sector_size / sizeof(Fat16Entry); i++) {
            if (entries[i].filename[0] == 0x00) return 0;
            if (entries[i].filename[0] == 0xE5) continue;
            if (entries[i].attributes & 0x08) continue; 

            char name[13];
            format_fat_name(&entries[i], name);
            filler(buf, name, NULL, 0, 0);
        }
    }
    return 0;
}

static int fat_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    Fat16Entry entry;
    if (find_entry_by_path(path, &entry, NULL, NULL) != 0) return -ENOENT;
    
    return fat_read_data(&entry, buf, size, offset);
}

static int fat_unlink(const char *path) {
    return fat_delete_file(path);
}

static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) mode; (void) fi;
    return fat_create_file(path);
}

static int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    return fat_write_data(path, buf, size, offset);
}

static int do_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
  
    return 0;
}

static int fat_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
    (void) fi;
    (void) tv;

    return 0;
}

static struct fuse_operations fat_oper = {
    .getattr    = fat_getattr,
    .readdir    = fat_readdir,
    .read       = fat_read,
    .unlink     = fat_unlink,
    .create     = do_create,   
    .write      = do_write,    
    .truncate   = do_truncate, 
    .utimens    = fat_utimens,
};

int main(int argc, char *argv[]) {
    
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <image_file> <mountpoint> [fuse_options]\n", argv[0]);
        return 1;
    }

    char *image_path = argv[1];
    
   
    if (fat_init(image_path) != 0) {
        fprintf(stderr, "Failed to initialize FAT filesystem from %s\n", image_path);
        return 1;
    }

   
    argv[1] = argv[0];
    
    int fuse_res = fuse_main(argc - 1, argv + 1, &fat_oper, NULL);
    
    fat_close();
    return fuse_res;
}