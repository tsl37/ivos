#include "vga.h"
#include "keyboard.h"
#include "serial.h"
#include "ide.h"
#include "cli.h"
#include "fat.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "scheduler.h"

extern unsigned char _bss_start;
extern unsigned char _bss_end;
extern void isr_dummy();
extern void irq0_stub(); // Váš časovač
void test_task_1();
void test_task_2();


//extremely iportant for the start kerrnel function to be the first function in the file, otherwise the linker will not set it as the entry point and the kernel will not boot
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

    // 1. Nastavení IDT a PIC (Přerušení ještě NEJSOU povolena)
    init_interrupts();
    serial_print("Interrupts initialized.\n");
    gt_init();
    // 2. Nastavení frekvence časovače
    timer_init(100);
    serial_print("Timer initialized.\n");

    // 3. Inicializace scheduleru (MUSÍ BÝT PŘED STI)
     
    serial_print("Scheduler initialized.\n");

    // 4. Vytvoření testovacích vláken
    gt_create(test_task_1, 5); 
    gt_create(test_task_2, 5); 
    serial_print("Test threads created.\n");

    vga_print("Welcome to ChaOS!\n");

    // 5. TEPRVE TEĎ povolíme přerušení. Nyní, když timer "tikne", 
    //    scheduler už bude vědět, co má dělat.
    __asm__ __volatile__("sti");
    serial_print("Interrupts enabled! Entering CLI loop...\n");

    // 6. Hlavní smyčka
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



void test_task_1() {
    while (1) {
        // Replace kprint/vga_print with your actual text printing function
        serial_print("Task 1 is running...\n"); 
        
        // Add a simple delay loop to slow down the output
        for (volatile int i = 0; i < 10000000; i++); 

        // If your scheduler is cooperative, explicitly yield:
        // scheduler_yield(); or gthr_yield();
    }
}

void test_task_2() {
    while (1) {
        serial_print("Task 2 is running...\n");
        
        for (volatile int i = 0; i < 10000000; i++); 

        // scheduler_yield(); or gthr_yield();
    }
}