#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <system.h>

void *memset(void *dest, int32_t c, size_t n);
void *memsetw(void *dest, int32_t c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

#endif