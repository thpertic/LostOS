#include <mm/kheap.h>
#include <mm/pmm.h>

#include <debug_utils/printf.h>

uint32_t *_kheapStart, _kheapEnd;

/**
 * Initialize kernel heap.
 * 
 * This is done with a linked list.
 * The starting address will be after the PMM stack.
 */
void init_kheap() {
    // First address to be given
    _kheapStart = roundPageAligned((free_mem_t *)&end) + _halfMaxStack;
    _kheapEnd = _kheapStart;   
}

// TODO: void *kinda-malloc(size) 
// looks through the list and allocate if it is free, 
// if it isn't ask for another contiguous block 
// and allocate the needed space