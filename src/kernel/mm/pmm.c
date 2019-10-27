#include <mm/pmm.h>
#include <debug_utils/printf.h>

bool pushAllPMM(free_mem_t m, bool merge);
free_mem_t popAllPMM();

bool defrag = false;

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

    // Finds the worst case scenario for the _stack (for a 4GB RAM == 8MB)
    if (size > 0)    
        _half_maxStack = ((size * 1024) / PAGE_SIZE) * (sizeof(free_mem_t));

    return size;
}


bool mergePMM(free_mem_t toMerge) {
    free_mem_t m = popAllPMM();

    // Invalid
    if (m.addr == 0x0 && m.nContiguousPages == 0x0)
        return false;

    if ((uint32_t)toMerge.addr == ((uint32_t)m.addr + (m.nContiguousPages * PAGE_SIZE))) {
        // toMerge is after
//        printf("newM-PUSHING-AFTER: m.addr = 0x%x; m.pages = %d\n", m.addr, m.nContiguousPages);
        m.nContiguousPages += toMerge.nContiguousPages;
        //merged = true;
//        printf("newM-PUSHING-AFTER: m.addr = 0x%x; m.pages = %d\n", m.addr, m.nContiguousPages);
        pushAllPMM(m, false);
        return true;
    } else if ((uint32_t)m.addr == ((uint32_t)toMerge.addr + (toMerge.nContiguousPages * PAGE_SIZE))) {
        // toMerge is before
//        printf("newM-PUSHING-BEFORE: m.addr = 0x%x; m.pages = %d\n", m.addr, m.nContiguousPages);
        m.addr = toMerge.addr;
        m.nContiguousPages += toMerge.nContiguousPages;
//printf("newM-PUSHING-BEFORE: m.addr = 0x%x; m.pages = %d\n", m.addr, m.nContiguousPages);
        pushAllPMM(m, false);
        return true;
    } else {
//        printf("newM-REMERGE: toMerge.addr = 0x%x; toMerge.pages = %d\n", toMerge.addr, toMerge.nContiguousPages);
        bool ret = mergePMM(toMerge);
//        printf("newM-PUSHING: m.addr = 0x%x; m.pages = %d\n", m.addr, m.nContiguousPages);
        pushAllPMM(m, false);
        return ret;
    }
}

/**
 * This function is going to check for every struct, if it is possible to merge with other.
 *
 * This algorithm has O(n^2) (I think?) so be careful when to call it.
 * For now it is called when the _stack is half the dimension of the worst case scenario.
 * Choices are:
 *      1. when you call free() (when you push to the free _stack)
 *      2. when the _stack size reaches its limit (4Mb sounds reasonable on a 4G machine)
 *      3. whenever you schedule the idle task
 */
void defragmentPMM(bool original) {
    printf("defrag\n");
    /*free_mem_t *outer_stack = _stack - sizeof(free_mem_t *); // _stack points to the next entry
    free_mem_t *inner_stack = outer_stack - sizeof(free_mem_t *);    // Starts a struct after outer_stack
    */
    free_mem_t m;
    do {
        m = popAllPMM();
        //mergePMM(m);

        if (!mergePMM(m)) {
            // Couldn't merge - Try the next entry
            defragmentPMM(false);
        } else if (!(m.addr == 0x0 && m.nContiguousPages == 0x0)) {
            // Merged - Have to check all the _stack again
            // If it is the first call - check the _stack, otherwise push
            if (!original)
                pushAllPMM(m, false);
            else    
                defragmentPMM(true);
        }
            

    } while(m.addr == 0x0 && m.nContiguousPages == 0x0);
    defrag = true;
    /*free_mem_t m;

    // Foreach element of the _stack
    while ((m = popAllPMM()) == true) {//outer_stack > (free_mem_t *)&end) {       
         = popAllPMM();
        
        if (!mergePMM(m))

         Foreach other element of the _stack 
        while (inner_stack > (free_mem_t *)&end) {
            // Check if it is mergeble
            if (inner_stack->addr == (m.addr + (m.nContiguousPages * PAGE_SIZE))) {
                inner_stack->addr = m.addr;
                inner_stack->nContiguousPages += m.nContiguousPages;

                _stack -= sizeof(free_mem_t *);
                break;

            } else if (m.addr == (inner_stack->addr + (m.nContiguousPages * PAGE_SIZE))) {
                inner_stack->nContiguousPages += m.nContiguousPages;
                _stack -= sizeof(free_mem_t *);
                break;
            }

            inner_stack -= sizeof(free_mem_t *);
        }

        outer_stack -= sizeof(free_mem_t *);
    }*/
}

/**
 * Insert in sorted order.
 */
void sortedInsertPMM(free_mem_t m) {
    // If empty or m is greater than the top address
    if (_stack == (free_mem_t *)&end || (m.addr > (_stack - sizeof(free_mem_t *))->addr && (_stack - sizeof(free_mem_t *)) > (free_mem_t *)&end)) {
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
 * Recursive function that sorts the _stack.
 */
void sortPMM() {
    if(_stack > (free_mem_t *)&end) {
        free_mem_t tmp = popAllPMM();

        sortPMM();
        sortedInsertPMM(tmp);
    }
}

/**
 * This function pushes a struct of address + nContiguousPages onto the _stack.
 * The _stack must be ordered from the bigger address (on top) to the smaller.
 */
bool pushAllPMM(free_mem_t m, bool merge) {
    bool merged = false;

    if (merge)
        merged = mergePMM(m);

    // If it wasn't merged, insert it but in order
    if (!merged) {
        // Time to push
        *_stack = m;
        _stack += sizeof(free_mem_t);

        // A temporary _stack would be better :'(
        if (merge)
            sortPMM();
    }

    if (_stack == (free_mem_t *)(&end + _half_maxStack))
        defragmentPMM(true);

    return true;
}

/**
 * This function pops the first struct of address + nContiguousPages off the _stack.
 */
free_mem_t popAllPMM() {
    _stack -= sizeof(free_mem_t);

    if (_stack < (free_mem_t *)&end) {
        // Base _stack pointer
        _stack = (free_mem_t *)&end;

        free_mem_t m;
        m.addr = NULL;
        m.nContiguousPages = NULL;
        return m;
    } else
        // Return the value
        return *_stack;
}

/**
 * Interface Function.
 * 
 * This function allocates a page (WILL BE 4K ALIGNED) and returns the address.
 */
uint32_t pAllocPage() {
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
bool pFreePage(void *addr) {
    free_mem_t m;

    m.addr = (uint32_t)addr & 0xFFFFF000;
    m.nContiguousPages = 1;

    return pushAllPMM(m, true);
}

/**
 * Uses a First-Fit recursive technique.
 */
free_mem_t firstFit(uint32_t size) {
    free_mem_t m = popAllPMM();

    // The right one or Invalid
    if ((m.nContiguousPages * PAGE_SIZE > size) || (m.addr == NULL && m.nContiguousPages == (int)NULL))
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
        
        if (n == bigger)
            return bigger;
        else if (n == smaller)
            return smaller;
        
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
 * While pushing in the _stack there are various controls to merge and an "emergency" function when the _stack is too big, so we're safe.
 */
uint32_t pAllocPages(uint32_t size) {
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
bool pFreePages(void *addr, uint32_t size) {
    free_mem_t m;
    
    if (size == 0)
        return false;

    m.addr = addr;
    m.nContiguousPages = roundPageAligned(size) / PAGE_SIZE;

    return pushAllPMM(m, true);
}

/**
 * Little boundary check for the BootPageDirectory. It is just 4KB but I need to make sure.
 */
bool checkPD (uint32_t pd_start, uint32_t pd_end, free_mem_t m) {
    if ((uint32_t)m.addr < pd_start && (((uint32_t)m.addr + m.nContiguousPages * PAGE_SIZE) > pd_start && ((uint32_t)m.addr + m.nContiguousPages * PAGE_SIZE) < pd_end)) {
        // The memory sector start below the pd and finishes in between (3)
        m.nContiguousPages = (m.nContiguousPages * PAGE_SIZE - (pd_end - (uint32_t)m.addr)) / PAGE_SIZE;

    } else if (((uint32_t)m.addr > pd_start && (uint32_t)m.addr < pd_end) && ((uint32_t)m.addr + m.nContiguousPages * PAGE_SIZE) > pd_end) { 
        // The sector starts in the middle of the pd and ends after it (4)
        m.addr = pd_end;
        m.nContiguousPages = (m.nContiguousPages * PAGE_SIZE - (pd_end - (uint32_t)m.addr)) / PAGE_SIZE;

    } else if ((uint32_t)m.addr < pd_start && ((uint32_t)m.addr + m.nContiguousPages * PAGE_SIZE) > pd_end) {       
        // The pd is in the middle of the memory sector (5)
        free_mem_t tmp = m;

        // Push the part below
        m.nContiguousPages = (pd_start - (uint32_t)m.addr) / PAGE_SIZE;
        pushAllPMM(m, true);

        // Push the part above
        m.addr = pd_end;
        m.nContiguousPages = ((uint32_t)tmp.addr + (tmp.nContiguousPages * PAGE_SIZE) - pd_end) / PAGE_SIZE;
    } else 
        return false;

    pushAllPMM(m, true);
    return true;
}

/**
 * Checks the memory map with some overlap controls. (+ = free memory; - = [used memory])
 *
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
 *    [PHYS] +++++----++.
 */
void checkBoundaries(uint32_t pd_start, uint32_t pd_end,  uint32_t length, uint32_t base_addr) {
    free_mem_t m;

    // Kernel
    if ((base_addr < _start_addr_phys && (base_addr + length) < _start_addr_phys) ||   // The sector is totally below the kernel (1)
        (base_addr > _end_addr_phys)) {    // The sector is totally above the kernel (2)

        m.addr = base_addr & 0xFFFFF000;
        m.nContiguousPages = length / PAGE_SIZE;

    } else if (base_addr < _start_addr_phys && ((base_addr + length) > _start_addr_phys && (base_addr + length) < _end_addr_phys)) {
        // The memory sector start below the kernel and finishes in between (3)
        m.addr = base_addr & 0xFFFFF000;
        m.nContiguousPages = (base_addr + length -_end_addr_phys) / PAGE_SIZE;

    } else if ((base_addr > _start_addr_phys && base_addr < _end_addr_phys) && (base_addr + length) > _end_addr_phys) { 
        // The sector starts in the middle of the kernel and ends after it (4)
        m.addr = _end_addr_phys;
        m.nContiguousPages = (length - (_end_addr_phys - base_addr)) / PAGE_SIZE;

    } else if (base_addr < _start_addr_phys && (base_addr + length) > _end_addr_phys) {       
        // The kernel is in the middle of the memory sector (5)
        // Push the part below
        m.addr = base_addr & 0xFFFFF000;
        m.nContiguousPages = (_start_addr_phys - base_addr) / PAGE_SIZE;
        
        if (!checkPD(pd_start, pd_end, m))
            pushAllPMM(m, true);

        // Push the part above
        m.addr = _end_addr_phys;
        m.nContiguousPages = (base_addr + length -_end_addr_phys) / PAGE_SIZE;
    }

    if (!checkPD(pd_start, pd_end, m))
       pushAllPMM(m, true);
}

/**
 * To implement the physical memory manager something to hold all the free addressed is needed.
 * I'm using a run-length encoded _stack: this technique constists in pushing the address and the number of pages all at once.
 * This has several advantages:
 *      1. it requires considerably less memory
 *      2. it's trivial to fill up the initial _stack from E820 data as it uses the same format :-)
 *      3. you can search for entries with more pages if you really want to allocate contiguous physical pages
 * 
 * At this moment the real page size is 4MB still, 
 * but in the initialization of the Virtual Memory Manager it's gonna switch to 4KB.
 */
void pmm_init(multiboot_info_t* mbt, uint32_t *pd) {
    if ((mbt->flags & 0x20) == 0) {
        printf("No memory map from GRUB.\n");
        return;
    }

    _half_maxStack = 0;
    _RAM_size = findRAMSize(mbt);
    printf("Total RAM size: %d KiB\n", _RAM_size);

    // Set the start of the _stack
    _stack = (free_mem_t *)&end;
    _start_addr_phys = (uint32_t)((&start) - KERNEL_VIRTUAL_BASE) & 0xFFFFF000;
    _end_addr_phys = roundPageAligned((uint32_t)&end + _half_maxStack - KERNEL_VIRTUAL_BASE);

    // Find out what addresses are free
    memory_map_t *mmap = mbt->mmap_addr;

    // Gonna push every free block if it isn't in the kernel + _stack space
    while ((uint32_t)mmap < (mbt->mmap_addr + mbt->mmap_length)) {
        /**
         * If the memory sector is not reserved or the address is below 1MB, exclude it.
         * GRUB's mmap doesn't include some stuff that's mapped to memory below 1MB.
         */
        if (mmap->type == 0x1) {
            if(mmap->base_addr_low >= 0x100000)
                checkBoundaries(pd, pd + 1024, mmap->length_low, mmap->base_addr_low);
            if (mmap->base_addr_high >= 0x100000)
                checkBoundaries(pd, pd + 1024, mmap->length_high, mmap->base_addr_high);
        }

        mmap = (memory_map_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
}