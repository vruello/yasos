#include <stdint.h>
#include <stddef.h>
#include "drivers/vga.h"
#include "libk/string.h"
#include "drivers/io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDR 0xB8000
#define VGA_CONTROL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5
#define VGA_HIGH_CURSOR_BYTE 14
#define VGA_LOW_CURSOR_BYTE 15

static inline uint8_t vga__entry_color(enum vga_color fg, enum vga_color bg);
static inline uint16_t vga__entry(char uc, uint8_t color);
static void vga__move_cursor(void);
static void vga__scroll(void);

static uint8_t terminal_row;
static uint8_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

static inline uint8_t vga__entry_color(enum vga_color fg, enum vga_color bg) 
{
	return (uint8_t) (fg | bg << 4);
}
 
static inline uint16_t vga__entry(char uc, uint8_t color) 
{
	return (uint16_t) (((uint16_t) uc) | ((uint16_t) color) << 8);
}
 
static void vga__move_cursor()
{
    uint16_t location = (uint16_t) (terminal_row * VGA_WIDTH + terminal_column); 
    outb(VGA_CONTROL_PORT, VGA_HIGH_CURSOR_BYTE); // Tell VGA board we are setting the high cursor byte
    outb(VGA_DATA_PORT, (uint8_t) (location >> 8)); // set the high cursor byte
    outb(VGA_CONTROL_PORT, VGA_LOW_CURSOR_BYTE); // Tell VGA board we are setting the low cursor byte
    outb(VGA_DATA_PORT, (uint8_t) location);
}

static void vga__scroll() 
{
    // Save terminal color
    uint8_t old_terminal_color = terminal_color;
    terminal_color = vga__entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Move buffer memory
    memmove((void*) VGA_ADDR, (void*) (VGA_ADDR + VGA_WIDTH * 2), VGA_WIDTH * (VGA_HEIGHT - 1) * 2);
    
    // Empty the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga__entry(' ', terminal_color);
    }

    // Restore terminal color
    terminal_color = old_terminal_color;
    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
}

void vga__initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga__entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) VGA_ADDR;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga__entry(' ', terminal_color);
		}
	}
}
 
void vga__setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void vga__putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga__entry(c, color);
}

void vga__putchar(char c) 
{
    if (c == '\n') {
        terminal_column = VGA_WIDTH - 1;
    }
    else {
    	vga__putentryat(c, terminal_color, terminal_column, terminal_row);
    }

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			vga__scroll();
	}

    vga__move_cursor();
}
 
void vga__write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
	    vga__putchar(data[i]);
}
 
void vga__writestring(const char* data) 
{
	vga__write(data, strlen(data));
}

void vga__writedec(uint32_t n) {
    uint8_t digits[10]; // 10 is the maximal number of digits that
                    // the decimal repr of an unsigned 32 bits
                    // can have
    size_t i = 0;

    if (n == 0) {
        digits[i++] = 0;
    }

    while (n > 0) {
        digits[i] = (uint8_t) (n % 10); 
        n /= 10;
        i++;
    }

    for (; i > 0; i--) {
        vga__putchar((char) (0x30 + digits[i-1]));
    }
}
