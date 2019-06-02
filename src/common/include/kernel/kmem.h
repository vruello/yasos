#ifndef KMEM_H
#define KMEM_H

#include <stdint.h>

void kmem__init(void);
void kmem__bootstrap_init(void);
void* kmem__alloc(uint32_t size, uint32_t flags);
void kmem__free(void* addr);
void kmem__dump_heap(void);

#endif
