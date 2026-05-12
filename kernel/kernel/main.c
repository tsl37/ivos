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

    init_interrupts();
   // 3. Inicializace časovače (např. 100 Hz = 10ms ticks)
    timer_init(100);
     __asm__ __volatile__("sti");
    // 4. Povolení hardwarových přerušení (instrukce STI)

    vga_print("Welcome to ChaOS!\n");

    gt_init(); // Inicializace scheduleru a první vlákno (kernel thread)
    gt_create(test_task_1, 5); // Vytvoření testovacího vlákna 1 s prioritou 5
    gt_create(test_task_2, 5); // Vytvoření testovacího
    gt_schedule(); // Spuštění plánovače, který přepne na první vlákno
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
        vga_print("Task 1 is running...\n"); 
        
        // Add a simple delay loop to slow down the output
        for (volatile int i = 0; i < 10000000; i++); 

        // If your scheduler is cooperative, explicitly yield:
        // scheduler_yield(); or gthr_yield();
    }
}

void test_task_2() {
    while (1) {
        vga_print("Task 2 is running...\n");
        
        for (volatile int i = 0; i < 10000000; i++); 

        // scheduler_yield(); or gthr_yield();
    }
}