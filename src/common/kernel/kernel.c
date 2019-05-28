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
#include "boot/multiboot.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void kernel_main(multiboot_info_t* mbi);
void parse_multiboot_info(multiboot_info_t* mbi);

void parse_multiboot_info(multiboot_info_t* mbi) {
    if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
        // the mem_* fields are valid
        vga__setcolor(VGA_COLOR_LIGHT_BLUE);
        vga__writestring("mem_lower: ");
        vga__writehex(mbi->mem_lower);
        vga__writestring(" ; mem_upper: ");
        vga__writehex(mbi->mem_upper);
        vga__writestring("\n\n");
        vga__setcolor(VGA_COLOR_LIGHT_GREY);
    }

    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        // the mmap_* fields are valid
        uint32_t size = 0;
        uint32_t total_phy_memory = 0;
        vga__setcolor(VGA_COLOR_LIGHT_GREEN);
        vga__writestring("mmap_addr: ");
        vga__writehex(mbi->mmap_addr);
        vga__writestring("\nmmap_length: ");
        vga__writedec(mbi->mmap_length);
        vga__writestring("\n\n");
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) (mbi->mmap_addr);
        while (size < mbi->mmap_length) {
            // vga__writestring("size: ");
            // vga__writedec(mmap->size);
            vga__writestring("addr_high: ");
            vga__writehex(mmap->addr_high);
            vga__writestring(" ; addr_low: ");
            vga__writehex(mmap->addr_low);
            vga__writestring(" ; len: ");
            vga__writehex(mmap->len_low);
            //vga__writestring("\nlen_high: ");
            //vga__writehex(mmap->len_high);
            vga__writestring(" ; type: ");
            vga__writedec(mmap->type);
            vga__putchar('\n');
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                total_phy_memory += mmap->len_low;
            }
            size += mmap->size + 4;
            mmap = (multiboot_memory_map_t*) (mbi->mmap_addr + size);
        }
        vga__writestring("\nAvailable memory: ");
        vga__writedec(total_phy_memory);
        vga__putchar('\n');
        vga__setcolor(VGA_COLOR_LIGHT_GREY);
    }
}

void kernel_main(multiboot_info_t* mbi) {
    /* Initialize terminal interface */
    vga__initialize();

    /* Parse multiboot info */
    parse_multiboot_info(mbi);

    /* Initialize the descriptor tables */
    descriptor_tables__init();

    /* Initialize paging */
    paging__init();

    /* Initialize the PIT */
    pit__init(100);
    
    /* Initialize the keyboard */
    keyboard__init();

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
