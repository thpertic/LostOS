#include <mm/pmm.h>
#include <debug_utils/printf.h>

bool pushAllPMM(free_mem_t m, bool merge);
free_mem_t popAllPMM();

/**
 * Get and anylize the GRUB memory map to count the RAM size.
 */
unsigned int findRAMSize(multiboot_info_t* mbt) {
    uint32_t size = 0;

    /**
     * If the bit 0 is set, it is possible to safely refer to mbd->mem_lower for conventional memory and mbd->mem_upper for high memory.
     * Both are given in kibibytes, i.e. blocks of 1024 bytes each.
     */
    if(mbt->flags & 0x1)
        size = mbt->mem_lower + mbt->mem_upper;
    /**
     * Otherwise bit 6 in the flags uint16_t is set, then the mmap_* fields are valid, 
     * and indicate the address and length of a buffer containing a memory map of the machine provided by the BIOS
     */
    else if (mbt->flags & 0x20) {
        memory_map_t *mmap = mbt->mmap_addr;

        while ((unsigned int)mmap < (mbt->mmap_addr + mbt->mmap_length)) {
            size += mbt->mmap_length;
            mmap = (memory_map_t *)(mmap + mmap->size + sizeof(mmap->size));
        }
    }

    // Finds the worst case scenario for the stack (for a 4GB RAM == 8MB)
    if (size > 0)    
        half_maxStack = ((size * 1024) / PAGE_SIZE) * (sizeof(free_mem_t));

    return size;
}

/**
 * This function is going to check for every struct, if it is possible to merge with other.
 *
 * This algorithm has O(n^2) (I think?) so be careful when to call it.
 * For now it is called when the stack is half the dimension of the worst case scenario.
 * Choices are:
 *      1. when you call free() (when you push to the free stack)
 *      2. when the stack size reaches its limit (4Mb sounds reasonable on a 4G machine)
 *      3. whenever you schedule the idle task
 */
void defragmentPMM() {
    printf("defrag\n");
    free_mem_t *outer_stack = stack - sizeof(free_mem_t *); // stack points to the next entry
    free_mem_t *inner_stack = outer_stack - sizeof(free_mem_t *);    // Starts a struct after outer_stack

    // Foreach element of the stack
    while (outer_stack > (free_mem_t *)&end) {       
        free_mem_t m = *outer_stack;
        
        // Foreach other element of the stack 
        while (inner_stack > (free_mem_t *)&end) {
            // Check if it is mergeble
            if (inner_stack->addr == (m.addr + (m.nContiguousPages * PAGE_SIZE))) {
                inner_stack->addr = m.addr;
                inner_stack->nContiguousPages += m.nContiguousPages;

                stack -= sizeof(free_mem_t *);
                break;

            } else if (m.addr == (inner_stack->addr + (m.nContiguousPages * PAGE_SIZE))) {
                inner_stack->nContiguousPages += m.nContiguousPages;
                stack -= sizeof(free_mem_t *);
                break;
            }

            inner_stack -= sizeof(free_mem_t *);
        }

        outer_stack -= sizeof(free_mem_t *);
    }
}

/**
 * Insert in sorted order.
 */
void sortedInsertPMM(free_mem_t m) {
    // If empty or m is greater than the top address
    if (stack == (free_mem_t *)&end || (m.addr > (stack - sizeof(free_mem_t *))->addr && (stack - sizeof(free_mem_t *)) > (free_mem_t *)&end)) {
        pushAllPMM(m, false);
        return;
    }

    // Otherwise remove the top item and order
    free_mem_t tmp = popAllPMM();
    sortedInsertPMM(m);

    // Push it back
    pushAllPMM(tmp, false);
}

/**
 * Recursive function that sorts the stack.
 */
void sortPMM() {
    if(stack > (free_mem_t *)&end) {
        free_mem_t tmp = popAllPMM();

        sortPMM();
        sortedInsertPMM(tmp);
    }
}

/**
 * This function pushes a struct of address + nContiguousPages onto the stack.
 * The stack must be ordered from the bigger address (on top) to the smaller.
 */
bool pushAllPMM(free_mem_t m, bool merge) {
    bool merged = false;
    free_mem_t *fetch_stack = stack - sizeof(free_mem_t *);

    if (merge) {
        // Check if m can be merged
        while (fetch_stack > (free_mem_t *)&end) {
            if (fetch_stack->addr == (m.addr + (m.nContiguousPages * PAGE_SIZE))) {
                // m is before
                fetch_stack->addr = m.addr;
                fetch_stack->nContiguousPages += m.nContiguousPages;

                merged = true;
                printf("merged before\n");
                break;

            } else if (m.addr == (fetch_stack->addr + (m.nContiguousPages * PAGE_SIZE))) {
                // m is after
                fetch_stack->nContiguousPages += m.nContiguousPages;
                merged = true;
                printf("merged after\n");
                break;
            }
            fetch_stack -= sizeof(free_mem_t *);
        }
    }

    // If it wasn't merged, insert it but in order
    if (!merged) {
        // Time to push
        *stack = m;
        stack += sizeof(free_mem_t);

        // A temporary stack would be better :'(
        if (merge)
            sortPMM();
    }

    if (stack == (free_mem_t *)(&end + half_maxStack))
        defragmentPMM();

    return true;
}

/**
 * This function pops the first struct of address + nContiguousPages off the stack.
 */
free_mem_t popAllPMM() {
    stack -= sizeof(free_mem_t);

    if (stack < (free_mem_t *)&end) {
        // Base stack pointer
        stack = (free_mem_t *)&end;

        free_mem_t m;
        m.addr = NULL;
        m.nContiguousPages = NULL;
        return m;
    } else
        // Return the value
        return *stack;
}

/**
 * Interface Function.
 * 
 * This function allocates a page (WILL BE 4K ALIGNED) and returns the address.
 */
uint32_t allocateBlock() {
    free_mem_t m = popAllPMM();
    uint32_t addr = NULL;

    if (m.nContiguousPages == 1) {
        return m.addr;
    } else if (m.nContiguousPages > 1) {
        addr = m.addr;

        m.addr = addr + PAGE_SIZE;
        m.nContiguousPages--;

        pushAllPMM(m, true);
        return addr;
    }

    return addr;
}

/**
 * Interface Function.
 * 
 * This function frees a page, returning true or false if it finished well.
 */
bool unallocateBlock(uint32_t addr) {
    free_mem_t m;

    m.addr = addr & 0xFFFFF000;
    m.nContiguousPages = 1;

    return pushAllPMM(m, true);
}

/**
 * Uses the First-Fit recursive technique.
 */
free_mem_t firstFit(uint32_t size) {
    free_mem_t m = popAllPMM();

    // The right one or Invalid
    if ((m.nContiguousPages * PAGE_SIZE > size) || (m.addr == NULL && m.nContiguousPages == NULL))
        return m;

    free_mem_t b = firstFit(size);
    pushAllPMM(m, true);

    return b;
}

/**
 * This function returns the bigger 4KB aligned size.
 * 
 * (i. e. for 315 Byte will return 4KB, while for 16387 Byte will return 20K)
 */
uint32_t roundPageAligned(uint32_t n) {
    uint32_t smaller = 0;
    uint32_t bigger = PAGE_SIZE;

    while (!(n > smaller && n < bigger)) {
        smaller += PAGE_SIZE;
        bigger += PAGE_SIZE;
    }
    return bigger;
}

/**
 * Interface Function.
 * 
 * This function allocates a wanted size (NOT NUMBER OF PAGES - BYTES).
 * It uses the technique called "First-Fit". It's faster but causes fragmentation. 
 * While pushing in the stack there are various controls to merge and an "emergency" function when the stack is too big, so we're safe.
 */
uint32_t allocateBlocks(uint32_t size) {
    if (size <= 0)
        return NULL;

    uint32_t alignedSize = roundPageAligned(size);

    free_mem_t m = firstFit(alignedSize); 
    uint32_t addr = m.addr;

    m.addr = (uint32_t)addr + alignedSize;
    m.nContiguousPages -= (alignedSize) / PAGE_SIZE;

    // If it is valid
    if (m.nContiguousPages > 0)
        pushAllPMM(m, true);

    return addr;
}

/**
 * Interface Function.
 * 
 * This function frees memory from the address addr for size.
 */
bool unallocateBlocks(uint32_t addr, uint32_t size) {
    free_mem_t m;
    
    if (size == 0)
        return false;

    m.addr = addr;
    m.nContiguousPages = roundPageAligned(size) / PAGE_SIZE;

    return pushAllPMM(m, true);
}

/**
 * To implement the physical memory manager something to hold all the free addressed is needed.
 * I'm using a run-length encoded stack: this technique constists in pushing the address and the number of pages all at once.
 * This has several advantages:
 *      1. it requires considerably less memory
 *      2. it's trivial to fill up the initial stack from E820 data as it uses the same format :-)
 *      3. you can search for entries with more pages if you really want to allocate contiguous physical pages
 * 
 * At this moment the real page size is 4MB still, 
 * but in the initialization of the Virtual Memory Manager it's gonna switch to 4KB.
 */
void pmm_init(multiboot_info_t* mbt) {
    half_maxStack = 0;
    printf("Total RAM size: %d KiB\n", findRAMSize(mbt));

    // Set the start of the stack
    stack = (free_mem_t *)&end;
    start_addr = (&start) - KERNEL_VIRTUAL_BASE;

// TODO: In init of the virtual memory manager
    // Create a new Page Directory (the official one)
    // Create a new Page Table
    // Identity map the first MB - not this
    // Map 0xC00000000 to 0x0 for the enitre page table
    // Switch 4MB pages to 4KB ones

    // Find out what addresses are free
    memory_map_t *mmap = mbt->mmap_addr;
    end_addr = (uint32_t)&end + half_maxStack;

    // Gonna push every free block if it isn't in the kernel + stack space
    while ((uint32_t)mmap < (mbt->mmap_addr + mbt->mmap_length)) {
        // If the memory sector is not reserved
        if (mmap->type == 0x1) {
            free_mem_t m;

            // If it is a valid memory sector
            if (mmap->length_low > 0) {
                /**
                 * Some overlap controls. (+ = free memory; - = [kernel + stack space])
                 * 1) [GRUB] +++++++       
                 *    [PHYS] +++++++----
                 * 
                 * 2) [GRUB]     +++++++       
                 *    [PHYS] ----+++++++
                 * 
                 * 3) [GRUB] +++++++       
                 *    [PHYS] +++++----
                 * 
                 * 4) [GRUB]   +++++++       
                 *    [PHYS] ----+++++
                 * 
                 * 5) [GRUB] +++++++++++
                 *    [PHYS] +++++----++
                 */
                if ((mmap->base_addr_low < start_addr && (mmap->base_addr_low + mmap->length_low) < start_addr) ||   // The sector is totally below the kernel (1)
                    (mmap->base_addr_low > end_addr)) {    // The sector is totally above the kernel (2)

                    m.addr = mmap->base_addr_low & 0xFFFFF000;
                    m.nContiguousPages = mmap->length_low / PAGE_SIZE;

                } else if (mmap->base_addr_low < start_addr && ((mmap->base_addr_low + mmap->length_low) > start_addr && (mmap->base_addr_low + mmap->length_low) < end_addr)) {   // The memory sector start below the kernel and finishes in between (3)
                    m.addr = mmap->base_addr_low & 0xFFFFF000;
                    m.nContiguousPages = (mmap->length_low - (end_addr - mmap->base_addr_low)) / PAGE_SIZE;

                } else if ((mmap->base_addr_low > start_addr && mmap->base_addr_low < end_addr) && (mmap->base_addr_low + mmap->length_low) > end_addr) { // The sector starts in the middle of the kernel and ends after it (4)
                    m.addr = end_addr;
                    m.nContiguousPages = (mmap->length_low - (end_addr - mmap->base_addr_low)) / PAGE_SIZE;

                } else if (mmap->base_addr_low < start_addr && (mmap->base_addr_low + mmap->length_low) > end_addr) {       // The kernel is in the middle of the memory sector (5)
                    // Push the part below
                    m.addr = mmap->base_addr_low & 0xFFFFF000;
                    m.nContiguousPages = (mmap->base_addr_low - start_addr) / PAGE_SIZE;
                    
                    printf("GRUB: m.addr = 0x%x; pages = %d\n", m.addr, m.nContiguousPages);
                    pushAllPMM(m, true);

                    // Push the part above
                    m.addr = end_addr;
                    m.nContiguousPages = (((mmap->base_addr_low & 0xFFFFF000) + mmap->length_low) - mmap->length_low) / PAGE_SIZE;
                }
                printf("GRUB: m.addr = 0x%x; pages = %d\n", m.addr, m.nContiguousPages);
                pushAllPMM(m, true);
            }

            // Same thing as the above but for the high part
            if (mmap->length_high > 0) {
                if ((mmap->base_addr_high < start_addr && (mmap->base_addr_high + mmap->length_high) < start_addr) ||   // The sector is totally below the kernel (1)
                    (mmap->base_addr_high > end_addr)) {    // The sector is totally above the kernel (2)

                    m.addr = mmap->base_addr_high & 0xFFFFF000;
                    m.nContiguousPages = mmap->length_high / PAGE_SIZE;

                } else if (mmap->base_addr_high < start_addr && ((mmap->base_addr_high + mmap->length_high) > start_addr && (mmap->base_addr_high + mmap->length_high) < end_addr)) {   // The memory sector start below the kernel and finishes in between (3)
                    m.addr = mmap->base_addr_high & 0xFFFFF000;
                    m.nContiguousPages = (mmap->length_high - (end_addr - mmap->base_addr_high)) / PAGE_SIZE;

                } else if ((mmap->base_addr_high > start_addr && mmap->base_addr_high < end_addr) && (mmap->base_addr_high + mmap->length_high) > end_addr) { // The sector starts in the middle of the kernel and ends after it (4)
                    m.addr = end_addr;
                    m.nContiguousPages = (mmap->length_high - (end_addr - mmap->base_addr_high)) / PAGE_SIZE;

                } else if (mmap->base_addr_high < start_addr && (mmap->base_addr_high + mmap->length_high) > end_addr) {       // The kernel is in the middle of the memory sector (5)
                    // Push the part below
                    m.addr = mmap->base_addr_high & 0xFFFFF000;
                    m.nContiguousPages = (mmap->base_addr_high - start_addr) / PAGE_SIZE;
printf("GRUB: m.addr = 0x%x; pages = %d\n", m.addr, m.nContiguousPages);
                    pushAllPMM(m, true);
                    
                    // Push the part above
                    m.addr = end_addr;
                    m.nContiguousPages = (((mmap->base_addr_high & 0xFFFFF000) + mmap->length_high) - mmap->length_high) / PAGE_SIZE;
                }
                printf("GRUB: m.addr = 0x%x; pages = %d\n", m.addr, m.nContiguousPages);
                pushAllPMM(m, true);
            }
        }
        mmap = (memory_map_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }

    uint32_t a = allocateBlock();
    printf("a allocated at 0x%x\n", a);

    uint32_t b = allocateBlocks(23);
    printf("b allocated at 0x%x\n", b);

    uint32_t c = allocateBlock();
printf("c allocated at 0x%x\n", c);
    uint32_t d = allocateBlocks(12654);
    printf("d allocated at 0x%x\n", d);
    
    unallocateBlocks(b, 23);
    printf("b deallocated \n");
    unallocateBlock(c);
    printf("c deallocated \n");
    c = allocateBlock();
        printf("c reallocated at 0x%x\n",c);
   unallocateBlock(a);
     printf("a deallocated \n");
    unallocateBlocks(d, 12654);
    printf("d deallocated \n");
    unallocateBlock(c);
    printf("c deallocated \n");

    free_mem_t tmp;
            
    do {
        tmp = popAllPMM();
        printf("address: 0x%x, pages: %d \n", tmp.addr, tmp.nContiguousPages);

    } while(!(tmp.addr == 0 && tmp.nContiguousPages == 0));
}