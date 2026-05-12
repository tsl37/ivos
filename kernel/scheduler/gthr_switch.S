[bits 32]
section .text
global gt_switch

gt_switch:
    ; [esp + 4] je první argument (old_ctx)
    ; [esp + 8] je druhý argument (new_ctx)
    mov eax, [esp + 4]
    mov edx, [esp + 8]

    ; Uložení starého kontextu
    mov [eax + 0], esp
    mov [eax + 4], edi
    mov [eax + 8], esi
    mov [eax + 12], ebp
    mov [eax + 16], ebx

    ; Načtení nového kontextu
    mov esp, [edx + 0]
    mov edi, [edx + 4]
    mov esi, [edx + 8]
    mov ebp, [edx + 12]
    mov ebx, [edx + 16]

    ret