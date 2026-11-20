#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SOUND_NAME_FAH,
    SOUND_NAME_SPONGEBOB_FAIL,
} sound_name_t;

typedef struct {
    // DMA number & Channel
    // Associated timer
} speaker_config_t;

typedef struct {
    speaker_config_t config;
} speaker_t;

bool speaker_init(speaker_t *speaker, speaker_config_t *cfg);

bool play_sound(sound_name_t sfx_name);

#endif