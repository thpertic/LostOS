#include <system.h>

/**
 * We will use this later on for reading from the I/O ports to get data
 * from devices such as the keyboard. We are using what is called
 * 'inline assembly' in these routines to actually do the work 
 */
uint8_t inportb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/** 
 * We will use this to write to I/O ports to send bytes to devices. This
 * will be used for changing the textmode cursor position. 
 * Again, we use some inline assembly for the stuff that simply
 * cannot be done in C 
 */
void outportb (uint16_t _port, uint8_t _data) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}