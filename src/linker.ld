/**
 * The bootloader will look at this image and start execution at the symbol
 * designated at the entry point. 
 */
ENTRY(setup)
OUTPUT_FORMAT(elf32-i386)

KERNEL_VIRTUAL_BASE = 0xC0000000;

SECTIONS {
    /**
     * The real kernel binary starts at 1MB.
     */
    . = 0x00100000;

    /**
     * Set a section for the lower half code.
     */
    .lowerhalf ALIGN(4K) : {
        *(.lowerhalf.data)
        *(.lowerhalf.text)
    }

    /** 
     * Kernel binary starts at 3GB + 1MB.
     * This is a virtual memory address and now all the variables, code labels 
     * are going to be referenced based on this.
     */
    . += KERNEL_VIRTUAL_BASE;

    /**
     * *(.section name) means: all sections named "section name" in all input object files, please put them here.
     * .multiboot is a section defined in entry.asm, it contains information for grub to find where our kernel entry point is.
     * Use keyword AT to relocate text section, same for rodata, data and bss section.
     */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE) {
        *(.text)
    }
    .data ALIGN (4K) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE) {
        *(.data)
        *(.rodata*)
    }

    /**
     * .bss is also data section, but it contains uninitialized data
     * .common is also uninitialized data section
     */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE) {
        _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
    }

    /DISCARD/ : {
        *(.eh_frame);
        *(.comment*);
    }

    /**
     * Put a symbol end here, it tells us where all the kernel code/data ends, it means everything after 'end' can be used for something else
     */
    end = .;
}
