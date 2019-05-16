#include <stdint.h>
#include <stddef.h>

#include "kernel/registers.h"
#include "drivers/io.h"
#include "kernel/interrupt_handlers.h"

#define PIC1_COMMAND 0x20
#define PIC2_COMMAND 0xA0
#define PIC_EOI 0x20

void irq__handler(registers_t);

void irq__handler(registers_t regs) {

    // Send an EOI (End Of Interrupt) signal to the PICs.
    
    // Send reset signal to master
    outb(PIC1_COMMAND, PIC_EOI);

    // If this inrrupt involved the slave
    if (regs.int_no >= 40) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    
    int_handler_t handler = interrupt_handlers__get((uint8_t) regs.int_no); 
    if (handler != NULL) {
        handler(regs);
    }
}
