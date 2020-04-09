#include <tables/gdt.h>

/**
 * The Global Descriptor Table (GDT) is a data structure used by Intel x86-family processors starting with the 80286 
 * in order to define the characteristics of the various memory areas used during program execution, including 
 * the base address, the size, and access privileges like executability and writability.
 * 
 * These memory areas are called segments in Intel terminology. 
 */

gdt_entry_t gdt_entries[DESCRIPTORS];
gdt_descriptor_t gdt_ptr;

void init_gdt() {
    gdt_ptr.size = sizeof(gdt_entries) - 1;
    gdt_ptr.offset = (uint32_t)gdt_entries;

    // The first entry must be a NULL descriptor
    gdt_setEntry(0, 0, 0, 0, 0);

    /**
     * Kernel CS (from 0 to 4GB).
     * Access = 0x9A:
     *     - 1  present
     *     - 00 ring 0
     *     - 1  always 1
     *     - 1  code segment
     *     - 0  can be executed by ring lower or equal to DPL,
     *     - 1  code segment is readable
     *     - 0  access bit, always 0, cpu set this to 1 when accessing this sector
     * Flags 0xCF:
     *     - 1  If 0 the limit is in 1 B blocks (byte granularity), if 1 the limit is in 4 KiB blocks (page granularity).
     *     - 1  If 0 the selector defines 16 bit protected mode. If 1 it defines 32 bit protected mode. You can have both 16 bit and 32 bit selectors at once. 
     *     - 0  0
     *     - 0  0
     *     - 1 1 1 1 -- ignored 
     */
    gdt_setEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    /**
     * Kernel DS (from 0 to 4GB).
     * Access = 0x92:
     *     - Only differ at the fifth bit(counting from least insignificant bit), 0 means it's a data segment.
     */
    gdt_setEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /**
     * User CS and DS.
     * Only differ in ring number (ring 3)
     */
    gdt_setEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_setEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_load((uint32_t)&gdt_ptr);
}

/** 
 * This function sets the bits of a gdt entry.
 */
void gdt_setEntry(int index, uint32_t base, uint64_t limit, uint8_t access, uint8_t flags) {
    gdt_entry_t *gdt_entry = &gdt_entries[index];
    
    // Set base attribute of the entry
    gdt_entry->base_low = base & 0xFFFF;
    gdt_entry->base_middle = (base >> 16) & 0xFF;
    gdt_entry->base_high = (base >> 24) & 0xFF;

    // Set limit attribute of the entry
    gdt_entry->limit_low = limit & 0xFFFF;
    gdt_entry->granularity = (limit >> 16) & 0xF;

    // Set access attribute of the entry
    gdt_entry->access = access;

    // Set the flags of the entry (higher 4 bits)
    gdt_entry->granularity = gdt_entry->granularity | (flags & 0xF0);
}