#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "kernel/interrupt_handlers.h"
#include "libk/string.h"
#include "kernel/kmem.h"
#include "libk/bitset.h"
#include "kernel/paging.h"
#include "kernel/utils.h"
#include "drivers/vga.h"

#define FRAMES_BY_BITSET_ENTRY 32
#define FRAME_ALIGNMENT 0x1000
#define NO_AVAILABLE_FRAME 0xffffffff
#define PHYSICAL_MEMORY_SIZE 0x1000000
#define PAGE_FAULT_EXCEPTION 14

// it is defined in kernel/kmem.c
extern uint32_t placement_address;

static void paging__set_frame(uint32_t frame);
static void paging__clear_frame(uint32_t frame);
static bool paging__test_frame(uint32_t frame);
static uint32_t paging__first_frame_free(void);
static void paging__alloc_frame(page_t *page, bool is_kernel, bool is_writable);
static void paging__free_frame(page_t* page);

static uint32_t* frames_bitset;
static uint32_t frames_bitset_size;
static size_t nframes;
static page_directory_t* current_directory;
static page_directory_t* kernel_directory;

static inline void paging__set_frame(uint32_t frame) {
    bitset__set(frame, frames_bitset);
}

static inline void paging__clear_frame(uint32_t frame) {
    bitset__clear(frame, frames_bitset);
}

static inline bool paging__test_frame(uint32_t frame) {
    return bitset__test(frame, frames_bitset);
}

static uint32_t paging__first_frame_free() {
    size_t index, bit;
    bool found = bitset__find_first_clear(frames_bitset, frames_bitset_size, &index, &bit);
    if (!found) {
        // There is no available frame.
        // TODO : We should swap or something...
        // For now, return the maximum value which should be unused
        return NO_AVAILABLE_FRAME;
    }
    return index * FRAMES_BY_BITSET_ENTRY + bit;
}

/*
 * Function to allocate a frame
 */
static void paging__alloc_frame(page_t *page, bool is_kernel, bool is_writable) {
    if (page->frame != 0) {
        return; // Frame was already allocated
    }
    
    uint32_t frame = paging__first_frame_free();
    if (frame == NO_AVAILABLE_FRAME) {
        PANIC("No free frames!");
    }

    paging__set_frame(frame); // Mark the frame as used
    page->present = 1;
    page->writable = (is_writable) ? 1 : 0;
    page->user = (is_kernel) ? 0 : 1;
    page->frame = frame;
}

static void paging__free_frame(page_t* page) {
    if (page->frame == NULL) {
        return; // page didn't actually have an allocated frame
    }
    
    paging__clear_frame(page->frame);
    page->present = 0;
    page->frame = NULL;
}

void paging__init() {
    // Initiliaze frames bitset
    nframes = PHYSICAL_MEMORY_SIZE / FRAME_ALIGNMENT;
    frames_bitset_size = nframes / FRAMES_BY_BITSET_ENTRY;
    frames_bitset = (uint32_t*) kmem__alloc(frames_bitset_size);
    memset(frames_bitset, 0, frames_bitset_size);
     
    // Create a kernel page directory
    kernel_directory = (page_directory_t*) kmem__alloc_a(
            sizeof(page_directory_t), true);
    memset(kernel_directory, 0, sizeof(page_directory_t));

    // Identify map (phys addr = virt addr) from 0x0 to end of used memory
    // so we can access this transparently as if pagning wasn't enabled.
    size_t i = 0;
    while (i < placement_address) {
        paging__alloc_frame(paging__get_page(i, true, kernel_directory), true, true); 
        i += FRAME_ALIGNMENT;
    }

    // Register a handler for PAGE_FAULT exception
    interrupt_handlers__register(PAGE_FAULT_EXCEPTION, paging__fault_handler);
    
    // Enable paging
    paging__switch_page_directory(kernel_directory);
}

void paging__switch_page_directory(page_directory_t* dir) {
    current_directory = dir;
    __asm__ __volatile__("mov %0, %%cr3":: "r"(dir->tables_physical));
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    __asm__ __volatile__("mov %0, %%cr0":: "r"(cr0));
}

page_t* paging__get_page(uint32_t address, bool make, page_directory_t *dir) {
    address /= FRAME_ALIGNMENT;
    size_t table_index = address / 1024; // index of the pages table
    size_t page_index = address % 1024; // index of the page
    if (dir->tables[table_index]) {
        // the table already exists
        return &dir->tables[table_index]->pages[page_index];
    }
    else if(make) {
        uint32_t physic;
        // Allocate the table structure in virtual memory, while storing
        // its physical address in physic
        dir->tables[table_index] = (page_table_t*) kmem__alloc_ap(
                sizeof(page_table_t), true, &physic);
        memset(dir->tables[table_index], 0, sizeof(page_table_t));
        // Adding the physical address to dir->tables_physical. It is the PTE
        // read by the CPU.
        dir->tables_physical[table_index] = physic | 0x7; // PRESENT, RW, US
        return &dir->tables[table_index]->pages[page_index];
    }
    else {
        return NULL;
    }
}

void paging__fault_handler(registers_t* regs) {
    // The faulting address is stored in %cr2
    uint32_t addr;
    __asm__("mov %%cr2, %0":"=r"(addr));
    // The error code gives us details of what happened.
   int present   = !(regs->err_code & 0x1); // Page not present
   int writable = regs->err_code & 0x2;           // Write operation?
   int user = regs->err_code & 0x4;           // Processor was in user-mode?
   int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
   int id = regs->err_code & 0x10;          // Caused by an instruction fetch?
    
   vga__setcolor(VGA_COLOR_LIGHT_RED);
   vga__writestring("Page fault ( ");
   if (present)
        vga__writestring("not-present ");
   if (writable)
       vga__writestring("read-only ");
   if (user)
       vga__writestring("kernel-mode-only ");
   if (reserved) 
       vga__writestring("reserved ");
   vga__writestring(") at ");
   vga__writehex(addr);
   vga__writestring("\n");
   PANIC("Page fault");
}
