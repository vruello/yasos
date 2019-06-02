#include <stdint.h>
#include <stddef.h>

#include "kernel/pmm.h"
#include "boot/multiboot.h"
#include "libk/bitset.h"
#include "kernel/utils.h"

#define FRAME_SIZE 4096 // 0x1000

static size_t compute_frames_number(multiboot_memory_map_t* mmap, 
        multiboot_uint32_t length);
static void clear_available_frames(multiboot_memory_map_t* mmap, 
        multiboot_uint32_t length);

static bitset_t* frames_bitset;

static size_t compute_frames_number(multiboot_memory_map_t* mmap, 
        multiboot_uint32_t length) {
    uint32_t size = 0;
    uint64_t res = 0;
    while (size < length) { 
        debug("[MMAP] addr_high: 0x%x ; addr_low: 0x%x ; len: %u ; type: %d", 
                mmap->addr_high, mmap->addr_low, mmap->len_low, mmap->type);
        uint64_t addr = (uint64_t) mmap->addr_high;
        addr = (addr << 32) + mmap->addr_low + mmap->len_low;
        if (addr > res) {
            res = addr;
        }
        size += mmap->size + sizeof(multiboot_uint32_t);
        mmap = (multiboot_memory_map_t*) ((char*) mmap + mmap->size + sizeof(multiboot_uint32_t));
    }
    if (res % FRAME_SIZE == 0) {
        return (size_t) (res / FRAME_SIZE);
    }
    else {
        return (size_t) (res / FRAME_SIZE + 1);
    }
}

static void clear_available_frames(multiboot_memory_map_t* mmap, 
        multiboot_uint32_t length) {
    uint32_t size = 0;
    while (size < length) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t addr = (uint64_t) mmap->addr_high;
            addr = (addr << 32) + mmap->addr_low;
            size_t index_start = (size_t) (addr / FRAME_SIZE);
            if (mmap->addr_low % FRAME_SIZE != 0) {
                index_start++;
            }
            size_t index_end = index_start + mmap->len_low / FRAME_SIZE;
            index_start = (index_start < 1024) ? 1024 : index_start; 
            for (size_t i = index_start; i < index_end; i++) {
                bitset__clear(i, frames_bitset);            
            }
        }
        size += mmap->size + sizeof(multiboot_uint32_t);
        mmap = (multiboot_memory_map_t*) ((char*) mmap + mmap->size + sizeof(multiboot_uint32_t));
    }
}

/**
 * Init Physical Memory Manager 
 * Create a frames bitset. Mark every frames as used, except those
 * corresponding to available memory 
 */
void pmm__init(multiboot_memory_map_t* mmap, multiboot_uint32_t length) {
    size_t frames_number = compute_frames_number(mmap, length);
    debug("Initialize PMM with %u frames", frames_number);
    frames_bitset = bitset__new(frames_number);
    
    // Mark all frames unavailable by default
    for (size_t i = 0; i < bitset__size(frames_bitset); i++) {
        bitset__set(i, frames_bitset);
    }

    // clear only frames that correspond to available memory
    // except the 1024 first frames which are already used
    clear_available_frames(mmap, length);
}

/*
 * Function to allocate a frame
 * @returns the physical address of the allocated frame 
 */
uint32_t pmm__alloc_frame() {
    int frame_index = bitset__set_first_clear(frames_bitset);
    if (frame_index == -1) {
        PANIC("No available frames");
    }
    return (size_t) frame_index * FRAME_SIZE;
}

void pmm__free_frame(size_t frame) {
    bitset__clear(frame, frames_bitset);
}
