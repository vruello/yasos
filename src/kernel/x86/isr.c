#include <stdint.h>
#include "drivers/vga.h"

struct registers {
    uint32_t ds; // saved ds
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // registers pushed by pusha
    uint32_t int_no, err_code; // Interrupt number and error code (0 if none)
    uint32_t eip, cs, eflags, useresp, ss; // push automatically by the processor
};

typedef struct registers registers_t;

void isr__handler(registers_t regs) {
   vga__writestring("received interrupt: ");
   vga__writedec(regs.int_no);
   vga__putchar('\n');

   if (regs.err_code != 0) {
       vga__writestring("with error code: ");
       vga__writedec(regs.err_code);
       vga__putchar('\n');
   }
}
