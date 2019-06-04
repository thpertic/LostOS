global setGDT
setGDT:
    mov eax, [esp + 4]
    lgdt [eax]

    ; Reload CS with a long jump
    jmp 0x08:reloadCS

reloadCS:
    ; Reload the rest of the registers
    mov ax, 0x10 ; 0x10 points at the new data selector
    
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
