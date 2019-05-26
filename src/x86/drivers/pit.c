#include <stdint.h>

#include "drivers/pit.h"
#include "drivers/io.h"
#include "kernel/interrupt_handlers.h"
#include "kernel/registers.h"
#include "drivers/vga.h"

#define PIT_DEFAULT_FREQUENCY 1193180
#define PIT_COMMAND 0x43
#define PIT_SQUARE_WAVE_MODE 0x36  // 0b00110110

#define PIT_CH0_PORT 0x40
#define PIT_CH1_PORT 0x41
#define PIT_CH2_PORT 0x42

void timer_callback(registers_t* regs);


static uint8_t io_ports[] = {PIT_CH0_PORT, PIT_CH1_PORT, PIT_CH2_PORT};
static uint32_t tick = 0;

void timer_callback(registers_t* regs) {
    tick++;
    // vga__writestring("Tick ");
    // vga__writedec(tick);
    // vga__putchar('\n');
}

void pit__init(uint32_t frequency) {
    interrupt_handlers__register(IRQ0, &timer_callback);
    
    pit__set_frequency(PIT_CH0, frequency);
}

void pit__set_frequency(pit__channel_t channel, uint32_t frequency) {
    // The value we send to the PIT is the value to divide it's input clock
    // (PIT_DEFAULT_FREQUENCY Hz) by, to get our required frequency. Important 
    // to note is that the divisor must be small enough to fit into 16 bits
    
    uint16_t divisor = (uint16_t) (PIT_DEFAULT_FREQUENCY / frequency);

    // Send the command byte
    uint8_t cmd = (uint8_t) (PIT_SQUARE_WAVE_MODE) | (uint8_t) (channel << 6);
    outb(PIT_COMMAND, cmd);

    // divisor has to be sent byte-wise, so split here into upper/lower bytes
    uint8_t low = (uint8_t) (divisor & 0xff);
    uint8_t high = (uint8_t) ((divisor >> 8) & 0xff);
    
    // send the frequency divisor
    outb(io_ports[channel], low);
    outb(io_ports[channel], high);
}
