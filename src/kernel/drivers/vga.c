#include <drivers/vga.h>
#include <common/utility.h>

// Cursor x and y
int x = 0, y = 0;

// Background and foreground color
uint8_t attribs;

void init_video() {
    set_color(LIGHT_GREY, BLACK);
    clear();
}

/**
 * Clears the screen.
 * Simply put spaces in the video memory.
 */
void clear() {
    memsetw(SCREEN, PRINT((uint8_t)' ', LIGHT_GREY), WIDTH * HEIGHT / 2);
}

void try_scroll() {
    // If you shoudn't scroll returns immediately
    if (y < HEIGHT)
        return;

    // Move up a line
    void *upperScreen = (void *)SCREEN + 1 * WIDTH * 2;
    uint32_t size = y * WIDTH * 2;

    memcpy(SCREEN, upperScreen, size);
    
    // Clear the last line
    upperScreen = (void *)SCREEN + size;
    memsetw(upperScreen, PRINT(' ', LIGHT_GREY), WIDTH);
    y--;
}

/**
 * Moves the little blinking line under the last pressed char.
 * This sends a command to indicies 14 and 15 in the CRT Control Register of the VGA controller. 
 * These are the high and low bytes of the index that show where the hardware cursor is to be 'blinking'. 
 * To learn more, you should look up some VGA specific programming documents.
 */
void update_cursor() {
    uint32_t pos = y * WIDTH + x;

    outportb(0x3D4, 14);
    outportb(0x3D5, pos >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, pos);
}

/**
 * Prints a char.
 * Arrived at WIDTH of width goes newline, arrived at HEIGHT of height scrolls up.
 * Can handle \n, \t, \b.
 */
int putc(const char c) {
    try_scroll();

    if (c == '\n') {
        // Handle the newline 
        x = 0;
        y++;
    } else if (c == '\t') {
        // Handle the tab
        uint8_t i;
        for (i = 0; i < 4; i++) 
            putc(' ');
    } else if (c == '\b') {
        // Handle the backspace
        if (x > 0)
            x--;
    } else if (c >= ' ') {
        // Print the character
        PIXEL(x, y) = PRINT(c, get_color());
        x++;
        if (x == WIDTH) {
            x = 0;
            y++;
        }
    }
    update_cursor();
    return c;
}

/**
 * Loops through all characters and print them.
 * Should return EOF (\0).
 */
int puts(char *string) {
    while (*string != '\0') {
        putc(*string);
        string++;
    }
    return *string;
}

/**
 * Set current foreground and background color.
 * Top 4 bytes are the background, bottom 4 bytes are the foreground color.
 */
void set_color(uint8_t foreColor, uint8_t backColor) {
    attribs = (backColor << 4) | (foreColor & 0x0F);
}

uint8_t get_color() {
    return attribs;
}