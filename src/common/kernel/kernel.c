#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "drivers/vga.h"
#include "boot/descriptor_tables.h"
#include "drivers/pit.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void kernel_main(void);

void kernel_main(void) {
    /* Initialize the descriptor tables */
    descriptor_tables__init();

    /* Initialize terminal interface */
	vga__initialize();

    /* Initialize the PIT */
    pit__init(100);
    
    /* Initialize the keyboard */
    keyboard__init();
   
    /* Infinite loop */
    while(1);
}
