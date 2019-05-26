#include <stdint.h>
#include "drivers/vga.h"
#include "kernel/registers.h"
#include "kernel/interrupt_handlers.h"

void isr__handler(registers_t* regs);

void isr__handler(registers_t* regs) {
    int_handler_t handler = interrupt_handlers__get((uint8_t) regs->int_no);
   if (handler != NULL) {
       handler(regs);
   }
}
