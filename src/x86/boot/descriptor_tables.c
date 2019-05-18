#include "boot/descriptor_tables.h"
#include "boot/idt.h"
#include "boot/gdt.h"

void descriptor_tables__init() {
    gdt__init();
    idt__init();
}
