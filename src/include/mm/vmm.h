#ifndef VMM_H
#define VMM_H

#include <system.h>

#define BIT_PD_PT_PRESENT   0x00000001 // Present in memory?
#define BIT_PD_PT_RW        0x00000002 // Writable?
#define BIT_PD_PT_USER      0x00000004 // Accessible in user mode?
#define BIT_PD_PT_PWT       0x00000008 // Write through cache?
#define BIT_PD_PT_PCD       0x00000010 // Disable caching?
#define BIT_PD_PT_ACCESSED  0x00000020 // Frame/table was accessed.
#define BIT_PD_PT_DIRTY     0x00000040 // Frame/table was modified.
#define BIT_PD_PAGE_SIZE    0x00000080 // Is a 4MB page frame?

#define PD_VADDR 0xFFFFF000
#define PT_BASE_VADDR 0xFFC00000

#define PAGE_DIRECTORY_ADDR_OFFSET 22
#define PAGE_TABLE_ADDR_OFFSET 12

#define PAGE_DIRECTORY_INDEX(x) (((x) >> PAGE_DIRECTORY_ADDR_OFFSET) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> PAGE_TABLE_ADDR_OFFSET) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

void init_vmm();

bool vMapPage(void *phys, void *virt, uint32_t flags);
void vUnmapPage(void *virt);

extern void set_cr3(uint32_t *pd_phys_addr);

#endif