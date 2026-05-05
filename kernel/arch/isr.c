#include "isr.h"
// Zde si includujte své funkce pro výpis na obrazovku (např. kprintf)

/* Toto je pole chybových hlášek pro standardní CPU výjimky (0-31) */
const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t *r) {
    // Zatím zachytáváme jen CPU výjimky (0-31)
    if (r->int_no < 32) {
        // Tady by se normálně zavolalo panic() nebo výpis chyby
        // kprintf("Vyjimecny stav CPU: %s (Cislo: %d)\n", exception_messages[r->int_no], r->int_no);
        
        // Zastavení systému při CPU výjimce (tzv. kernel panic)
        while(1) {
            __asm__ __volatile__("cli; hlt");
        }
    }
}