CC = gcc
LD = ld
ASM = nasm
OBJCOPY = objcopy

CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pie -Ikernel/drivers -Ikernel/scheduler -Ikernel/arch -Ikernel/cli -Wall -Wextra

SCHED_SOURCES = kernel/scheduler/scheduler.c
SCHED_ASM = kernel/scheduler/gthr_switch.S


C_SOURCES = kernel/kernel/main.c \
            kernel/cli/cli.c \
            kernel/cli/string.c \
            kernel/drivers/vga.c \
            kernel/drivers/serial.c \
            kernel/drivers/keyboard.c \
            kernel/drivers/ide.c \
            kernel/arch/io.c \
            kernel/arch/idt.c \
            kernel/arch/isr.c \
            kernel/arch/pic.c \
            kernel/arch/timer.c \
            kernel/drivers/fat.c 
          

ASM_SOURCES = kernel/arch/isr.S

# Změna: C objekty budou .o, ASM objekty budou .asm.o
C_OBJS = $(C_SOURCES:.c=.o) \
            $(SCHED_SOURCES:.c=.o) \
            $(SCHED_ASM:.S=.o)
ASM_OBJS = $(ASM_SOURCES:.S=.asm.o)

OBJ_FILES = $(C_OBJS) $(ASM_OBJS)

all: os.img

kernel.bin: $(OBJ_FILES)
	$(LD) -m elf_i386 -T link.ld -o kernel.elf $(OBJ_FILES)
	$(OBJCOPY) -O binary kernel.elf kernel.bin

boot.bin: boot/mbr.asm
	$(ASM) -Iboot/ -f bin boot/mbr.asm -o boot.bin

os.img: boot.bin kernel.bin sd.img
	cp sd.img os.img
	dd if=boot.bin of=os.img bs=1 count=446 conv=notrunc
	dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc

# Pravidlo pro C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nové pravidlo pro Assembler
%.asm.o: %.S
	$(ASM) -f elf32 $< -o $@


# Změňte 'as' na 'nasm'
kernel/scheduler/gthr_switch.o: kernel/scheduler/gthr_switch.S
	nasm -f elf32 -o $@ $<

run: all
	qemu-system-i386 -drive format=raw,file=os.img -serial stdio

clean:
	rm -f $(C_OBJS) $(ASM_OBJS) kernel.elf kernel.bin boot.bin os.bin os.elf os.img