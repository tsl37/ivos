
TARGET = fat_fuse

CC = gcc
CFLAGS = -Wall -g $(shell pkg-config fuse3 --cflags)
LIBS = $(shell pkg-config fuse3 --libs)


SRCS = fat.c fat_fuse.c
OBJS = $(SRCS:.c=.o)
DEPS = fat.h


IMG = sd.img
MNT = mnt

.PHONY: all clean run setup_mnt unmount


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@


setup_mnt:
	@mkdir -p $(MNT)


run: all setup_mnt
	./$(TARGET) $(IMG) $(MNT) -f -d


unmount:
	fusermount3 -u $(MNT) || true

clean:
	rm -f $(TARGET) $(OBJS)
	@rmdir $(MNT) 2>/dev/null || true