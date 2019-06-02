#include <stddef.h>

#include "kernel/utils.h"
#include "drivers/vga.h"
#include "drivers/pc_speaker.h"
#include "libk/stdio.h"

void debug(const char* str, ...) {
#if DEBUG
    va_list ap;
    va_start(ap, str);    
    vga__setcolor(VGA_COLOR_LIGHT_BROWN);
    char buf[256];
    vsnprintf(buf, 256, str, ap);
    printf("[DEBUG] %s\n", buf);    
    vga__setcolor(VGA_COLOR_LIGHT_GREY);
    va_end(ap);
#endif
}

void panic(const char* str, const char* filename, size_t line) {
    vga__setcolor(VGA_COLOR_RED);
    printf("\nKERNEL PANIC at %s line %u\nReason : %s\n", filename, (unsigned int) line, str);
    
    // Play a very (very) annoying sound
    pc_speaker__play(880);

    // disable interrupts 
    __asm__ __volatile__ ("cli");

    // End with an infinite loop
    for(;;) {
        __asm__ __volatile__ ("hlt");
    }
}
