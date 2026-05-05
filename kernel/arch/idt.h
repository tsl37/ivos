#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Struktura jedné položky v IDT (Gate Descriptor) */
struct idt_entry_struct {
    uint16_t base_lo;   // Spodních 16 bitů adresy handleru (ISR)
    uint16_t sel;       // Segment selector (Kernel Code Segment)
    uint8_t  always0;   // Vyhrazeno, musí být vždy 0
    uint8_t  flags;     // Příznaky (Present, DPL, Type)
    uint16_t base_hi;   // Horních 16 bitů adresy handleru
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

/* Struktura předávaná instrukci lidt */
struct idt_ptr_struct {
    uint16_t limit;     // Velikost celé tabulky IDT v bajtech mínus 1
    uint32_t base;      // Počáteční adresa tabulky IDT
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

// Funkce pro registraci handleru
void idt_set_gate(int n, uint32_t handler);

// Funkce pro inicializaci a načtení IDT
void idt_load();

#endif // IDT_H