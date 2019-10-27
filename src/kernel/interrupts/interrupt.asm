global enable_interrupts
global disable_interrupts
global interrupt_save_disable
global interrupt_restore

section .text

enable_interrupts:
    sti
    ret
disable_interrupts:
    cli
    ret

interrupt_save_disable:
    pushfd
    pop eax
    cli
    ret
interrupt_restore:
    mov eax, [esp + 4]
    push eax
    popfd
    sti
    ret
