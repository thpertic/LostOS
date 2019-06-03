#include <multiboot.h>

#include <drivers/vga.h>
#include <tables/gdt.h>
#include <tables/idt.h>
#include <interrupts/timer.h>

#include <debug_utils/printf.h>
#include <debug_utils/serial.h>

// Guides I've followed
// https://wiki.osdev.org
// http://www.osdever.net/bkerndev/Docs/title.htm
// http://jamesmolloy.co.uk/tutorial_html/
// https://github.com/szhou42/osdev
// http://littleosbook.github.io/

/*** TODO: Working on the clock ***/

void kmain(multiboot_info_t* mbd, int n) {
    // Start all
    video_init();
    printf("Video initialized.\n\n");

    printf("GRUB: 0x%x.\n\n", n);

    printf("Initializing COM1...\n");
    init_serial();
    printfSerial("INIT COM1");
    printf("COM1 initialized.\n\n");

    printf("Initializing the GDT...\n");
    gdt_init();
    printf("GDT initialized.\n\n");
    
    printf("Initializing the IDT...\n");
    idt_init();
    printf("IDT initialized.\n\n");

    printf("Initializing the clock...\n");
    clock_init(50);
    printf("Clock initialized.\n\n");

    if (mbd->boot_device != NULL)
        printf("Boot device != NULL\n\n");

//  int num = 5 / 0;
//  asm("int $4");

    while (1);// __asm__ ("hlt");
}