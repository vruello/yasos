#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "kernel/kmem.h"

#define BASE_ADDRESS 0x200000 // 2 MiB

// Should be global to be read by kernel/paging.c
uint32_t placement_address = BASE_ADDRESS;

uint32_t kmem__alloc(uint32_t size) {
    /** Always align on 8 bytes **/
    if ((placement_address & 0xfffffff7) != placement_address) {
        placement_address &= 0xfffffff7;
        placement_address += 0x8;
    }
    uint32_t tmp = placement_address;
    placement_address += size;    
    return tmp;
}

uint32_t kmem__alloc_a(uint32_t size, bool align) {
    if (align && (placement_address & 0xfffff000) != placement_address) {
        // We must align the placement_address
        placement_address &= 0xfffff000;
        placement_address += 0x1000; 
    }
    return kmem__alloc(size);
}

uint32_t kmem__alloc_p(uint32_t size, uint32_t* phys) {
    uint32_t res = kmem__alloc(size);
    if (phys != NULL) {
        *phys = res;
    }
    return res;
}

uint32_t kmem__alloc_ap(uint32_t size, bool align, uint32_t* phys){
    uint32_t res = kmem__alloc_a(size, align);
    if (phys != NULL) {
        *phys = res;
    }
    return res;
}
