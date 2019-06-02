#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct {
    uint32_t* set;
    size_t size;
} bitset_t;

/**
 * Set the bit at the given index in the given bitset
 * @return 0 if success, -1 if index is invalid.
 */
int bitset__set(size_t index, bitset_t* bitset);

/**
 * Clear the bit at the given index in the given bitset
 * @return 0 if success, -1 if index is invalid.
 */
int bitset__clear(size_t index, bitset_t* bitset);

/**
 * Test the bit at the given index in the given bitset
 * @return
 *  1 if the bit is set
 *  0 if the bit is clear
 *  -1 if the index is invalid
 */
int bitset__test(size_t index, bitset_t* bitset);

/**
 * Set the first clear bit in the given bitset
 * @return
 *  the index of the bit set
 *  -1 if there is no clear bit
 */
int bitset__set_first_clear(bitset_t* bitset);

/**
 * @return a new allocated bitset of given size
 */
bitset_t* bitset__new(size_t size);

/**
 * @return the size of the given bitset
 */
size_t bitset__size(bitset_t* bitset);

#endif
