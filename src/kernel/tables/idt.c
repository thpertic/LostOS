#include <common/string.h>
#include <tables/idt.h>
#include <interrupts/isrs.h>
#include <interrupts/irqs.h>
#include <common/utility.h>

#include <debug_utils/printf.h>

idt_gate_t idt_gates[INTERRUPTS];
idt_descriptor_t idt_ptr;

void idt_init() {
    // Set the idt pointer
    idt_ptr.limit = sizeof(idt_gates) - 1;
    idt_ptr.base = (uint32_t)&idt_gates;

    // Clear out the entire IDT
    memset((uint32_t)&idt_gates, 0, (sizeof(idt_gate_t) * INTERRUPTS));
    
    // ISRs and IRQs 
    isrs_init();
    irqs_init();

    // Points the processor's internal register to the new IDT
    idt_load((uint32_t)&idt_ptr);
    __asm__ __volatile__("sti");
}

void idt_setGate(uint8_t index, uint32_t offset, uint16_t selector, uint8_t type_addr) {
    idt_gate_t *idt_gate = &idt_gates[index];

    // Set the address of the idt entry
    idt_gate->offset_low = offset & 0xFFFF;
    idt_gate->offset_high = (offset >> 16) & 0xFFFF;

    // Set the selector
    idt_gate->selector = selector;

    // Clean the zero
    idt_gate->zero = 0;

    idt_gate->type_attr = type_addr | 0x60; // <-- For user mode
}