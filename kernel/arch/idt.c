#include "idt.h"

/* Vytvoření pole pro 256 možných přerušení a pointeru pro lidt */
idt_entry_t idt[256];
idt_ptr_t idt_ptr;

void idt_set_gate(int n, uint32_t handler) {
    // Spodních 16 bitů adresy
    idt[n].base_lo = handler & 0xFFFF;
    
    // Segment selector pro kernel code segment (dle zadání 0x08)
    idt[n].sel = 0x08; 
    
    // Tento bajt musí být vynulován
    idt[n].always0 = 0; 
    
    // Flags: 0x8E
    // 1000 1110 v binární soustavě:
    // Bit 7: Present (1) - handler je přítomen v paměti
    // Bity 5-6: DPL (00) - Ring 0 (kernel úroveň)
    // Bit 4: 0
    // Bity 0-3: Type (1110) - 32-bit Interrupt Gate
    idt[n].flags = 0x8E; 
    
    // Horních 16 bitů adresy
    idt[n].base_hi = (handler >> 16) & 0xFFFF;
}

void idt_load() {
    // Limit je velikost pole v bajtech mínus jedna
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    // Base je adresa prvního prvku pole
    idt_ptr.base = (uint32_t)&idt;

    // Vynulování celé tabulky před registrací handlerů (dobrá praxe)
    // Pokud zde nemáte memsets(), můžete použít jednoduchý for cyklus.
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0);
    }

    // Načtení IDT pointeru do procesoru pomocí inline assembleru
    __asm__ __volatile__("lidt %0" : : "m" (idt_ptr));
}