#ifndef KHEAP_H
#define KHEAP_H

#include <system.h>

#define KHEAP_LENGTH 0x10000000       ///< The maximum length of the heap - 256MB

/**
 * This structure represent a block inside the linked list of the heap. 
 * (It really is only a contiguous block of memory)
 * It is based in the header of the chunk.
 * Thus, when returned, it is important to remember to add sizeof(kheapHeader);
 * and when a pointer is received, subtract it.
 */
typedef struct _kheapHeader {
    struct _kheapHeader *prev;      ///< Previous block in the chunk
    struct _kheapHeader *next;      ///< Next block of the chunk 

    uint32_t size;                   ///< Size of the chunk minus the size of this header
} kheapHeader;

void init_kheap();

void *kmalloc(uint32_t size);
void *kcalloc(uint32_t n, uint32_t size);
void *krealloc(void *ptr, uint32_t newSize);

void *kfree(void *addr);

#endif