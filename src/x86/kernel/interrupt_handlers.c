#include <stdint.h>
#include "kernel/interrupt_handlers.h"

static int_handler_t interrupt_handlers[256];

void interrupt_handlers__register(uint8_t num, int_handler_t handler) {
    interrupt_handlers[num] = handler;   
}

int_handler_t interrupt_handlers__get(uint8_t num) {
    return interrupt_handlers[num];
}

