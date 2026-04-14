#include "io.h"
#include <stdint.h>


#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_COMM_STAT    0x1F7



int ide_wait() {
    uint32_t timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_COMM_STAT);
    
        if (status & 0x01) return -1; 
        
        if (!(status & 0x80) && (status & 0x08)) {
            return 0; 
        }
    }
    serial_print("\nIDE timed out ");
    return -2;
}

int ide_read_sector(uint32_t lba, void *buffer) {
    
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    
    outb(ATA_PRIMARY_SECCOUNT, 1);
    
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    
    outb(ATA_PRIMARY_COMM_STAT, 0x20);
 
    serial_print("\nIDE read command issued ");
    serial_print_hex(lba);
    ide_wait();
    
 
    uint16_t *buf = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_PRIMARY_DATA);
    }
    
    return 0; 
}

int ide_write_sector(uint32_t lba, const void *buffer) {
    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, 1);
    
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    
   
    outb(ATA_PRIMARY_COMM_STAT, 0x30);
    serial_print("\nIDE write command issued ");
    serial_print_hex(lba);
    ide_wait();
 
    const uint16_t *buf = (const uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_DATA, buf[i]);
    }
    
  
    outb(ATA_PRIMARY_COMM_STAT, 0xE7);
    
    return 0; 
}