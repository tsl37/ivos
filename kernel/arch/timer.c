#include "io.h"

uint32_t tick = 0;
extern void scheduler_tick(); // Deklarace vaší funkce z plánovače

void timer_callback() {
    tick++;
    
    // Zde napojíme plánovač! 
    scheduler_tick();
    
    // End Of Interrupt (EOI) pro PIC
    // Pokud bychom to neudělali, PIC už nám nepošle další přerušení.
    outb(0x20, 0x20); 
}

void timer_init(uint32_t frequency) {
    // Hardwarová frekvence krystalu je 1193180 Hz
    uint32_t divisor = 1193180 / frequency;

    // Pošleme command byte (Nastavíme kanál 0, mode 3 - square wave, lobyte/hibyte)
    outb(0x43, 0x36);

    // Pošleme dělitel (divisor)
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );
    
    outb(0x40, l);
    outb(0x40, h);
}