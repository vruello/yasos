#include <stdint.h>

#include "drivers/io.h"
#include "kernel/interrupt_handlers.h"
#include "kernel/registers.h"

#define PIT_DEFAULT_FREQUENCY 1193180
#define PIT_COMMAND 0x43
#define PIT_DIVIDE_FREQUENCY 0x36
#define PIT_CH0 0x40

void pit__init(uint32_t frequency);
void timer_callback(registers_t regs);

void timer_callback(registers_t regs) {
    // do something
}

void pit__init(uint32_t frequency) {
    interrupt_handlers__register(IRQ0, &timer_callback);
    
    // The value we send to the PIT is the value to divide it's input clock
    // (PIT_DEFAULT_FREQUENCY Hz) by, to get our required frequency. Important 
    // to note is that the divisor must be small enough to fit into 16 bits
    
    uint16_t divisor = (uint16_t) PIT_DEFAULT_FREQUENCY;

    // Send the command byte
    outb(PIT_COMMAND, PIT_DIVIDE_FREQUENCY);

    // divisor has to be sent byte-wise, so split here into upper/lower bytes
    uint8_t low = (uint8_t) (divisor & 0xff);
    uint8_t high = (uint8_t) ((divisor >> 8) & 0xff);

    // send the frequency divisor
    outb(PIT_CH0, low);
    outb(PIT_CH0, high);
}
