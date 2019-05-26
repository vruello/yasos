#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void bitset__set(uint32_t entry, uint32_t* bitset);
void bitset__clear(uint32_t entry, uint32_t* bitset);
bool bitset__test(uint32_t entry, uint32_t* bitset);
bool bitset__find_first_clear(uint32_t* bitset, size_t size, size_t* index, size_t* bit);

#endif
