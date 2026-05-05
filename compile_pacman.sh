gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c pacman.c -o pacman.o

ld -m elf_i386 -T pacman.ld pacman.o -o pacman.elf

objcopy -O binary -j .text -j .rodata -j .data pacman.elf pacman.bin

mcopy -i sd.img@@1048576 pacman.bin ::/