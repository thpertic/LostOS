#ifndef PMM_H
#define PMM_H

#include <multiboot.h>
#include <stdint.h>
#include <system.h>

#define PAGE_SIZE 4096

/* First address then nContiguousPages */
typedef struct free_mem {
    uint32_t *addr;
    uint32_t nContiguousPages;
} free_mem_t;

free_mem_t *_stack;

extern char start;  // Of the kernel      | Linker
extern char end;    // Start of the stack | Symbols

uint32_t _start_addr_phys;
uint32_t _end_addr_phys;

uint32_t _RAM_size;

uint32_t _half_maxStack;

void pmm_init(multiboot_info_t* mbt, uint32_t *pd);

uint32_t pAllocPage();
bool pFreePage(void *addr);

uint32_t pAllocPages(uint32_t size);
bool pFreePages(void *addr, uint32_t size);

#endif