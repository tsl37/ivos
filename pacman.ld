SECTIONS
{
    . = 0x20000;
    
    .text : {
        *(.text.entry)
        *(.text*)
    }
    
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) *(COMMON) }
}