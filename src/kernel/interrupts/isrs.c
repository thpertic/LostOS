#include <debug_utils/printf.h>
#include <interrupts/isrs.h>
#include <tables/idt.h>

// Messages of the exceptions
char *exception_messages[32] = {
    "Division by zero exception",
    "Debug exception",
    "Non maskable interrupt",
    "Breakpoint exception",
    "Into detected overflow",
    "Out of bounds exception",
    "Invalid opcode exception",
    "No coprocessor exception",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown interrupt exception",
    "Coprocessor fault",
    "Alignment check exception",
    "Machine check exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/**
 * In computer systems programming, an interrupt handler, 
 * also known as an interrupt service routine or ISR, is a special block of code associated with a specific interrupt condition. 
 * Interrupt handlers are initiated by hardware interrupts, software interrupt instructions, or software exceptions,
 * and are used for implementing device drivers or transitions between protected modes of operation, such as system calls.
 */
void isrs_init() {
    idt_setGate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_setGate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_setGate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_setGate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_setGate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_setGate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_setGate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_setGate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_setGate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_setGate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_setGate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_setGate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_setGate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_setGate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_setGate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_setGate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_setGate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_setGate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_setGate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_setGate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_setGate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_setGate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_setGate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_setGate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_setGate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_setGate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_setGate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_setGate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_setGate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_setGate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_setGate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_setGate(31, (uint32_t)isr31, 0x08, 0x8E);

    printf("ISRs set.\n");
}

/**
 * All of our Exception handling Interrupt Service Routines will point to this function. 
 * This will tell us what exception has happened.
 * Right now, we simply halt the system by hitting an endless loop. 
 * All ISRs disable interrupts while they are being serviced as a 'locking' mechanism 
 * to prevent an IRQ from happening and messing up kernel data structures.
 */
void isr_faultHandler(regs_t *r) {
    if (r->int_no < 32) {
        // An exception: print error.
        set_color(RED, BLACK);
        printf("Exception: %s (err code %x)\n", exception_messages[r->int_no], r->err_code);
        printf("DS:0x%x, CS:0x%x, ES:0x%x, GS:0x%x, FS:0x%x\n", r->ds, r->cs, r->es, r->gs, r->fs);
        printf("EAX:0x%x, EBX:0x%x, ECX:0x%x, EDX:0x%x\n", r->eax, r->ebx, r->ecx, r->edx);
        printf("ESP:0x%x, EBP:0x%x, EIP:0x%x, EDI:0x%x, ESI:0x%x\n", r->esp, r->ebp, r->eip, r->edi, r->esi);
        set_color(LIGHT_GREY, BLACK);
        for(;;) ;
    }
}