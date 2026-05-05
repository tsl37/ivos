.extern isr_handler

/* Makro pro přerušení BEZ chybového kódu (přidáme dummy 0) */
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli                 ; Zakážeme další přerušení
    push $0             ; Dummy chybový kód
    push $\num          ; Číslo přerušení
    jmp isr_common_stub
.endm

/* Makro pro přerušení S chybovým kódem (CPU už ho dalo na zásobník) */
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    cli                 ; Zakážeme další přerušení
    push $\num          ; Číslo přerušení
    jmp isr_common_stub
.endm

/* Definice prvních několika výjimek CPU (0 až 31) */
ISR_NOERRCODE 0  ; Div by zero
ISR_NOERRCODE 1  ; Debug
ISR_NOERRCODE 2  ; Non-maskable interrupt
ISR_NOERRCODE 3  ; Breakpoint
ISR_NOERRCODE 4  ; Into detected overflow
ISR_NOERRCODE 5  ; Out of bounds
ISR_NOERRCODE 6  ; Invalid opcode
ISR_NOERRCODE 7  ; No coprocessor
ISR_ERRCODE   8  ; Double fault (má error code)
ISR_NOERRCODE 9  ; Coprocessor segment overrun
ISR_ERRCODE   10 ; Bad TSS (má error code)
ISR_ERRCODE   11 ; Segment not present (má error code)
ISR_ERRCODE   12 ; Stack fault (má error code)
ISR_ERRCODE   13 ; General protection fault (má error code)
ISR_ERRCODE   14 ; Page fault (má error code)
ISR_NOERRCODE 15 ; Unknown interrupt
/* ... (můžete doplnit až do 31) ... */

/* Náš společný stub, kam skočí všechna ISR */
isr_common_stub:
    pusha               ; Uloží edi, esi, ebp, esp, ebx, edx, ecx, eax

    ; Volání C funkce
    push %esp           ; Předáme ukazatel na zásobník jako argument (registers_t *r)
    call isr_handler
    add $4, %esp        ; Úklid argumentu po návratu z C funkce

    popa                ; Obnoví původní registry
    add $8, %esp        ; Uklidí číslo přerušení a chybový kód ze zásobníku
    sti                 ; Znovu povolí přerušení
    iret                ; Instrukce 'Interrupt Return' - návrat do přerušeného kódu