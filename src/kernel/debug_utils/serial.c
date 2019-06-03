#include <debug_utils/serial.h>
#include <common/string.h>

#include <system.h>

/**
 * Initialize the serial port COM1.
 */
void init_serial() {
    outportb(SERIAL_COM1_BASE + 1, 0x00);                           // Disable all interrupts
    outportb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1_BASE), 0x80);     // Enable DLAB (set baud rate divisor)
    outportb(SERIAL_COM1_BASE, 0x03);                               // Set divisor to 3 (lo byte) 38400 baud
    outportb(SERIAL_COM1_BASE + 1, 0x00);                           //                  (hi byte)
    outportb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1_BASE), 0x03);     // 8 bits, no parity, one stop bit
    outportb(SERIAL_FIFO_COMMAND_PORT(SERIAL_COM1_BASE), 0xC7);     // Enable FIFO, clear them, with 14-byte threshold
    outportb(SERIAL_MODEM_COMMAND_PORT(SERIAL_COM1_BASE), 0x0B);    // IRQs enabled, RTS/DSR set

    
}

int serialReceived() {
    return inportb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1_BASE)) & 0x1;
}

/**
 * Read from the serial port COM1.
 */
char serialRead() {
    // Wait until the port can be read
    while(serialReceived() == 0);

    return inportb(SERIAL_COM1_BASE);
}

int canTransmit() {
    return inportb(SERIAL_LINE_STATUS_PORT(SERIAL_COM1_BASE)) & 0x20;
}

/**
 * Write to the serial port COM1.
 */
void serialWrite(char c) {
    // Wait until the port is ready to be written
    while (canTransmit() == 0);

    outportb(SERIAL_COM1_BASE, c);
}

/**
 * Write a string to the serial port.
 */
int serialWriteString(const char *string) {
    while (*string != '\0') {
        serialWrite(*string);
        string++;
    }
    return *string;
}

int printfSerial(const char *format, ...) {
    char *s;

    va_list list;
    va_start(list, format);

    while(*format != '\0') {
        if (*format == '%') {
            // Handle the format specifier
            format++;
            switch (*format) {
                // Character 
                case 'c':
                    serialWrite((char)va_arg(list, int));
                    break;

                // String
                case 's':
                    s = (char *)va_arg(list, int);
                    serialWriteString(s);
                    break;

                // Hexadecimal
                case 'x':
                    s = utoa((uint64_t)va_arg(list, int), s, 16);
                    serialWriteString(s);
                    break;

                // Decimal
                case 'd':
                    s = itoa((int)va_arg(list, int), s, 10);
                    serialWriteString(s);
                    break;

                // Unsigned
                case 'u':
                    s = utoa((uint64_t)va_arg(list, int), s, 10);
                    serialWriteString(s);
                    break;

                // Binary
                case 'b':
                    s = utoa((uint64_t)va_arg(list, int), s, 2);
                    serialWriteString(s);
                    break;
                    
                default:
                    return -1;
            }
            format++;
        } else
            serialWrite(*format++);            
    }
    va_end(list);

    return 1;
}