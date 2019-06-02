#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

typedef void (*putchar_t)(char);
typedef void (*puts_t)(const char*);

void stdio__init(putchar_t, puts_t);
void printf(const char* str, ...);
void snprintf(char *dst, size_t max, const char *str, ...);
void vsnprintf(char *dst, size_t max, const char *str, va_list ap);
void vprintf(const char* str, va_list ap);
void putchar(char c);
void puts(char *str);

#endif
