#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "kernel/vmm.h"
#include "kernel/kmem.h"
#include "kernel/utils.h"

#define MAX_HEAP_SIZE 0x10000000 // 256 MiB
#define MIN_HEAP_BLOCK_PAYLOAD_SIZE 16 // 16o
#define KERNEL_HEAP_BASE 0xD0000000

typedef struct heap_block_struct {
    struct heap_block_struct* prev;
    struct heap_block_struct* next;
    uint32_t size : 31;
    uint32_t used   : 1;
} heap_block_t;

extern char bootstrap_heap_start;
extern char bootstrap_heap_end;

static void* bootstrap_alloc(uint32_t size, uint32_t flags);
static void* real_alloc(uint32_t size, uint32_t flags);
static void real_free(void* addr);
static void dump_heap_block(heap_block_t* block);

static uint32_t bootstrap_heap;
static uint32_t bootstrap_heap_available;

static bool heap_initialized;
static void* heap_end;
static void* heap_start;

void kmem__bootstrap_init() {    
    bootstrap_heap = (uint32_t) &bootstrap_heap_start;
    bootstrap_heap_available = (uint32_t) (&bootstrap_heap_end - &bootstrap_heap_start);
    debug("Initialize bootstrap heap (start: 0x%x, available : %u)", bootstrap_heap, bootstrap_heap_available);
    /* The bootstrap heap has been initialized, but not the "real" heap */
    heap_initialized = false;
}

void kmem__init() {
    heap_start = (void*) KERNEL_HEAP_BASE;
    // Allocate enough space for one heap block
    heap_end = vmm__heap_extend((char*) KERNEL_HEAP_BASE + sizeof(heap_block_t));
    debug("Initialize real heap (start: 0x%x, end: 0x%x, max: %u)", heap_start, heap_end, MAX_HEAP_SIZE);
    heap_block_t* first_block = (heap_block_t*) heap_start;
    first_block->prev = NULL;
    first_block->next = NULL;
    first_block->size = MAX_HEAP_SIZE - sizeof(heap_block_t);
    first_block->used = false;
    heap_initialized = true;
}

void* kmem__alloc(uint32_t size, uint32_t flags) {
    if (heap_initialized) {
        return real_alloc(size, flags);
    }
    else {
        return bootstrap_alloc(size, flags);
    }
}

void kmem__free(void* addr) {
    if (!heap_initialized) {
        PANIC("bootstrap heap blocks can not be freed");
    }
    real_free(addr);
}

static void* bootstrap_alloc(uint32_t size, uint32_t flags) {
    if (bootstrap_heap_available < size) {
        debug("Tried to allocate %u bytes but there are only %u bytes available.", size, bootstrap_heap_available);
        PANIC("Not enough space available in bootstrap_heap");
    }

    /** Always align on 8 bytes **/
    if ((bootstrap_heap & 0xfffffff7) != bootstrap_heap) {
        bootstrap_heap &= 0xfffffff7;
        bootstrap_heap += 0x8;
    }
    uint32_t tmp = bootstrap_heap;
    bootstrap_heap += size;    
    bootstrap_heap_available -= size;
    debug("kmem__alloc(%u) ; bootstrap_heap: 0x%x --> 0x%x", size, tmp, bootstrap_heap);
    return (void*) tmp;
}

void kmem__dump_heap() {
    heap_block_t* block = (heap_block_t*) heap_start;
    while(block) {
        dump_heap_block(block);
        block = block->next;
    }
}

static void dump_heap_block(heap_block_t* block) {
    debug("0x%x => prev: 0x%x ; next: 0x%x ; size: %d; used: %d", block, block->prev, block->next, block->size, block->used);
}

static void real_free(void* addr) {
    heap_block_t* block = (heap_block_t*) ((char*) addr - sizeof(heap_block_t));
    debug("Free block 0x%x", block);
    dump_heap_block(block);
    if (!block->used) {
        PANIC("Freeing a block already freed or never allocated");
    }
    
    block->used = false;

    // Previous block
    if (block->prev && !block->prev->used) {
        debug("Previous block is unused: 0x%x", block->prev);
        // Linkage
        block->prev->next = block->next;
        block->next->prev = block->prev;
        // Add size
        block->prev->size += sizeof(heap_block_t) + block->size;
        block = block->prev;
    }

    // Next block
    if (block->next && !block->next->used) {
        debug("Next block is unused: 0x%x", block->next);
        // Add size
        block->size += sizeof(heap_block_t) + block->next->size;
        // Linkage
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
}

static void* real_alloc(uint32_t size, uint32_t flags) {
    heap_block_t* block = (heap_block_t*) heap_start;
    while (block) {
        if (!block->used && size <= block->size) {
            block->used = true;
            if (size + sizeof(heap_block_t) + MIN_HEAP_BLOCK_PAYLOAD_SIZE < block->size) {
                // Enough available space to create a new heap block
                heap_block_t* new_block = (char*) block + sizeof(heap_block_t) + size;
                // Extend the heap if necessary
                if ((char*) new_block + sizeof(heap_block_t) >= (char*) heap_end) {
                    heap_end = vmm__heap_extend((char*) new_block + sizeof(heap_block_t));
                }
                // Link
                new_block->prev = block;
                new_block->next = block->next;
                block->next = new_block;
                // Mark the next block as unused
                new_block->used = false;
                new_block->size = block->size - size - sizeof(heap_block_t);
                block->size = size;
            }
            else {
                // Not enough space to create a new block so allocate the whole block
                block->used = true;
                // Extend the heap if necessary
                if ((char*) block + block->size >= (char*) heap_end) {
                    heap_end = vmm__heap_extend((char*) block + block->size);
                }
            }
            return (char*) block + sizeof(heap_block_t);
        }
        block = block->next;
    }
    PANIC("Heap exhausted");
    return NULL;
}
