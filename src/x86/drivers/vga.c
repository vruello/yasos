#include <stdint.h>
#include <stddef.h>
#include "drivers/vga.h"
#include "libk/string.h"
#include "drivers/io.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDR 0xC00B8000
#define VGA_CONTROL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5
#define VGA_HIGH_CURSOR_BYTE 14
#define VGA_LOW_CURSOR_BYTE 15
#define VGA_TAB_SIZE 4

static inline uint8_t vga__entry_color(enum vga_color fg, enum vga_color bg);
static inline uint16_t vga__entry(char uc, uint8_t color);
static void vga__move_cursor(void);
static void vga__scroll(void);
static void vga__deletechar(void);
static size_t vga__get_index(uint8_t column, uint8_t row);
static uint16_t vga__get_entry(uint8_t column, uint8_t row);

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

static size_t vga__get_index(uint8_t column, uint8_t row) {
    return (size_t) (column + VGA_WIDTH * row);
}

static uint16_t vga__get_entry(uint8_t column, uint8_t row) {
    size_t index = vga__get_index(column, row);
    return terminal_buffer[index];
}

void vga__putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga__entry(c, color);
}

static void vga__deletechar() {
    if (terminal_column > 0) {
        terminal_column--;
        vga__putentryat(' ', terminal_color, terminal_column, terminal_row);
    }
    else if (terminal_row > 0) {
        terminal_row--;
        terminal_column = VGA_WIDTH - 1;
        while (terminal_column > 0 && (vga__get_entry(terminal_column, terminal_row) & 0xff) == ' ') {
            terminal_column--;
        }
        if (terminal_column > 0) {
            terminal_column++;
        }
    
    }
    vga__move_cursor();
    return;
}

void vga__putchar(char c) 
{
    if (c == '\n') {
        terminal_column = VGA_WIDTH - 1;
    }
    else if (c == '\t') {
        uint8_t spaces_number = (uint8_t) VGA_TAB_SIZE - (uint8_t) (terminal_column % VGA_TAB_SIZE);
        uint8_t next_term_column = (uint8_t) (terminal_column + spaces_number);
        if (next_term_column < VGA_WIDTH) {
            for (size_t i = 0; i < spaces_number; i++) {
                vga__putchar(' ');
            }
        }
        return;
    }
    else if (c == '\b') {
       vga__deletechar();
       return;
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
