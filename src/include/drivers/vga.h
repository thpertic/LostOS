#ifndef VGA_H
#define VGA_H

#include <system.h>
#include <common/string.h>

#define BLACK          0
#define BLUE           1
#define GREEN          2
#define CYAN           3
#define RED            4
#define MAGENTA        5
#define BROWN          6
#define LIGHT_GREY     7
#define DARK_GREY      8
#define LIGHT_BLUE     9
#define LIGHT_GREEN   10
#define LIGHT_CYAN    11
#define LIGHT_RED     12
#define LIGHT_MAGENTA 13
#define LIGHT_BROWN   14
#define WHITE         15

// VGA 80 * 25 screen
#define WIDTH 80
#define HEIGHT 25

// Video memory
#define SCREEN ((uint16_t *)(KERNEL_VIRTUAL_BASE + 0xB8000))

// Get/set a pixel through the video memory
#define PIXEL(x, y) SCREEN[y * WIDTH + x]

// Print in the video memory
#define PRINT(character, attribs) (((attribs & 0x0F) << 8) | (character & 0xFF))

void init_video();

int puts(char* string);
int putc(const char c);

void set_color(uint8_t foreColor, uint8_t backColor);
uint8_t get_color();

void clear();

#endif