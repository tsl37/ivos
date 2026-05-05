#include "io.h" // Obsahuje outb, inb

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

void pic_remap() {
    unsigned char a1, a2;
    
    a1 = inb(PIC1_DATA); // Uložení původních masek
    a2 = inb(PIC2_DATA);
    
    // Inicializační sekvence (ICW1)
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    
    // ICW2: Nové offsety (IRQ0 bude 32, IRQ8 bude 40)
    outb(PIC1_DATA, 32);  
    outb(PIC2_DATA, 40);
    
    // ICW3: Nastavení kaskády (Master/Slave)
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    
    // ICW4: 8086/88 (MCS-80/85) mód
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Obnova masek
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}