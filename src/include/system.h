#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Some useful macros
#define EOF -1

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define K 1024
#define M (K * 1024)
#define G (M * 1024)

// This defines what the stack looks like after an ISR was running
typedef struct regs {
    unsigned int gs, fs, es, ds;                            /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;    /* pushed by 'pusha' */
    unsigned int int_no, err_code;                          /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;              /* pushed by the processor automatically */ 
} __attribute__((packed)) regs_t;

// Defined in port_io.c
uint8_t inportb (uint16_t _port);
void outportb(uint16_t _port, uint8_t _data);

#endif