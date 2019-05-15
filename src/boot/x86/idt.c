#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "gdt.h"
#include "kernel/common/libk/string.h"

struct idt_entry {
    uint16_t base_low; // the lower 16 bits to the address to jump to
    uint16_t sel; // kernel segment selector
    uint8_t zero; // this must always be zero
    uint8_t flags; // More flags
    uint16_t base_high; // the higher 16 bits to the address to jump to 
} __attribute__((packed)); // tell the compiler to not rearrange the fields

typedef struct idt_entry idt_entry_t;

struct idt_ptr {
    uint16_t limit; // the size of the idt table in bytes
    uint32_t base; // the address of the first idt_entry_t 
} __attribute__((packed));

typedef struct idt_ptr idt_ptr_t;

static void idt__set_entry(size_t index, uint32_t base, uint16_t sel,
        uint8_t flags);
extern void idt__flush(uint32_t);

#define IDT_SIZE 256
#define IDT_FLAGS 0x8E

static idt_entry_t idt_entries[IDT_SIZE];
static idt_ptr_t idt;

/*
 * Load 32 CPU-dedicated interrupts handlers from asm
 */

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/**
 * Initialize the IDT table
 */
void idt__init() {
    idt.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idt.base = (uint32_t) idt_entries;
   
    // Initialize idt_entries memory
    memset(idt_entries, 0, sizeof(idt_entry_t) * IDT_SIZE);
    
    // Set 32 CPU-decicated interrupt handlers
    idt__set_entry(0, (uint32_t) isr0, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(1, (uint32_t) isr1, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(2, (uint32_t) isr2, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(3, (uint32_t) isr3, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(4, (uint32_t) isr4, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(5, (uint32_t) isr5, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(6, (uint32_t) isr6, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(7, (uint32_t) isr7, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(8, (uint32_t) isr8, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(9, (uint32_t) isr9, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(10, (uint32_t) isr10, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(11, (uint32_t) isr11, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(12, (uint32_t) isr12, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(13, (uint32_t) isr13, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(14, (uint32_t) isr14, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(15, (uint32_t) isr15, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(16, (uint32_t) isr16, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(17, (uint32_t) isr17, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(18, (uint32_t) isr18, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(19, (uint32_t) isr19, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(20, (uint32_t) isr20, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(21, (uint32_t) isr21, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(22, (uint32_t) isr22, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(23, (uint32_t) isr23, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(24, (uint32_t) isr24, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(25, (uint32_t) isr25, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(26, (uint32_t) isr26, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(27, (uint32_t) isr27, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(28, (uint32_t) isr28, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(29, (uint32_t) isr29, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(30, (uint32_t) isr30, KERN_CODE_SEG, IDT_FLAGS);
    idt__set_entry(31, (uint32_t) isr31, KERN_CODE_SEG, IDT_FLAGS);
    
    idt__flush((uint32_t)&idt);
}

/**
 * Set the idt entry at the given index
 */
static void idt__set_entry(size_t index, uint32_t base, uint16_t sel,
        uint8_t flags) {
    idt_entries[index].base_low = (uint16_t) base & 0xffff;
    idt_entries[index].base_high = (uint16_t) (base >> 16) & 0xffff;
    
    idt_entries[index].sel = sel;
    idt_entries[index].flags = flags;
}
