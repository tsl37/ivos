#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/* Struktura reprezentující stav registrů uložených na zásobníku */
typedef struct {
    // Uloženo instrukcí 'pusha'
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; 
    
    // Uloženo ručně v ASM (číslo přerušení a případný chybový kód)
    uint32_t int_no, err_code; 
    
    // Uloženo automaticky procesorem při vyvolání přerušení
    uint32_t eip, cs, eflags, useresp, ss; 
} registers_t;

// Hlavní C handler, který bude volán z assembleru
void isr_handler(registers_t *r);

#endif // ISR_H