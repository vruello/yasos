#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>

#include "kernel/registers.h"

typedef struct {
    uint32_t present    : 1; // Page present in memory
    uint32_t writable   : 1; // Writable page
    uint32_t user       : 1; // supervisor level only if clear
    uint32_t reserved   : 2; // reserved by CPU
    uint32_t accessed   : 1; // page accessed
    uint32_t dirty      : 1; // page has been written to
    uint32_t zero       : 1; // zero / PAT
    uint32_t global     : 1; // global flag
    uint32_t available  : 3; // available
    uint32_t frame      : 20;
} page_t;

typedef struct {
    page_t pages[1024];
} page_table_t;

typedef struct {
    /* Array of page tables */
    page_table_t *tables[1024];

    /* Array containing the physical address of page tables
     * Used for loading into the CR3 register. */ 
    uint32_t tables_physical[1024]; 

    /* Physical address of tables_physical
     * This comes into play when we get our kernel heap allocated and the directory
     * may be in a different location in virtual memory */
    uint32_t physical_addr; 
} page_directory_t;

/* 
 * Sets up the environment, page directories etc and enables paging
 */
void paging__init(void);

/**
 * Causes the specificed page directory to be loaded into the
 * CR3 register
**/
void paging__switch_page_directory(page_directory_t *new);

/**
 * Retrieves a pointer to the page required.
 * If make is true, if the page table in which this page should reside
 * isn't created, create it!
**/
page_t* paging__get_page(uint32_t address, bool make, page_directory_t *dir);

/**
 * Handler for page faults
**/
void paging__fault_handler(registers_t* reg);

#endif
