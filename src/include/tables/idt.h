#ifndef IDT_H
#define IDT_H

#include <system.h>

#define INTERRUPTS 256

/**
 * Limit: Defines the length of the IDT in bytes - 1 (minimum value is 100h, a value of 1000h means 200h interrupts).
 * Base: This 32 bits are the linear address where the IDT starts (INT 0).
 */
typedef struct idt_descriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_descriptor_t;

/**
 * The offset is a 32 bit value, split in two parts. It represents the address of the entry point of the ISR.
 * The selector is a 16 bit value and must point to a valid CS descriptor in your GDT.
 * The type_attr is specified here:
 *       7                           0
 *     +---+---+---+---+---+---+---+---+
 *     | P |  DPL  | S |    GateType   |
 *     +---+---+---+---+---+---+---+---+
 */
typedef struct idt_gate {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_gate_t;

void init_idt();
void idt_setGate(uint8_t index, uint32_t offset, uint16_t selector, uint8_t type_addr);

// defined in idt_load.asm
extern void idt_load(uint32_t idt_ptr);

#endif