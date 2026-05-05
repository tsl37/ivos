ENTRY(start_kernel)
OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
    /* 1. Nastavení počáteční adresy, kam bootloader načítá kernel z disku */
    . = 0x8000;

    /* 2. Kódová sekce */
    .text : {
        /* DŮLEŽITÉ: Vynutíme, aby kód ze souboru main.o byl vložen na úplný 
           začátek. Tím zajistíme, že bootloader skočí rovnou na start_kernel() */
        *main.o(.text)
        
        /* Poté vložíme kód ze všech ostatních .o souborů */
        *(.text)
    }

    /* 3. Data určená pouze pro čtení (např. vaše textové řetězce a konstanty) */
    .rodata : {
        *(.rodata)
    }

    /* 4. Inicializované globální a statické proměnné */
    .data : {
        *(.data)
    }

   /* 5. Neinicializované globální proměnné a pole (automaticky vynulované) */
    .bss : {
        _bss_start = .;        /* Define symbol for start of BSS */
        *(.COMMON)
        *(.bss)
        _bss_end = .;          /* Define symbol for end of BSS */
    }
}