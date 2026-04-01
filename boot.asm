[bits 16]


start:
    ; Initialize segments and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Save boot drive ID provided by BIOS
    mov [BOOT_DRIVE], dl   

    ; Print status message
    mov si, msg_loading
    call print_string

    ; Load kernel from disk
    mov bx, 0x7E00          ; Load destination
    mov al, 40             ; Number of sectors
    mov ch, 0               ; Cylinder 0
    mov dh, 0               ; Head 0
    mov cl, 2               ; Sector 2 (starts after bootloader)
    mov dl, [BOOT_DRIVE]
    mov ah, 0x02            ; BIOS read function
    int 0x13
    jc disk_error           ; Jump if carry flag set (error)

    ; Switch to Protected Mode
    cli                     ; Disable interrupts
    lgdt [gdt_descriptor]   ; Load GDT
    mov eax, cr0
    or eax, 0x1             ; Set Protected Mode bit
    mov cr0, eax
    
    jmp 0x08:init_pm        ; Far jump to clear pipeline and switch segment

[bits 32]
init_pm:
    ; Update data segments for 32-bit
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Setup new stack in safe memory
    mov ebp, 0x90000
    mov esp, ebp

    [extern start_kernel] ; Define calling point. Must have same name as kernel.c 'main' function
    call start_kernel ; Calls the C function. The linker will know where it is placed in memory
    jmp $

[bits 16]
print_string:
    mov ah, 0x0E            ; BIOS teletype function
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

disk_error:
    mov si, msg_error
    call print_string
    jmp $                   ; Infinite loop on error

; Global Descriptor Table (GDT)
gdt_start:
    dq 0x0                  ; Null descriptor
gdt_code:                   ; Code segment (index 0x08)
    dw 0xffff, 0x0, 0x9a00, 0x00cf
gdt_data:                   ; Data segment (index 0x10)
    dw 0xffff, 0x0, 0x9200, 0x00cf
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BOOT_DRIVE db 0
msg_loading db 'ChaoOS: booting', 13, 10, 0
msg_error   db 'Disk Error!', 0

times 510-($-$$) db 0
dw 0xAA55