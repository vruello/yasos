#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "libk/string.h"
#include "libk/stdio.h"

static size_t uint_to_str(unsigned int value, char* str, int base);
static size_t int_to_str(int value, char* str, int base);
static bool snprintf_putc_until_limit(char* dst, char c, size_t* n, size_t max);
static bool snprintf_puts_until_limit(char* dst, char* s, size_t* n, size_t max);

static uint8_t tmpbuf[64];
static putchar_t display_putchar;
static puts_t display_puts;
static char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static size_t int_to_str(int value, char* str, int base) {
    size_t res = 0;
    if (value < 0) {
        *(str++) = '-';
        value *= -1;
        res++;
    }
    return res + uint_to_str((unsigned int) value, str, base);
}

static size_t uint_to_str(unsigned int value, char* str, int base) {
    if (base < 2 || base > 16) {
        return 0;
    }
    
    if (value == 0) {
        str[0] = bchars[0];
        str[1] = 0;
        return 1;
    }
    
    size_t i = 0;
    while (value > 0) {
        tmpbuf[i] = (uint8_t) (value % base); 
        value /= base;
        i++;
    }
    size_t res = i;
    for (; i > 0; i--) {
        *(str++) = bchars[tmpbuf[i-1]];
    }

    *(str++) = 0;
    return res;
}

void putchar(char c) {
    display_putchar(c);
}

void puts(char *str) {
    display_puts(str);
}

void stdio__init(putchar_t _putchar, puts_t _puts) {
    display_puts = _puts;
    display_putchar = _putchar;
}


/**
 * @return true if n is max, false otherwise
 */
static bool snprintf_putc_until_limit(char* dst, char c, size_t* n, size_t max) {
    if (*n == max) {
        return true;
    }
    dst[*n] = c;
    (*n)++;
    return false;
}

static bool snprintf_puts_until_limit(char* dst, char* s, size_t* n, size_t max) {
    size_t i = 0;
    bool stop = false;;
    while (s[i] != 0 && stop == false) {
        stop = snprintf_putc_until_limit(dst, s[i], n, max);
        i++;
    }
    return stop;
}

void snprintf(char *dst, size_t max, const char *str, ...) {
    va_list ap;
    va_start(ap, str);
    vsnprintf(dst, max, str, ap);
    va_end(ap);
}

void vsnprintf(char *dst, size_t max, const char *str, va_list ap) {
    // Always append a zero at the end
    max--;
    size_t n = 0;
    bool stop = false;
    for(size_t i = 0; i < strlen(str); i++) {
        if (stop) {
            // max chars have been written
            break;
        }
        if (str[i] == '%') {
            switch(str[i+1]) {
                case 's':
                    {
                        char *s = va_arg(ap, char*);
                        stop = snprintf_puts_until_limit(dst, s, &n, max);
                        i++;
                        continue;
                    }
                case 'd':
                    {
                        int value = va_arg(ap, int);
                        char buf[11] = {0};
                        int_to_str(value, buf, 10);
                        stop = snprintf_puts_until_limit(dst, buf, &n, max);
                        i++;
                        continue;
                    }
                case 'u':
                    {
                        unsigned int value = va_arg(ap, unsigned int);
                        char buf[10] = {0};
                        uint_to_str(value, buf, 10);
                        stop = snprintf_puts_until_limit(dst, buf, &n, max);
                        i++;
                        continue;
                    }
                case 'x':
                    {
                        unsigned int value = va_arg(ap, unsigned int);
                        char buf[9] = {0};
                        uint_to_str(value, buf, 16); 
                        stop = snprintf_puts_until_limit(dst, buf, &n, max);
                        i++;
                        continue;
                    }
                case 'c':
                    {
                        char value = (char) va_arg(ap, int);
                        stop = snprintf_putc_until_limit(dst, value, &n, max);
                        i++;
                        continue;
                    }
                case 'b':
                    {
                        unsigned int value = va_arg(ap, unsigned int);
                        char buf[33] = {0};
                        uint_to_str(value, buf, 2); 
                        stop = snprintf_puts_until_limit(dst, buf, &n, max);

                        i++;
                        continue;
                    }
                case '%':
                    stop = snprintf_putc_until_limit(dst, '%', &n, max);
                    i++;
                    continue;
                default:
                    break;
            }
        }
        else {
            stop = snprintf_putc_until_limit(dst, str[i], &n, max);
        }
    }
    dst[n++] = 0;
    va_end(ap);
}

void vprintf(const char* str, va_list ap) {
    char buf[256];
    vsnprintf(buf, 256, str, ap);
    puts(buf);
}

void printf(const char* str, ...) {
    va_list ap;
    va_start(ap, str);
    vprintf(str, ap);
    va_end(ap);
}

