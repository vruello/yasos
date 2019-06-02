#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "kernel/interrupt_handlers.h"
#include "kernel/utils.h"
#include "drivers/vga.h"
#include "libk/stdio.h"
#include "kernel/vmm.h"
#include "kernel/pmm.h"

#define PAGE_FAULT_EXCEPTION 14
#define KERNEL_HEAP_BASE 0xD0000000
#define PAGE_SIZE 4096
#define PT_ENTRIES_NUMBER 1024
#define PD_ENTRIES_NUMBER 1024
#define ADDR_PD_BASE 0xfffff000
#define ADDR_PT_BASE 0xffc00000

#define PAGE_PRESENT 1
#define PAGE_WRITABLE 2

static void page_fault_handler(registers_t* regs);
static void dump_page_directory(void);
static void flush_tlb(void);

static void* kernel_heap_end;

void vmm__init() {
    debug("Initialize VMM");
    kernel_heap_end = (void*) KERNEL_HEAP_BASE;
    // Register a handler for PAGE_FAULT exception
    interrupt_handlers__register(PAGE_FAULT_EXCEPTION, page_fault_handler);    
}

static void page_fault_handler(registers_t* regs) {
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
   printf("Page fault ( ");
   if (present)
       printf("not-present ");
   if (writable)
       printf("read-only ");
   if (user)
       printf("kernel-mode-only ");
   if (reserved) 
       printf("reserved ");
   printf(") at 0x%x\n", addr);
   PANIC("Page fault");
}

static void dump_page_directory() {
    uint32_t* base = (uint32_t*) ADDR_PT_BASE;
    uint32_t* val = (uint32_t*) ADDR_PD_BASE;
    for (size_t i = 0; i < PD_ENTRIES_NUMBER; i++) {
        if (*(val + i) != 0) {
            debug("PDE %u : 0x%x", i, *(val+i));
            uint32_t* pte_base = (char*) base + (i << 12);
            for (size_t j = 0; j < PT_ENTRIES_NUMBER; j++) {
                uint32_t* pte = pte_base + j;
                if (*pte != 0) {
                    debug("\tPTE %u : 0x%x", j, *(pte));
                }
            }
        }
    }
}

static void flush_tlb() {
    __asm__ __volatile__("movl %cr3, %eax;"
            "movl %eax, %cr3;");
}

static void allocate_page_table(size_t pde_index, uint32_t flags) {
    uint32_t* pde_entry = (uint32_t*) (ADDR_PD_BASE + sizeof(uint32_t) * pde_index);
    if ((*pde_entry & 1) == 1) {
        PANIC("The page table already exists.");
    }
    // Allocate a physical frame for the page table
    uint32_t frame_addr = pmm__alloc_frame();
    *pde_entry = frame_addr | flags;
    flush_tlb();
}

static void allocate_page_table_entry(size_t pde_index, size_t pte_index, uint32_t flags) {
    uint32_t* pde_entry = (uint32_t*) (ADDR_PD_BASE + sizeof(uint32_t) * pde_index);
    // If the pde is not present, the PT must be allocated first
    if ((*pde_entry & 1) == 0) {
        allocate_page_table(pde_index, flags);
    }
    uint32_t* pte_entry = (uint32_t*) (ADDR_PT_BASE + (pde_index << 12) + sizeof(uint32_t) * pte_index);
    *pte_entry = pmm__alloc_frame() | flags;
    flush_tlb(); 
}

void* vmm__heap_extend(void* end) {
    debug("vmm_heap_extend called with 0x%x", end);
    while(kernel_heap_end <= end) {
        // Allocate a new frame
        uint32_t page = (uint32_t) (kernel_heap_end) / PAGE_SIZE;
        size_t pde_index = page / PT_ENTRIES_NUMBER;
        size_t pte_index = page % PT_ENTRIES_NUMBER;
        allocate_page_table_entry(pde_index, pte_index, PAGE_PRESENT | PAGE_WRITABLE);
        kernel_heap_end = (char*) kernel_heap_end + PAGE_SIZE;
    }
    return kernel_heap_end;
}
