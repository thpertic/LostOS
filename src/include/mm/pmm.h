#ifndef PMM_H
#define PMM_H

#include <multiboot.h>
#include <stdint.h>
#include <system.h>

#define PAGE_SIZE 4096

typedef struct free_mem {
    uint32_t *addr;
    uint32_t nContiguousPages;
} free_mem_t;

free_mem_t *stack;

extern char start;  // Of the kernel      | Linker
extern char end;    // Start of the stack | Symbols

uint32_t start_addr;
uint32_t end_addr;

uint32_t half_maxStack;

void pmm_init(multiboot_info_t* mbt);

/* First address then nContiguousPages */
uint32_t allocateBlock();
bool unallocateBlock(uint32_t addr);

uint32_t allocateBlocks(uint32_t size);
bool unallocateBlocks(uint32_t addr, uint32_t size);

#endif