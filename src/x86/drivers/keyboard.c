#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "drivers/keyboard.h"
#include "kernel/registers.h"
#include "drivers/vga.h"
#include "kernel/interrupt_handlers.h"
#include "drivers/io.h"

#define KEYBOARD_READ_PORT 0x60

#define SHIFT_LEFT 0x2A
#define SHIFT_RIGHT 0x36
#define CONTROL_LEFT 0x1D
#define ALT_LEFT 0x38
#define CAPS_LOCK 0x3A
#define ENTER_ASCII_CODE 13

void keyboard_callback(registers_t* regs);

static bool shift = false;
static bool alt = false;
static bool caps_lock = false;
static bool control = false;

// TODO : keyboard layout should be initialized by higher level component
char set1_to_ascii[] = {
    0, 27, // escape
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, // backspace
    '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
    ENTER_ASCII_CODE, 0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', '\'', 
    '`', 0, '\\', 'w', 'x', 'c', 'v', 'b', 'n', ',', '.', ':', '/', 
    0, 0, 0, ' '};

size_t set1_to_ascii_size = sizeof(set1_to_ascii);

void keyboard_callback(registers_t* regs) {
    // TODO : keyboard driver should take a higher level handler and gives
    // it a structure entry representing the key combination
    uint8_t value = inb(KEYBOARD_READ_PORT);
    bool pressed = true;
    if (value & 0x80) {
        // The key has been released
        pressed = false;
    }
    
    value &= 0x7f;

    if (value == SHIFT_LEFT || value == SHIFT_RIGHT) {
        shift = !shift;
    }
    else if (value == CONTROL_LEFT) {
        control = !control;
    }
    else if (value == ALT_LEFT) {
        alt = !alt;
    }
    else if (value == CAPS_LOCK && pressed) {
        caps_lock = !caps_lock;
    }
    else {
        // TODO : this should be done in higher level component
        // The keyboard driver should not deal with the VGA driver...
        if (pressed && value < set1_to_ascii_size && set1_to_ascii[value] != 0) {
            char out = set1_to_ascii[value];
            bool uppercase = false;
            if (shift)
                uppercase = !uppercase;
            if (caps_lock)
                uppercase = !uppercase;

            if (uppercase && out >= 'a' && out <= 'z') {
                out -= ('a' - 'A');
            }

            if (out == ENTER_ASCII_CODE) {
                out = '\n';
            }
            
            // printable character
            if ((out >= 33 && out <= 126) || out == ' ' || out == '\n' || out == '\b' || out == '\t') {
                vga__putchar(out);
            }
        }    
    }
    
}

void keyboard__init() {
    interrupt_handlers__register(IRQ1, keyboard_callback);
}
