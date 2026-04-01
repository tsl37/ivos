
CC = gcc
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Ikernel/arch -Ikernel/drivers -Ikernel/cli -Ikernel/kernel
LD = ld
LDFLAGS = -m elf_i386

C_SOURCES = $(wildcard kernel/kernel/*.c kernel/drivers/*.c kernel/arch/*.c kernel/cli/*.c)
OBJ = $(C_SOURCES:.c=.o)

all: os.img helloworld.bin

helloworld.bin: helloworld.c
	$(CC) $(CFLAGS) -c helloworld.c -o helloworld.o
	$(LD) $(LDFLAGS) -T link.ld helloworld.o -o helloworld.elf
	objcopy -O binary helloworld.elf helloworld.bin

boot.o: boot.asm
	nasm -f elf32 boot.asm -o boot.o

os.img: boot.o $(OBJ) helloworld.bin
	$(LD) $(LDFLAGS) -o os.img -Ttext 0x7C00 --oformat binary boot.o $(OBJ)
	truncate -s 102400 os.img 
	dd if=helloworld.bin of=os.img bs=512 seek=30 conv=notrunc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	qemu-system-i386 -drive format=raw,file=os.img -serial stdio

clean:
	rm -f *.bin *.o *.elf os.img $(OBJ)