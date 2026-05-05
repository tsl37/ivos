#include "vga.h"
#include "keyboard.h"
#include "serial.h"
#include "ide.h"
#include "cli.h"
#include "fat.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"

extern unsigned char _bss_start;
extern unsigned char _bss_end;
extern void isr_dummy();
extern void irq0_stub(); // Váš časovač




void start_kernel()
{
   unsigned char *bss = &_bss_start;
    while (bss < &_bss_end) {
        *bss++ = 0;
    }
    const char *logo = " \xB0\xDB\xDB\xDB\xDB\xDB\xDB  \xB0\xDB\xDB                   \xB0\xDB\xDB\xDB\xDB\xDB\xDB    \xB0\xDB\xDB\xDB\xDB\xDB\xDB   \n\xB0\xDB\xDB   \xB0\xDB\xDB \xB0\xDB\xDB                   \xB0\xDB\xDB   \xB0\xDB\xDB  \xB0\xDB\xDB   \xB0\xDB\xDB  \n\xB0\xDB\xDB       \xB0\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB   \xB0\xDB\xDB\xDB\xDB\xDB\xDB   \xB0\xDB\xDB     \xB0\xDB\xDB \xB0\xDB\xDB       \n\xB0\xDB\xDB       \xB0\xDB\xDB    \xB0\xDB\xDB       \xB0\xDB\xDB  \xB0\xDB\xDB     \xB0\xDB\xDB  \xB0\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB  \n\xB0\xDB\xDB       \xB0\xDB\xDB    \xB0\xDB\xDB \xB0\xDB\xDB\xDB\xDB\xDB\xDB\xDB  \xB0\xDB\xDB     \xB0\xDB\xDB         \xB0\xDB\xDB \n \xB0\xDB\xDB   \xB0\xDB\xDB \xB0\xDB\xDB    \xB0\xDB\xDB \xB0\xDB\xDB   \xB0\xDB\xDB   \xB0\xDB\xDB   \xB0\xDB\xDB   \xB0\xDB\xDB   \xB0\xDB\xDB  \n  \xB0\xDB\xDB\xDB\xDB\xDB\xDB  \xB0\xDB\xDB    \xB0\xDB\xDB  \xB0\xDB\xDB\xDB\xDB\xDB\xB0\xDB\xDB   \xB0\xDB\xDB\xDB\xDB\xDB\xDB     \xB0\xDB\xDB\xDB\xDB\xDB\xDB   \n\n";
    vga_print(logo);
    serial_init();
  
    
     serial_print("ChaOS booting...\n"); 

    init_interrupts();
    // 3. Inicializace časovače (např. 100 Hz = 10ms ticks)
    timer_init(100);
     __asm__ __volatile__("sti");
    // 4. Povolení hardwarových přerušení (instrukce STI)

    vga_print("Welcome to ChaOS!\n");
   
   cli_loop();
}
void init_interrupts() {
    // 1. Přemapování PIC musí být PRVNÍ. 
    // Standardně jsou IRQ na 0-7, což koliduje s CPU výjimkami.
    // Musíme je posunout na 32-47.
    pic_remap();
    idt_load();
    // 2. Naplnění tabulky IDT dummy handlerem (bezpečnostní síť).
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_dummy);
    }
    
    // 3. Registrace časovače na přerušení 32 (IRQ0).
    idt_set_gate(32, (uint32_t)irq0_stub);
    
    // 4. TEPRVE TEĎ načteme IDT do procesoru (instrukce lidt)[cite: 1].
  
}                                                        