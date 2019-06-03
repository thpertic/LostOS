#include <interrupts/irqs.h>
#include <tables/idt.h>

#include <debug_utils/printf.h>

/**
 * Array of function pointers: 
 * these are the actual irq routines.
 */
void *irq_routines[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void irqs_init() {
    pic_init();

    printf("PIC set.\n");

    idt_setGate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_setGate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_setGate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_setGate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_setGate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_setGate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_setGate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_setGate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_setGate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_setGate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_setGate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_setGate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_setGate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_setGate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_setGate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_setGate(47, (uint32_t)irq15, 0x08, 0x8E);

    printf("IRQs set.\n");
}

/**
 * Normally, IRQs 0 to 7 are mapped to entries 8 to 15. 
 * This is a problem in protected mode, because IDT entry 8 is a Double Fault! 
 * Without remapping, every time IRQ0 fires, you get a Double Fault Exception, 
 * which is NOT actually what's happening. 
 * We send commands to the Programmable Interrupt Controller (PICs - also called the 8259's) 
 * in order to make IRQ0 to 15 be remapped to IDT entries 32 to 47 
 */
void pic_init() {
    // ICW1
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);

    // ICW2, irq 0 to 7 is mapped to 0x20 to 0x27, irq 8 to F is mapped to 28 to 2F
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    
    // ICW3, connect master pic with slave pic
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    
    // ICW4, set x86 mode
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    
    // clear the mask register
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}

/**
 * Each of the IRQ ISRs point to this function, rather than the 'isr_faultHandler' in 'isrs.c'.
 * The IRQ Controllers need to be told when you are done servicing them, 
 * so you need to send them an "End of Interrupt" command (0x20). 
 * There are two 8259 chips: 
 *      - The first exists at 0x20, 
 *      - the second exists at 0xA0.
 * If the second controller (an IRQ from 8 to 15) gets an interrupt, 
 * you need to acknowledge the interrupt at BOTH controllers, 
 * otherwise, you only send an EOI command to the first controller. 
 * If you don't send an EOI, you won't raise any more IRQs.
 */
void irq_faultHandler(regs_t *r) {
    // Blank function pointer
    void (*handler)(regs_t *r);

    // If the IRQ has a custom handler, call it
    handler = irq_routines[r->int_no - 32];
    if (handler)
        handler(r);

    /**
     * If the IDT entry that was invoked was greater than 40 (meaning IRQ8 - 15),
     * then we need to send an EOI to the slave controller 
     */
    if (r->int_no >= 40)
        outportb(0xA0, 0x20);
    
    outportb(0x20, 0x20);
}

/**
 * This installs a custom IRQ handler for the given IRQ 
 */
void irq_installHandler(int irq, void (*handler)(regs_t *r)) {
    irq_routines[irq - 32] = handler;
}

/**
 * This clears the handler for a given IRQ
 */
void irq_uninstallHandler(int irq) {
    irq_routines[irq - 32] = 0;
}