; multiboot headers constants
MBALIGN  equ 1<<0                                         ; align loaded modules on page boundaries
MEMINFO  equ 1<<1                                         ; provide memory map
FLAGS    equ MBALIGN | MEMINFO                            ; this is the Multiboot 'flag' field
MAGIC    equ 0x1BADB002                                   ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)                             ; checksum of above, to prove we are multiboot

; Some constants to load the higher half kernel
KERNEL_VIRTUAL_BASE equ 0xC0000000
PDE_INDEX   equ (KERNEL_VIRTUAL_BASE >> 22)

; Paging bits
PSE_BIT     equ 0x00000010
PG_BIT      equ 0x80000000

section .lowerhalf.data
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

align 4096
global BootPageDirectory
BootPageDirectory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:  
    ;   - bit 7: PS The kernel page is 4MB.
    ;   - bit 1: RW The kernel page is read/write.
    ;   - bit 0: P  The kernel page is present (in RAM).
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is enabled 
    ; because it can't fetch the next instruction! It's ok to unmap this page later.
    ; DON'T UNMAP IT as it is used in the Physical Memory Manager before setting up the real Page Directory!
    dd 0x00000083
    times(PDE_INDEX - 1) dd 0

    ; This page directory defines a 4MB page containing the kernel
    dd 0x00000083
    times(1024 - PDE_INDEX - 1) dd 0

section .lowerhalf.text progbits alloc exec nowrite align=16
global setup
setup:
    ; Load PDBR (CR3), it must contains the physical address of the Page Directory
    mov ecx, BootPageDirectory
    mov cr3, ecx

    ; Set PSE bit in CR4 to enable 4MB pages
    mov ecx, cr4
    or ecx, PSE_BIT
    mov cr4, ecx

    ; Set PG bit in CR0 to enable paging
    mov ecx, cr0
    or ecx, PG_BIT
    mov cr0, ecx

    ; Start fetching instructions in kernel space.
    ; Since 'eip' at this point holds physical address of this command (approximately 0x00100000)
    ; we need to do a long jump to the correct virtual address of 
    ; startInHigherHalf which is approximately in 0xC0100000.
    ; lea ecx, [BootPageDirectory]
    jmp startInHigherHalf

section .text
startInHigherHalf:
    ; Unmap the first 4MB physical memory, we don't need it anymore. Flush the tlb, too
    ;mov dword[BootPageDirectory], 0
    ;invlpg[0]

    ; Set the stack
    mov esp, start_stack
    
    ; Map the multiboot structure to higher half
    add ebx, KERNEL_VIRTUAL_BASE

    ; Pass the Multiboot magic number and the Multiboot info structure
    push eax
    push ebx
    
    ; Call the C++ global constructor      
    ; call _init

    extern kmain
    call kmain

    ; Hang if kmain unexpectedly returns
    cli
    l:  hlt
        jmp l
        
section .bss nobits
align 4
end_stack:
    ; 1024 * 1024 = 104856 (1MB)
    resb 104856
start_stack: