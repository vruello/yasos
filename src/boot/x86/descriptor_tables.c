#include "boot/descriptor_tables.h"
#include "idt.h"
#include "gdt.h"

void descriptor_tables__init() {
    gdt__init();
    idt__init();
}
