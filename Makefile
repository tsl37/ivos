CC = gcc
LD = ld
ASM = nasm
OBJCOPY = objcopy

# Parametry kompilátoru
CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pie -Ikernel/drivers -Ikernel/scheduler -Ikernel/arch -Ikernel/cli -Wall -Wextra

# Seznam všech zdrojových souborů C
C_SOURCES = kernel/kernel/main.c \
            kernel/cli/cli.c \
            kernel/cli/string.c \
            kernel/drivers/vga.c \
            kernel/drivers/serial.c \
            kernel/drivers/keyboard.c \
            kernel/drivers/ide.c \
            kernel/arch/io.c \
            kernel/drivers/fat.c \
			kernel/scheduler/scheduler.c

# Z C souborů vygenerujeme názvy cílových objektů (.o)
OBJ_FILES = $(C_SOURCES:.c=.o)

# Hlavní cíl se nyní jmenuje os.img
all: os.img

# 1. Překlad Kernelu (vytažení flat/raw binárky z ELF)
kernel.bin: $(OBJ_FILES)
	$(LD) -m elf_i386 -T link.ld -o kernel.elf $(OBJ_FILES)
	$(OBJCOPY) -O binary kernel.elf kernel.bin

# 2. Překlad Zavaděče
boot.bin: boot/mbr.asm
	$(ASM) -Iboot/ -f bin boot/mbr.asm -o boot.bin

# 3. Vytvoření finálního OS obrazu spojením sd.img, zavaděče a kernelu
os.img: boot.bin kernel.bin sd.img
	@echo "==== Vytvářím finální os.img ===="
	
	# Krok A: Zkopírujeme čistý FAT16 disk jako základ (záloha)
	cp sd.img os.img
	
	# Krok B: Zápis zavaděče (zachování partition tabulky - kopírujeme jen prvních 446 B)
	dd if=boot.bin of=os.img bs=1 count=446 conv=notrunc
	
	# Krok C: Zápis kernelu (zápis od sektoru 2 / přeskočení 1 sektoru na disku)
	dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc
	
	@echo "==== Obraz disku os.img je připraven ===="

# Pravidlo pro kompilaci všech C souborů do objektových souborů
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cíl pro spuštění v emulátoru (nyní spouštíme os.img místo sd.img)
run: all
	qemu-system-i386 -drive format=raw,file=os.img -serial stdio

# Úklid
clean:
	rm -f $(OBJ_FILES) kernel.elf kernel.bin boot.bin boot.o os.bin os.elf os.img