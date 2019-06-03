#ifndef GDT_H
#define GDT_H

#include <system.h>

/**
 * 2 segments descriptors for kernel mode;
 * 2 segments descriptors for user mode.
 * + 1 NULL
 */
#define DESCRIPTORS 5

/*** GDT STRUCTURES ***/
typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    // lower 4 bits for limit_high and higher 4 bits for flags
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

/**
 * The size is the size of the table subtracted by 1.
 * The offset is the linear address of the table itself.
 */
typedef struct gdt_descriptor {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) gdt_descriptor_t;

void gdt_init();
void gdt_setEntry(int index, uint32_t base, uint64_t limit, uint8_t access, uint8_t flags);

// Defined in gdt_load.asm
extern void gdt_load(uint32_t gdt_ptr);

#endif