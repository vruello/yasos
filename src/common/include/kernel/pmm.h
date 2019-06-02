#ifndef PMM_H
#define PMM_H

#include <stddef.h>
#include <stdint.h>

#include "boot/multiboot.h"

void pmm__init(multiboot_memory_map_t* mmap, multiboot_uint32_t length);
uint32_t pmm__alloc_frame(void);
void pmm__free_frame(size_t);

#endif
