#include <stddef.h>

#include "kernel/utils.h"
#include "drivers/vga.h"
#include "drivers/pc_speaker.h"

void panic(const char* str, const char* filename, size_t line) {
    vga__setcolor(VGA_COLOR_RED);
    vga__writestring("\nKERNEL PANIC at ");
    vga__writestring(filename);
    vga__writestring(" line ");
    vga__writedec((uint32_t)line);
    vga__writestring("\nReason: ");
    vga__writestring(str);
    
    // Play a very (very) annoying sound
    pc_speaker__play(880);

    // disable interrupts 
    __asm__ __volatile__ ("cli");

    // End with an infinite loop
    for(;;) {
        __asm__ __volatile__ ("hlt");
    }
}
