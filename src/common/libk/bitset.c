#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "libk/string.h"
#include "libk/bitset.h"
#include "drivers/vga.h"
#include "kernel/kmem.h"

#define ENTRY_SIZE 32

struct bitset__struct {
    uint32_t* set;
    size_t size;
};

static inline size_t get_index(size_t);
static inline size_t get_offset(size_t);

static inline size_t get_index(size_t index) {
    return index / ENTRY_SIZE; // each uint32_t value contains ENTRY_SIZE values
}

static inline size_t get_offset(size_t index) {
    return index % ENTRY_SIZE;
}

int bitset__set(size_t index, bitset_t* bitset) {
    if (index >= bitset->size) {
        return -1;
    }

    bitset->set[get_index(index)] |= (1 << get_offset(index));
    return 0;
}

int bitset__clear(size_t index, bitset_t* bitset) {
    if (index >= bitset->size) {
        return -1;
    }

    bitset->set[get_index(index)] &= (0 << get_offset(index));
    return 0;
}

int bitset__test(size_t index, bitset_t* bitset) {
    if (index >= bitset->size) {
        return -1;
    }

    return (bitset->set[get_index(index)] & (1 << get_offset(index))) ? 1 : 0;
}

int bitset__set_first_clear(bitset_t* bitset) {
    size_t length = bitset->size / ENTRY_SIZE;
    if (bitset->size % ENTRY_SIZE == 0) {
        length++;
    }
    for (size_t i = 0; i < length; i++) {
        if (bitset->set[i] == 0xffffffff) {
            // Nothing free, continue
            continue;
        }

        for (size_t j = 0; j < ENTRY_SIZE; j++) {
            if (!(bitset->set[i] & (1 << j))) {
                // offset is clear
                // Seat the corresponding dex
                bitset__set(i * ENTRY_SIZE + j, bitset);
                return (int) (i * ENTRY_SIZE + j); 
            }
        }
    }

    return -1;
}

bitset_t* bitset__new(size_t size) {
    bitset_t* bitset = kmem__alloc(sizeof(bitset_t), 0);
    bitset->size = size / ENTRY_SIZE + 1;
    bitset->set = kmem__alloc(sizeof(uint32_t) * bitset->size, 0);
    memset(bitset->set, 0, sizeof(uint32_t) * bitset->size);
    return bitset;
}

size_t bitset__size(bitset_t* bitset) {
    return bitset->size;
}
