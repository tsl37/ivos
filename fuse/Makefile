CC = gcc
CFLAGS = -Wall -Wextra -g -O0
FUSE_CFLAGS = $(shell pkg-config --cflags fuse3)
FUSE_LIBS = $(shell pkg-config --libs fuse3)

# Targets
FAT_LIB = ../fat/fat_lib.c ../fat/fat.h
FATFS = fatfs.c $(FAT_LIB)

# Default target
all: fatfs

# Build FUSE filesystem
fatfs: $(FATFS)
	$(CC) $(CFLAGS) $(FUSE_CFLAGS) -I.. -o $@ $< $(FUSE_LIBS)

# Run in foreground with debug output
run: fatfs
	mkdir -p /tmp/fatfs_mount
	./fatfs -f -d --image=../fat/sd.img /tmp/fatfs_mount

# Unmount if mounted
unmount:
	fusermount3 -u /tmp/fatfs_mount 2>/dev/null || true

# Clean
clean: unmount
	rm -f fatfs fatfs.o

.PHONY: all run unmount clean
