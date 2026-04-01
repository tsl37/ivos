CC = gcc
LD = ld
ASM = nasm
OBJCOPY = objcopy

# Parametry kompilátoru
CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -Ikernel/drivers -Ikernel/arch -Ikernel/cli -Ifat -Wall -Wextra

# Seznam zdrojových souborů C
C_SOURCES = kernel/kernel/main.c \
            kernel/cli/cli.c \
            kernel/cli/string.c \
            kernel/drivers/vga.c \
            kernel/drivers/serial.c \
            kernel/drivers/keyboard.c \
            kernel/drivers/ide.c \
            kernel/arch/io.c \
            fat/fat.c

OBJ_FILES = $(C_SOURCES:.c=.o)

all: sd.img

# 1. Překlad boot.asm do formátu ELF (aby linker poznal extern start_kernel)
boot.o: boot.asm
	$(ASM) -f elf32 boot.asm -o boot.o

# 2. Slinkování bootloaderu a kernelu do jednoho souboru
os.bin: boot.o $(OBJ_FILES)
	# Důležité: boot.o musí být první, aby byl na začátku výsledného souboru!
	$(LD) -m elf_i386 -T link.ld -o os.elf boot.o $(OBJ_FILES)
	$(OBJCOPY) -O binary os.elf os.bin

# 3. Zápis do sd.img (rozdělení spojeného souboru)
sd.img: os.bin
	@echo "==== Zápis zavaděče a kernelu do sd.img ===="
	
	# Zápis MBR (jen prvních 446 B z os.bin, aby se nepřepsala partition tabulka)
	dd if=os.bin of=sd.img bs=1 count=446 conv=notrunc
	
	# Zápis kernelu (od 2. sektoru na disku). 
	# skip=1: přeskočíme prvních 512 B (zavaděč) uvnitř os.bin
	# seek=1: začneme zapisovat až na LBA 1 uvnitř sd.img
	dd if=os.bin of=sd.img bs=512 skip=1 seek=1 conv=notrunc
	
	@echo "==== Obraz disku je připraven ===="

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	qemu-system-i386 -hda sd.img -serial stdio

clean:
	rm -f $(OBJ_FILES) boot.o os.elf os.bin