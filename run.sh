nasm boot.asm -o boot.bin
nasm kernel.asm -o kernel.bin
cat boot.bin kernel.bin > os.img
truncate -s 10k os.img
qemu-system-i386 -drive format=raw,file=os.img