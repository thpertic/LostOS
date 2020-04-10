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
 * Set 'n' bytes in 'dest' as 'c'.
 * 
 * @param dest Pointer to the destination buffer.
 * @param c Char to copy.
 * @param n Times to copy c.
 * 
 * @return The destination buffer.
 */
void *memset (void *dest, int32_t c, size_t n) {
  asm volatile("cld; rep stosb"
    : "=c"((int){0})
    : "D"(dest), "a"(c), "c"(n)
    : "flags", "memory");

  return dest;
}

/**
 * Set 'n' bytes in 'dest' as 'c'.
 * 
 * @param dest Pointer to the destination buffer.
 * @param c Char (as a word) to copy.
 * @param n Times to copy.
 * 
 * @return The destination buffer.
 */
void *memsetw(void *dest, int32_t c, size_t n) {
  asm volatile("cld; rep stosw"
    : "=c"((int){0})
    : "D"(dest), "a"(c), "c"(n)
    : "flags", "memory");

  return dest;
}

/**
 * Copy 'n' bytes of data from 'src' to 'dest'.
 * 
 * @param dest Pointer to the destination buffer. 
 * @param src Pointer to the source buffer.
 * @param n Times to copy c.
 * @return The destination buffer
 */
void *memcpy (void *dest, const void *src, size_t n) {
  asm volatile ("cld; rep movsb"
    : "=c"((int){0})
    : "D"(dest), "S"(src), "c"(n)
    : "flags", "memory");

  return dest;
}