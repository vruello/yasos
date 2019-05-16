#ifndef GDT_H
#define GDT_H

#define KERN_CODE_SEG 0x08
#define KERN_DATA_SEG 0x10
#define USER_CODE_SEG 0x18
#define USER_DATA_SEG 0x20

void gdt__init(void);

#endif
