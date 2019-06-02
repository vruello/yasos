#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/utils.h"
#include "drivers/vga.h"
#include "boot/descriptor_tables.h"
#include "drivers/pit.h"
#include "drivers/keyboard.h"
#include "drivers/pc_speaker.h"
#include "boot/multiboot.h"
#include "kernel/pmm.h"
#include "kernel/kmem.h"
#include "libk/stdio.h"
#include "kernel/vmm.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

#define KERNEL_OFFSET 0xC0000000

void kernel_main(multiboot_info_t* mbi);

void kernel_main(multiboot_info_t* mbi) {
    /* Initialize terminal interface */
    vga__initialize();

    /* Initialize stdio with vga primitives */
    stdio__init(vga__putchar, vga__writestring);
    
    /* Initialize bootstrap heap */
    kmem__bootstrap_init();
    
    /* Initialize Physical Memory Manager */
    // Translate mbi into its virtual address
    mbi = (multiboot_info_t*) ((char*) mbi + KERNEL_OFFSET);
    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        pmm__init((multiboot_memory_map_t*) ((char*) mbi->mmap_addr + KERNEL_OFFSET), mbi->mmap_length);
    }
    else {
        PANIC("No information about available memory.");
    }
    
    /* Initialize the descriptor tables */
    descriptor_tables__init();
    
    /* Initialize Virtual Memory Manager */
    vmm__init();
 
    /* Initialize real heap */
    kmem__init();  

    /* Initialize the PIT */
    pit__init(100);
    
    /* Initialize the keyboard */
    keyboard__init();
    
#if 1
    /* Play a welcome frightening sound */
    pc_speaker__play(340);
    for (volatile int i = 0; i < 100000000; i++);
    pc_speaker__stop();
    for (volatile int i = 0; i < 100000000; i++);
    pc_speaker__play(480);
    for (volatile int i = 0; i < 100000000; i++);
    pc_speaker__stop();
#endif
    
    uint32_t* test = kmem__alloc(sizeof(uint32_t) * 10, 0);
    printf("test: 0x%x\n", test);
    for (size_t i = 0; i < 10; i++) {
        test[i] = i;
        printf("test[%u] = %u\n", i, test[i]);
    }
    kmem__free(test);
    
    /* Infinite loop */
    for(;;) {
        __asm__ __volatile__ ("hlt");
    };

    PANIC("This should never happen...");
}
