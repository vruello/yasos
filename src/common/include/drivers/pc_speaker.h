#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H

#include <stdint.h>

void pc_speaker__play(uint32_t frequency);
void pc_speaker__stop(void);

#endif
