#include <multiboot.h>

#include <drivers/vga.h>
#include <tables/gdt.h>
#include <tables/idt.h>
#include <interrupts/timer.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

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

void kmain(uint32_t *pd, multiboot_info_t* mbd, int n) {
    // Start everything
    video_init();
    printf("VGA initialized.\n\n");

    printf("GRUB: 0x%x.\n\n", n);

    init_serial();
    printfSerial("INIT COM1\n");
    printf("COM1 initialized.\n\n");

    gdt_init();
    printf("GDT initialized.\n");
    idt_init();
    printf("IDT initialized.\n\n");

    clock_init(100);
    printf("Clock initialized.\n\n");

    pmm_init(mbd, pd);
    printf("PMM initialized.\n");
    vmm_init();
    printf("VMM initialized.\n\n");

//  int num = 5 / 0;
//  asm("int $4");

    while (1);// __asm__ ("hlt");
}