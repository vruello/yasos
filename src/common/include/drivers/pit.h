#ifndef PIT_H
#define PIT_H

#include <stdint.h>

typedef enum {
    PIT_CH0 = 0,
    PIT_CH1 = 1,
    PIT_CH2 = 2
} pit__channel_t;

void pit__init(uint32_t frequency);
void pit__set_frequency(pit__channel_t channel, uint32_t frequency);

#endif
