#include <multiboot.h>

#include <drivers/vga.h>
#include <tables/gdt.h>
#include <tables/idt.h>
#include <interrupts/timer.h>
#include <mm/pmm.h>

#include <debug_utils/printf.h>
#include <debug_utils/serial.h>

// TODO: Implement PMM.

// 0101 0100 == 'T'

// Guides I've followed
// https://wiki.osdev.org
// http://www.osdever.net/bkerndev/Docs/title.htm
// http://jamesmolloy.co.uk/tutorial_html/
// https://github.com/szhou42/osdev
// http://littleosbook.github.io/
// http://www.brokenthorn.com/Resources/OSDev1.html

void kmain(multiboot_info_t* mbd, int n) {
    // Start everything
    video_init();
    printf("Video initialized.\n\n");

    printf("GRUB: 0x%x.\n\n", n);

    printf("Initializing COM1...\n");
    init_serial();
    printfSerial("INIT COM1\n");
    printf("COM1 initialized.\n\n");

    printf("Initializing the GDT...\n");
    gdt_init();
    printf("GDT initialized.\n\n");
    
    printf("Initializing the IDT...\n");
    idt_init();
    printf("IDT initialized.\n\n");

    printf("Initializing the clock...\n");
    clock_init(100);
    printf("Clock initialized.\n\n");

    printf("Initializing the Physical Memory Manager...\n");
    pmm_init(mbd);
    printf("PMM initialized.\n\n");

//  int num = 5 / 0;
//  asm("int $4");

    while (1);// __asm__ ("hlt");
}