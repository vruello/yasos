#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel/utils.h"
#include "drivers/vga.h"
#include "boot/descriptor_tables.h"
#include "drivers/pit.h"
#include "drivers/keyboard.h"
#include "drivers/pc_speaker.h"
#include "kernel/paging.h"

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
   
    /* Initialize paging */
    paging__init();

    /* Play a welcome frightening sound */
    pc_speaker__play(340);
    for (volatile int i = 0; i < 100000000; i++);
    pc_speaker__stop();
    for (volatile int i = 0; i < 100000000; i++);
    pc_speaker__play(480);
    for (volatile int i = 0; i < 100000000; i++);
    pc_speaker__stop();

    /* Infinite loop */
    for(;;) {
        __asm__ __volatile__ ("hlt");
    };

    PANIC("This should never happen...");
}
