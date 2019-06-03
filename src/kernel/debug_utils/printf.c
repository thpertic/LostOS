#include <debug_utils/printf.h>

int printf(const char *format, ...) {
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