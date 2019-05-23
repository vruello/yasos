#include <stdint.h>

#include "drivers/pc_speaker.h"
#include "drivers/io.h"
#include "drivers/pit.h"

#define PC_SPEAKER_PORT 0x61

void pc_speaker__play(uint32_t frequency) {
    pit__set_frequency(PIT_CH2, frequency);
    uint8_t tmp = inb(PC_SPEAKER_PORT);
    if (tmp != (tmp | 3)) {
        outb(PC_SPEAKER_PORT, tmp | 3);
    }
}

void pc_speaker__stop() {
    uint8_t tmp = inb(PC_SPEAKER_PORT) & 0xfc;
 
 	outb(PC_SPEAKER_PORT, tmp);
}

