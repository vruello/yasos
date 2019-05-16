#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* str);
void* memmove(void* destination, const void* source, size_t num);
void* memset(void* ptr, int value, size_t num);

#endif
