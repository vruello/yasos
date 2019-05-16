#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

struct registers {
    uint32_t ds; // saved ds
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // registers pushed by pusha
    uint32_t int_no, err_code; // Interrupt number and error code (0 if none)
    uint32_t eip, cs, eflags, useresp, ss; // push automatically by the processor
};

typedef struct registers registers_t;

#endif
