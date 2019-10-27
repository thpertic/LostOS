#include <common/utility.h>

/**
 * Assembly instructions used:
 *      cld - clears direction flag;
 *      rep stosb - Store AL at address ES:(E)DI + rep CX times
 * 
 * The direction flag is used to influence the direction 
 * in which string instructions offset pointer registers.
 * 
 */

/**
 * Set 'n' bytes in 'dest' from 'c'.
 * Return 'dest' 
 */
void *memset (void *dest, int32_t c, size_t n) {
  asm volatile("cld; rep stosb"
    : "=c"((int){0})
    : "D"(dest), "a"(c), "c"(n)
    : "flags", "memory");

  return dest;
}

/**
 * Set 'n' bytes in 'dest' from 'c'.
 * Return 'dest'.
 * Takes a word as c. 
 */
void *memsetw(void *dest, int32_t c, size_t n) {
  asm volatile("cld; rep stosw"
    : "=c"((int){0})
    : "D"(dest), "a"(c), "c"(n)
    : "flags", "memory");

  return dest;
}

/**
 * Copy 'n' bytes of data from 'src' to 'dest',
 * finally return 'dest'.
 */
void *memcpy (void *dest, const void *src, size_t n) {
  asm volatile ("cld; rep movsb"
    : "=c"((int){0})
    : "D"(dest), "S"(src), "c"(n)
    : "flags", "memory");

  return dest;
}