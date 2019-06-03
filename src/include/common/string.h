#ifndef STRING_H
#define STRING_H

#include <system.h>

uint8_t *memcpy(void *dest, const void *src, uint32_t count);
uint8_t *memset(void *dest, uint8_t val, uint32_t count);
uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count);

char *itoa(long int num, char *str, int base);
char* utoa(uint32_t num, char *str, int base);

int strlen(const char *str);

#endif