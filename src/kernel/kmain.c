#include <multiboot.h>

#include <drivers/vga.h>
#include <tables/gdt.h>
#include <tables/idt.h>
#include <interrupts/timer.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/kheap.h>

#include <debug_utils/printf.h>
#include <debug_utils/serial.h>

// TODO: Implement Kernel Heap.

// 0101 0100 == 'T'

// Guides I've followed:
// https://wiki.osdev.org
// http://www.osdever.net/bkerndev/Docs/title.htm
// http://jamesmolloy.co.uk/tutorial_html/
// http://littleosbook.github.io/
// http://www.brokenthorn.com/Resources/OSDevIndex.html

// Repos that helped me:
// https://github.com/AjayMT/mako
// https://github.com/szhou42/osdev

// A great start to graphics: 
// http://www.brackeen.com/home/vga 

/** \mainpage LostOS's README
 * 
 * This is a 32-bit operating system written from scratch to help me better understand what's "under the hood".
 * 
 * \section Getting Started
 * \subsection Prerequisites
 * 
 * You will need a cross-compiler and Bochs/Qemu. I advice to stay on Linux.
 * 
 * I'm using the GCC cross-compiler built from source following the guide on https://wiki.osdev.org/GCC_Cross-Compiler.
 * Then install `nasm`, `xorriso` and `bochs` with this command (if you are on a Debian-like distro like me):
 * \code
 * apt-get install nasm xorriso bochs
 * \endcode
 * 
 * \subsection Executing
 * 
 * Once you download the project, execute
 * \code
 * ./start.sh
 * \endcode
 * It should be enough if everything is right.
 * 
 * I recommend execute the scripts singularly, though, as you can see if one of them crashes.
 * The order is:
 * \code
 * ./compile.sh
 * ./build.sh
 * ./bochs.sh  or ./qemu.sh
 * \endcode
 * 
 * The emulator should start, print some initialization messages to the screen and write "INIT COM1" in the com1.out file.
 * 
 * \section Clear
 * If you want to rebuild everything you can just execute
 * \code
 * ./clean.sh
 * \endcode
 * 
 * which will remove every .o file and the .iso image. 
 * You can then follow the **Executing** part to re-build the OS.
 * 
 * \section Documentation
 * To document the code I'm using Doxygen.
 * Install it with:
 * \code
 * apt-get install doxygen 
 * \endcode
 * And then to generate code run:
 * \code
 * doxygen doxyfile
 * \endcode
 * 
 * \section Features
 * - Little documentation
 * - GRUB
 * - Video support (printf-like function)
 * - Serial port (COM1) support
 * - Custom GDT and IDT installed
 * - PIC remapped 
 * - IRQs and ISRs set
 * - PIT (Channel 0)
 * - Physical Memory Manager
 * - Virtual Memory Manager
 * 
 * \section Todos
 * 
 * - Kernel Heap
 * - Merge printf(): Print to a generic output that can be redirected
 * 
 * \section Problems
 * 
 * \section Dreams
 * 
 * - Good project organization (an *arch* directory, for example)
 * - ERRNO integration
 * - panic function integration
 * - Semantic versioning (as soon I get to the alpha release)
 * - Keyboard driver
 * - Multithreading
 * - Filesystem
 * - Graphical interface
 * - Network
 * - System calls
 * 
 * \section Author
 * 
 * 
 * Thomas Perticaroli - [thpertic](https://github.com/thpertic)
 * 
 * \section License
 * 
 * This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
 */

/**
 * Main function of the kernel.
 * This initialize every part of the os.
 * 
 * @param pd The physical address of the first Page Directory.
 * @param mbd The pyhsical address of GRUB's multiboot structure.
 * @param n GRUB's number to verify that everything works correctly.
 */
void kmain(uint32_t *pd, multiboot_info_t* mbd, int n) {
    init_video();
    printf("VGA initialized.\n\n");

    printf("GRUB: 0x%x.\n\n", n);

    init_serial();
    printfSerial("INIT COM1\n");
    printf("COM1 initialized.\n\n");

    init_gdt();
    printf("GDT initialized.\n");
    init_idt();
    printf("IDT initialized.\n\n");

    init_clock(100);
    printf("Clock initialized.\n\n");

    init_pmm(mbd, pd);
    printf("PMM initialized.\n");
    init_vmm();
    printf("VMM initialized.\n\n");

    init_kheap();
    printf("Kernel heap initialized\n\n");

//  int num = 5 / 0;
//  asm("int $4");

    while (1);// __asm__ ("hlt");
}