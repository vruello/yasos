#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>

uint32_t kmem__alloc(uint32_t size);
uint32_t kmem__alloc_a(uint32_t size, bool align);
uint32_t kmem__alloc_p(uint32_t size, uint32_t* phys);
uint32_t kmem__alloc_ap(uint32_t size, bool align, uint32_t* phys);

#endif
