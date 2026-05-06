; kernel/scheduler/gthr_switch.S
[bits 32]
global gt_switch

gt_switch:
    ; Parameters: [esp + 4] = old_context, [esp + 8] = new_context
    mov eax, [esp + 4]      ; Get pointer to old gt_context
    mov edx, [esp + 8]      ; Get pointer to new gt_context

    ; Save current state
    mov [eax + 0], esp      ; Save ESP
    mov [eax + 4], edi      ; Save EDI
    mov [eax + 8], esi      ; Save ESI
    mov [eax + 12], ebp     ; Save EBP
    mov [eax + 16], ebx     ; Save EBX

    ; Load new state
    mov esp, [edx + 0]      ; Restore ESP
    mov edi, [edx + 4]      ; Restore EDI
    mov esi, [edx + 8]      ; Restore ESI
    mov ebp, [edx + 12]     ; Restore EBP
    mov ebx, [edx + 16]     ; Restore EBX

    ret