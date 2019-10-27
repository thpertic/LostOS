#ifndef STRING_H
#define STRING_H

#include <system.h>

char *itoa(long int num, char *str, int base);
char* utoa(uint32_t num, char *str, int base);

size_t strlen (const char *s);
int32_t strcmp (const char *s1, const char *s2);

#endif