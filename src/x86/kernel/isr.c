#include <stdint.h>
#include "drivers/vga.h"
#include "kernel/registers.h"

void isr__handler(registers_t regs);

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
