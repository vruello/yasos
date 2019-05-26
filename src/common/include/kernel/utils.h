#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define PANIC(str)  panic(str, __FILE__, __LINE__);

void panic(const char* str, const char* filename, size_t line);

#endif
