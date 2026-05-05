; load 'dh' sectors from drive 'dl' into [0x0000:bx]
disk_load:
    pusha

    ; Parametry z registrů dynamicky zapíšeme do našeho DAP
    mov byte [dap_sectors], dh   ; Nastavení počtu sektorů k přečtení
    mov byte [dap_sectors+1], 0  ; Vyčištění horního bajtu (aby to bylo čisté 16bit číslo)
    mov word [dap_offset], bx    ; Nastavení cílového offsetu v paměti

    ; Příprava na volání BIOSu
    mov ah, 0x42                 ; 0x42 = Extended Read (LBA)
    mov si, DAP                  ; DS:SI musí ukazovat na náš Disk Address Packet
    ; 'dl' už obsahuje číslo disku od volajícího (např. 0x80 pro HDD)
    
    int 0x13                     ; Zavolání BIOSu
    jc disk_error                ; Pokud nastala chyba (Carry flag = 1)

    popa
    ret

disk_error:
    jmp disk_loop

disk_loop:
    jmp $

; ==============================================================================
; Disk Address Packet (DAP)
; ==============================================================================
; Toto je paměťová struktura, kterou BIOS vyžaduje pro Extended Read (ah=0x42)
DAP:
    db 0x10                      ; [ 0] Velikost samotného DAP (vždy 16 bajtů)
    db 0                         ; [ 1] Nepoužito (musí být 0)
dap_sectors:
    dw 0                         ; [ 2] Počet sektorů ke čtení (upravujeme dynamicky)
dap_offset:
    dw 0                         ; [ 4] Offset cílové paměti (upravujeme dynamicky)
dap_segment:
    dw 0x0000                    ; [ 6] Segment cílové paměti (0x0000)
dap_lba:
    dq 1                         ; [ 8] Počáteční LBA (LBA 1 = sektor hned za bootloaderem)