#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "libk/bitset.h"

static inline size_t bitset__get_index(uint32_t entry);
static inline size_t bitset__get_bit(uint32_t entry);

static inline size_t bitset__get_index(uint32_t entry) {
    return entry / 32; // each uint32_t value contains 32 values
}

static inline size_t bitset__get_bit(uint32_t entry) {
    return entry % 32;
}

void bitset__set(uint32_t entry, uint32_t* bitset) {
    bitset[bitset__get_index(entry)] |= (1 << bitset__get_bit(entry));
}

void bitset__clear(uint32_t entry, uint32_t* bitset) {
    bitset[bitset__get_index(entry)] &= (0 << bitset__get_bit(entry));
}

bool bitset__test(uint32_t entry, uint32_t* bitset) {
    return (bool) (bitset[bitset__get_index(entry)] & (1 << bitset__get_bit(entry)));
}

bool bitset__find_first_clear(uint32_t* bitset, size_t size, size_t* index, size_t* bit) {
    for (size_t i = 0; i < size; i++) {
        if (bitset[i] == 0xffffffff) {
            // Nothing free, continue
            continue;
        }

        for (size_t j = 0; j < 32; j++) {
            if (!(bitset[i] & (1 << j))) {
                // bit is clear
                *index = i;
                *bit = j;
                return true;
            }
        }
    }

    return false;
}
