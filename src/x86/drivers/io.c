#include <stdint.h>
#include "drivers/io.h"

void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}
