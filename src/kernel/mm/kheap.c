#include <mm/kheap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#include <common/utility.h>

#include <debug_utils/printf.h>

uint32_t *_kheapStart;       ///< Virtual address of where the heap starts
uint32_t *_kheapEnd;         ///< Virtual address of where the heap ends (next available address)
kheapHeader *_kheapFirst;    ///< The first block of the heap linked list

/**
 * Initialize kernel heap.
 * 
 * This is done with a linked list.
 * The starting address will be after the PMM stack.
 */
void init_kheap() {
    _kheapStart = roundPageAligned((free_mem_t *)&end) + _halfMaxStack;
    _kheapEnd = _kheapStart;
}

/**
 * \brief Function to allocate the heap for the kernel.
 * 
 * A heap is a vital component of both application programs and the kernel. 
 * It is also generally superseded by a higher level of memory management that deals with larger chunks of memory. 
 * For most operating systems memory will be allocated based on pages or other large chunks. 
 * 
 * This is used to dynamically allocate a single large block of memory with the specified size. 
 * It returns a pointer of type void which can be cast into a pointer of any form.
 * 
 * @param size Size (in bytes) to be allocated.
 * 
 * @return Allocated pointer
 */
void *kmalloc(uint32_t size) {
    int n = roundPageAligned(size + sizeof(kheapHeader)) / PAGE_SIZE;
    if (_kheapFirst == NULL) {
        //If the first block isn't allocated, allocate it.
        _kheapFirst = vAllocPages(_kheapStart, BIT_PD_PT_PRESENT | BIT_PD_PT_RW, n, true);
        if(!_kheapFirst)
            return NULL;
        _kheapEnd += n * PAGE_SIZE;
        _kheapFirst->size = n * PAGE_SIZE;
        _kheapFirst->prev = NULL;
    }

    kheapHeader *block = _kheapFirst;
    while (block->next != NULL) {
        if (block->size == size) {
            // Eliminate this block from the list and return it
            block->prev->next = block->next;
            block->next->prev = block->prev;
            return block + sizeof(kheapHeader);

        } else if (block->size > size) {
            // Modify the block subtracting the size
            block->size -= (size + sizeof(kheapHeader));

            kheapHeader *ret_block = (uint32_t)block + block->size;
            ret_block->size = size;
            // Set this up to have an easier freeing
            ret_block->prev = block;
            ret_block->next = block->next;

            // Then return the calculated free address
            return (void *)((uint32_t)ret_block + sizeof(kheapHeader));
        }
        block = block->next;
    } 
    // Check on the last block (in this way, I won't lose it)
    if (block->size == size) {
        // Eliminate this block from the list and return it
        block->prev->next = block->next;
        block->next->prev = block->prev;
        return block + sizeof(kheapHeader);

    } else if (block->size > size) {
        // Modify the block subtracting the size
        block->size -= (size + sizeof(kheapHeader));

        kheapHeader *ret_block = (uint32_t)block + block->size;
        ret_block->size = size;
        // Set this up to have an easier freeing
        ret_block->prev = block;
        ret_block->next = block->next;

        // Then return the calculated free address
        return (void *)((uint32_t)ret_block + sizeof(kheapHeader));
    }
    
    if ((uint32_t)_kheapEnd + n * PAGE_SIZE > (uint32_t)_kheapStart + KHEAP_LENGTH) {
        // Run out of memory
        return NULL;
    }

    // No memory, but available request some
    kheapHeader *new_block;
    new_block = vAllocPages(_kheapEnd, BIT_PD_PT_PRESENT | BIT_PD_PT_RW, n, true);
    if (!new_block)
        return NULL;
    _kheapEnd += (n * PAGE_SIZE); 

    new_block->size = n * PAGE_SIZE - size - sizeof(kheapHeader);
    new_block->prev = block;
    new_block->next = NULL;

    block->next = new_block;

    // Take the last part of the new chunk and return it
    kheapHeader *ret_block = new_block + new_block->size;
    ret_block->size = size;
    ret_block->prev = new_block;
    ret_block->next = NULL;
    return ret_block + sizeof(kheapHeader);
}

/**
 * This is used to dynamically de-allocate the memory.
 * The memory allocated using functions malloc() and calloc() is not de-allocated on their own. 
 * Hence the free() method is used, whenever the dynamic memory allocation takes place. 
 * It helps to reduce wastage of memory by freeing it.
 * 
 * @param addr Address to free.
 * 
 * @return The freed address or NULL.
 */
void *kfree(void *addr) {
    kheapHeader *free_block = (uint32_t)addr - sizeof(kheapHeader);
    kheapHeader *tmp = _kheapFirst;
    
    while (tmp->next != NULL) {
        if ((free_block + free_block->size) == tmp) {
            // They're contiguous (free_block is behind), merge the blocks
            tmp->size += free_block->size;
            tmp = free_block;   // TODO: VERIFY THIS
            return tmp;
        
        } else if (free_block->next == tmp) {
            // Just add the block
            free_block->prev = tmp->prev;
            tmp->prev = free_block;
            return free_block;

        } else if ((tmp + tmp->size) == free_block) {
            // They're contiguous (free_block is in the front), merge the blocks
            tmp->size += free_block->size;
            return tmp;
        
        } else if (free_block->prev <= tmp) {
            // Just add the block
            free_block->next = tmp->prev;
            tmp->next = free_block;
            return free_block;

        }
        tmp = tmp->next;
    }
    // For the last block
    if ((free_block + free_block->size) == tmp) {
        // They're contiguous (free_block is behind), merge the blocks
        tmp->size += free_block->size;
        tmp = free_block;   // TODO: VERIFY THIS
        return tmp;
    
    } else if (free_block->next == tmp) {
        // Just add the block
        free_block->prev = tmp->prev;
        tmp->prev = free_block;
        return free_block;

    } else if ((tmp + tmp->size) == free_block) {
        // They're contiguous (free_block is in the front), merge the blocks
        tmp->size += free_block->size;
        return tmp;
    
    } else if (free_block->prev <= tmp) {
        // Just add the block
        free_block->next = tmp->prev;
        tmp->next = free_block;
        return free_block;

    }
    return NULL;
}

/**
 * This is used to dynamically allocate the specified number of blocks of memory of the specified type. 
 * It initializes each block with a default value ‘0’.
 *
 * @param n Number of elements
 * @param size Size of the element's type (sizeof())
 * 
 * @return Allocated pointer or NULL
 */
void *kcalloc(uint32_t n, uint32_t size) {
    uint32_t *ptr = kmalloc(n * size);
    if (!ptr)
        return NULL;

    memsetw(ptr, 0, n);
    if (!ptr)
        return NULL;
    return ptr;
}

/**
 * This is used to dynamically change the memory allocation of a previously allocated memory. 
 * In other words, if the memory previously allocated with the help of malloc or calloc is insufficient, 
 * realloc can be used to dynamically re-allocate memory.
 * 
 * @param ptr Pointer to reallocate
 * @param newSize New size
 * 
 * @return New pointer or NULL
 */
void *krealloc(void *ptr, uint32_t newSize) {
    kheapHeader *block = ptr - sizeof(kheapHeader);
    kheapHeader *tmp = _kheapFirst;

    if (newSize <= block->size)
        return NULL;

    while(tmp->next != NULL) {
        if (block + block->size == tmp) {
            // This is a contiguous chunk of memory
            block->size += (newSize - block->size);

            // Set up the new shifted block 
            kheapHeader *new_block = tmp + (newSize - block->size);
            new_block->prev = tmp->prev;
            new_block->next = tmp->next;

            tmp->prev->next = new_block;
            tmp->next->prev = new_block;            
            return block;
        }
        tmp = tmp->next;
    }
    // Last block
    if (block + block->size == tmp) {
        // This is a contiguous chunk of memory
        block->size += (newSize - block->size);

        // Set up the new shifted block 
        kheapHeader *new_block = tmp + (newSize - block->size);
        new_block->prev = tmp->prev;
        new_block->next = tmp->next;

        tmp->prev->next = new_block;
        tmp->next->prev = new_block;            
        return block;
    }

    // Otherwise find an entire new block and copy everything
    uint32_t new_ptr = kmalloc(newSize);
    if (!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, block->size);
    kfree(block);
    return new_ptr;
}