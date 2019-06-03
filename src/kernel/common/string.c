#include <common/string.h>

/**
 * Copy 'count' bytes of data from 'src' to
 * 'dest', finally return 'dest' 
 */
uint8_t *memcpy(void *dest, const void *src, uint32_t count) {
    char *d = dest;
    const char *s = src;

    while (count--)
        *d++ = *s++;

    return dest;
}
/**
 * Set 'count' bytes in 'dest' to 'val'.
 * Again, return 'dest' 
 */
uint8_t *memset(void *dest, uint8_t val, uint32_t count) {
    unsigned char *tmp = dest;
    while (count > 0) {
        *tmp++ = val;
        count--;
    }

    return dest;
}

/**
 * Same as above, but this time, we're working with a 16-bit
 * 'val' and dest pointer. Your code can be an exact copy of
 * the above, provided that your local variables if any, are unsigned short 
 */
uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count) {

    unsigned int i;
    for (i = 0; i < count; i++)
        *dest++ = val;

    return dest;
}

void swap(char *a, char *b) {
    int c = *a;
    *a = *b;
    *b = c;
}

/* A utility function to reverse a string  */
void reverse(char str[], int length) { 
    int start = 0; 
    int end = length -1; 
    while (start < end) {     
        swap((str + start), (str + end)); 
        start++; 
        end--; 
    } 
}

/**
 * An implementation of the famous itoa() function.
 */
char* itoa(long int num, char *str, int base) {
    int i = 0; 
    bool isNegative = false; 
    
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 
  
    // In standard itoa(), negative numbers are handled only with  
    // base 10. Otherwise numbers are considered unsigned. 
    if (num < 0 && base == 10) { 
        isNegative = true; 
        num = -num; 
    }
    
    // Process individual digits 
    while (num != 0) { 
        int rem = num % base; 
        
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base; 
    } 
     
    // If number is negative, append '-' 
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0'; // Append string terminator 
  
    // Reverse the string 
    reverse(str, i); 
  
    return str; 
}

/**
 * Variant of itoa(): translate unsigned numbers
 */
char* utoa(uint32_t num, char *str, int base) {
    int i = 0; 
    
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    }

    // Process individual digits 
    while (num != 0) { 
        int rem = num % base; 
        
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base; 
    }  
    str[i] = '\0'; // Append string terminator 

    // Reverse the string 
    reverse(str, i); 

    return str; 
}

/**
 * This loops through character array 'str', returning how
 * many characters it needs to check before it finds a 0.
 * In simple words, it returns the length in bytes of a string 
 */
int strlen(const char *str) {
    unsigned int i = 0;
    for(i = 0; str[i] != '\0'; i++);

    return i;
}