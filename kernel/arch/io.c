#include "io.h"

unsigned char inb(unsigned short port)
{
    unsigned char result;
    __asm__ __volatile__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}



unsigned short inw(unsigned short port)
{
    unsigned short result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}


void outw(unsigned short port, unsigned short value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}