; kernel/arch/isr.S

extern isr_handler

; Makro pro přerušení BEZ chybového kódu (přidáme dummy 0)
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                 ; Zakážeme další přerušení
    push dword 0        ; Dummy chybový kód (zarovnání stacku na 4 byty)
    push dword %1       ; Číslo přerušení
    jmp isr_common_stub
%endmacro

; Makro pro přerušení S chybovým kódem (CPU už ho dalo na zásobník)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli                 ; Zakážeme další přerušení
    push dword %1       ; Číslo přerušení
    jmp isr_common_stub
%endmacro

; Definice prvních několika výjimek CPU (0 až 31)
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
; ... (můžete doplnit až do 31) ...

; Náš společný stub, kam skočí všechna ISR
isr_common_stub:
    pusha               ; Uloží edi, esi, ebp, esp, ebx, edx, ecx, eax

    ; Volání C funkce
    push esp            ; Předáme ukazatel na zásobník jako argument (registers_t *r)
    call isr_handler
    add esp, 4          ; Úklid argumentu po návratu z C funkce

    popa                ; Obnoví původní registry
    add esp, 8          ; Uklidí číslo přerušení a chybový kód ze zásobníku
    sti                 ; Znovu povolí přerušení
    iret                ; Instrukce 'Interrupt Return' - návrat do přerušeného kódu


; ============================================
; TIMER IRQ0 STUB
; ============================================
global irq0_stub
extern timer_callback

irq0_stub:
    pusha               ; Save all GP registers
    
    ; Optional: Save current ESP into the current thread's TCB here
    ; so the scheduler knows where to resume this thread later.
    
    call timer_callback ; This calls scheduler_tick -> gthr_yield
    
    ; When the scheduler eventually picks this thread again, 
    ; execution resumes here.
    
    popa                ; Restore GP registers
    iret                ; Resume the thread[cite: 1]

global isr_dummy
isr_dummy:
    iret        ; Prostě se rovnou vrať bez dělání čehokoliv