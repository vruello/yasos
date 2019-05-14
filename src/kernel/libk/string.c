#include <stddef.h>
#include <stdint.h>

#include "kernel/libk/string.h"

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void* memmove(void* destination, const void* source, size_t num) {
    uint8_t* dst = (uint8_t*) destination;
    uint8_t* src = (uint8_t*) source;
    
    if (dst < src) {
        for (size_t i = 0; i < num; i++) {
            dst[i] = src[i];
        }
    }
    else if (dst > src) {
        for (size_t i = num; i > 0; i--) {
            dst[i-1] = src[i-1];
        }
    }

    return destination;
}
