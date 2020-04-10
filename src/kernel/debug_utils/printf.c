#include <debug_utils/printf.h>

/**
 * Prints to the screen.
 * 
 * @param format String to format.
 * @param ... List of arguments.
 * 
 * @return If everything went OK.
 */
int printf(const char *format, ...) {
    // Using the eyeball rule: 3 bytes of output for each byte of input. 
    // It is almost never a problem to allocate a little too much memory.
    char buf[sizeof(int) * 3 + 2]; // 1 for the sign and 1 for the null char at the end
    char *s = buf;

    va_list list;
    va_start(list, format);

    while(*format != '\0') {
        if (*format == '%') {
            // Handle the format specifier
            format++;
            switch (*format) {
                // Character 
                case 'c':
                    putc((char)va_arg(list, int));
                    break;

                // String
                case 's':
                    s = (char *)va_arg(list, int);
                    puts(s);
                    break;

                // Hexadecimal
                case 'x':
                    s = utoa((uint64_t)va_arg(list, int), s, 16);
                    puts(s);
                    break;

                // Decimal
                case 'd':
                    s = itoa((int)va_arg(list, int), s, 10);
                    puts(s);
                    break;

                // Unsigned
                case 'u':
                    s = utoa((uint64_t)va_arg(list, int), s, 10);
                    puts(s);
                    break;

                // Binary
                case 'b':
                    s = utoa((uint64_t)va_arg(list, int), s, 2);
                    puts(s);
                    break;
                    
                default:
                    return -1;
            }
            format++;
        } else
            putc(*format++);            
    }
    va_end(list);

    return 1;
}