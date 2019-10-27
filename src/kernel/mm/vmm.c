#include <mm/vmm.h>
#include <mm/pmm.h>

#include <common/utility.h>

#include <interrupts/interrupt.h>

#include <debug_utils/printf.h>

/**
 * The CPU pushes an error code on the stack before firing a page fault exception. 
 * The error code must be analyzed by the exception handler to determine how to handle the exception. 
 * The bottom 3 bits of the exception code are the only ones used, bits 3-31 are reserved. 
 * 
 * The combination of these flags specify the details of the Page Fault and indicate what action to take:
 *
 * US RW  P - Description
 *  0  0  0 - Supervisory process tried to read a non-present page entry
 *  0  0  1 - Supervisory process tried to read a page and caused a protection fault
 *  0  1  0 - Supervisory process tried to write to a non-present page entry
 *  0  1  1 - Supervisory process tried to write a page and caused a protection fault
 *  1  0  0 - User process tried to read a non-present page entry
 *  1  0  1 - User process tried to read a page and caused a protection fault
 *  1  1  0 - User process tried to write to a non-present page entry
 *  1  1  1 - User process tried to write a page and caused a protection fault
 **/

/**
 * Mapping a virtual address to a physical one.
 */
bool vMapPage(void *phys, void *virt, uint32_t flags) {
	uint32_t *pd = PD_VADDR;
	if (!(pd[PAGE_DIRECTORY_INDEX((uint32_t)virt)] & 1)) {
		// The page table doesn't exists
		// Create a new one and map it into the page directory
		uint32_t *new_pt = (uint32_t *)pAllocPage();
		pd[PAGE_DIRECTORY_INDEX((uint32_t)virt)] = (uint32_t)new_pt | flags;
	}

	uint32_t *pt = (uint32_t *)(PT_BASE_VADDR + (PAGE_DIRECTORY_INDEX((uint32_t)virt) * 0x1000));
	if (!(pt[PAGE_TABLE_INDEX((uint32_t)virt)] & 1)) {
		// The page isn't present
		pt[PAGE_TABLE_INDEX((uint32_t)virt)] = (uint32_t)phys | flags;
		
	} else
		return false;

	printf("physical address 0x%x mapped at 0x%x with flags 0x%x\n", phys, virt, flags);
	return true;
}

/**
 * Unmapping the virtual address from the current page directory.
 */
void vUnmapPage(void *virt) {
	uint32_t *pd = PD_VADDR;
	
	if (pd[PAGE_DIRECTORY_INDEX((uint32_t)virt)] & 1) {
		// The page table is present
		uint32_t *pt = (uint32_t *)(PT_BASE_VADDR + (PAGE_DIRECTORY_INDEX((uint32_t)virt) * 0x1000));
		if (pt[PAGE_TABLE_INDEX((uint32_t)virt)] & 1)
			// The page is mapped, unmap it.
			// Set not-present, but r/w
			pt[PAGE_TABLE_INDEX((uint32_t)virt)] = 0x2;		
		
		// Check if there are no more pages present
		int i = 0;
		while (i < 1024 && !(pt[i] & 1)) {
			i++;
		}

		if (i == 1024)
			// The page table is empty, free the memory
			pFreePage(pd[PAGE_DIRECTORY_INDEX((uint32_t)virt)] & 0xFFFFF000);
			
		printf("0x%x unmapped\n", virt);
	}
}

/**
 * The Virtual Memory Manager is going to start and handle paging in the system.
 * 
 * Paging is a system which allows each process to see a full virtual address space, without actually requiring the full amount of physical memory to be available or present.
 * Maximum address space for 32 bit systems like this is 4GB. 
 * 
 * The topmost paging structure is the PAGE DIRECTORY. It is essentially an array of page directory entries that take the following form:
 *  - 31_12 Page Table 4KB aligned address
 *  - 11_8 Available
 *  - 7_0 Flags
 *    - 7 G_Ignored
 *    - 6 S_Page_Size
 *    - 5 A_Accessed
 *    - 4 D_Cache_Disable
 *    - 3 W_Write_Through
 *    - 2 U_User\Supervisor
 *    - 1 R_Read\Write
 *    - 0 P_Present
 * 
 * In each PAGE TABLE, as it is, there are also 1024 entries. These are called page table entries, and are very similar to page directory entries:
 *  - 31_12 Physical Page 4KB aligned address
 *  - 11_9 Available
 *  - 7_0 Flags
 *    - 7 G_Global
 *    - 6 D_Dirty
 *    - 5 A_Accessed
 *    - 4 D_Cache_Disable
 *    - 3 W_Write_Through
 *    - 2 U_User\Supervisor
 *    - 1 R_Read\Write
 *    - 0 P_Present
 * 
 * This is the format of a 32 bit x86 virtual address:
 *      AAAAAAAAAA         BBBBBBBBBB        CCCCCCCCCCCC
 *      directory index    page table index  offset into page
 */
void vmm_init() {
    uint32_t eflags = interrupt_save_disable();	

	uint32_t *pd_p __attribute__((aligned(4096))) = pAllocPage();
	uint32_t *pd_v = 0xAAAAAA000;
	vMapPage(pd_p, pd_v, BIT_PD_PT_RW | BIT_PD_PT_PRESENT); 

    /**
     * Using the 'recursive mapping' technique.
     * It consists in mapping the last entry in the page directory to the page directory iteself.
     * The physical address of the page directory is in the registry 'cr3' and 
     * it's more helpful having it at the virtual address 0xFFFFF000.
     * 
     * This becomes more important when each process gets its own page directory, which can be anywhere in memory.
     */
    uint32_t *self_pde_v = 0xAAEAA000; 
	uint32_t *self_pde_p __attribute__((aligned(4096))) = (uint32_t)pAllocPage();
	vMapPage(self_pde_p, self_pde_v, BIT_PD_PT_RW | BIT_PD_PT_PRESENT);
    memset(self_pde_v, 0, sizeof(self_pde_v));
	
	self_pde_v = (uint32_t)pd_p | BIT_PD_PT_PRESENT | BIT_PD_PT_RW;
	pd_v[PAGE_DIRECTORY_INDEX(PD_VADDR)] = self_pde_v;
	
	// 4MB page for the kernel
	uint32_t *kernel_pde_v = 0xAB2AA000; 
	uint32_t *kernel_pde_p __attribute__((aligned(4096))) = (uint32_t)pAllocPage();
	vMapPage(kernel_pde_p, kernel_pde_v, BIT_PD_PT_RW | BIT_PD_PT_PRESENT);
	memset(kernel_pde_v, 0, sizeof(kernel_pde_v));

	kernel_pde_v = BIT_PD_PAGE_SIZE | BIT_PD_PT_PRESENT | BIT_PD_PT_RW;
	pd_v[PAGE_DIRECTORY_INDEX(KERNEL_VIRTUAL_BASE)] = kernel_pde_v;

	// Set the new Page Directory officially
	set_cr3(pd_p);
	interrupt_restore(eflags);
}