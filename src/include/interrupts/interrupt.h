#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <system.h>

// Enable/disable interrupts. Implemented in interrupt.s.
void enable_interrupts();
void disable_interrupts();
uint32_t interrupt_save_disable();
void interrupt_restore(uint32_t eflags);

#endif