#include <stdint.h>
#include <stddef.h>
#include "boot/gdt.h"

struct gdt_entry {
    uint16_t limit_low; // The lower 16 bits of the limit 
    uint16_t base_low; // The lover 16 bits of the base
    uint8_t base_middle; // the next 8 bits of the base
    uint8_t access; // access flags
    uint8_t granularity; // granularity flags + next 4 bits of limits
    uint8_t base_high; // the last 8 bits of the base
} __attribute__((packed)); // tell the compiler to not rearrange the fields

typedef struct gdt_entry gdt_entry_t;

struct gdt_ptr {
    uint16_t limit; // the size of the gdt table in bytes
    uint32_t base; // the address of the first gdt_entry_t 
} __attribute__((packed));

typedef struct gdt_ptr gdt_ptr_t;

static void gdt__set_entry(size_t index, uint32_t base, uint32_t limit,
        uint8_t access, uint8_t gran);
extern void gdt__flush(uint32_t);

// Set 5 gdt entries : Null, Kernel:code, Kernel:data, User:code User:data
#define GDT_SIZE 5

static gdt_entry_t gdt_entries[GDT_SIZE];
static gdt_ptr_t gdt;

/**
 * Initialize the GDT table
 */
void gdt__init() {
    gdt.limit = (sizeof(gdt_entry_t) * GDT_SIZE) - 1;
    gdt.base = (uint32_t) gdt_entries;

    gdt__set_entry(0, 0, 0, 0, 0); // null segment
    gdt__set_entry(1, 0, 0xfffff, 0x9a, 0xc); // kernel code segment
    gdt__set_entry(2, 0, 0xfffff, 0x92, 0xc); // kernel data segment
    gdt__set_entry(3, 0, 0xfffff, 0xfa, 0xc); // user code segment
    gdt__set_entry(4, 0, 0xfffff, 0xf2, 0xc); // user data segment
    
    gdt__flush((uint32_t)&gdt);
}

/**
 * Set the gdt entry at the given index
 */
static void gdt__set_entry(size_t index, uint32_t base, uint32_t limit,
        uint8_t access, uint8_t gran) {
    gdt_entries[index].base_low = base & 0xffff;
    gdt_entries[index].base_middle = (base >> 16) & 0xff;
    gdt_entries[index].base_high = (uint8_t) (base >> 24) & 0xff;

    gdt_entries[index].limit_low = limit & 0xffff;
    gdt_entries[index].granularity = (limit >> 16) & 0x0f;

    gdt_entries[index].granularity |= (uint8_t) (gran << 4);
    gdt_entries[index].access = access;
}
