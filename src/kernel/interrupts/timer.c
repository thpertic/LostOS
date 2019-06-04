#include <interrupts/timer.h>
#include <interrupts/irqs.h>

#include <debug_utils/printf.h>
#include <debug_utils/serial.h>

/**
 * The Programmable Interval Timer is a chip connected to IRQ0. 
 * It can interrupt the CPU at a user-defined rate (between 18.2Hz and 1.1931 MHz). 
 * The PIT is the primary method used for implementing a system clock and the only method available for implementing multitasking (switch processes on interrupt).
 *
 * The PIT has an internal clock which oscillates at approximately 1.1931MHz. 
 * This clock signal is fed through a frequency divider, to modulate the final output frequency. 
 * It has 3 channels, each with it's own frequency divider:
 *     - Channel 0 is the most useful. Its output is connected to IRQ0.
 *     - Channel 1 is very un-useful and on modern hardware is no longer implemented. It used to control refresh rates for DRAM.
 *     - Channel 2 controls the PC speaker.
 */

uint32_t tick = 0;

void tickHandler(regs_t *r) {
    if (r->int_no == 0)
        printfSerial("%d\n", r->int_no);
    
    tick++;
    outportb(0x20, 0x20); // End of interrupt
}

void clock_init(uint32_t frequency) {
    // Set the timer as the first IRQ
    irq_installHandler(IRQ0, &tickHandler);

    // The value sent to the PIT is the value to divide its input clock
    // (1193180 Hz) by, to get the required frequency. 
    // Important to note is that the divisor must be small enough to fit into 16-bits.
    uint16_t divisor = (uint16_t) (1193180 / frequency);

    // The divisor must be sent 8 bits at a time
    uint8_t low = (uint8_t) (divisor & 0xFF);
    uint8_t high = (uint8_t) ((divisor >> 8) & 0xFF);

    // Send the command byte
    outportb(0x43, 0x36);

    // Send the frequency divisor
    outportb(0x40, low);
    outportb(0x40, high);
}