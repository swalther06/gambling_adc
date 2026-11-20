#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

// All clips: 8 kHz, mono, 12-bit unsigned samples (0-4095, silence = 2048),
// right-aligned in uint32_t for direct writes to a 12-bit DAC data register.
#define AUDIO_SAMPLE_RATE_HZ 8000u

extern const uint32_t fah[];
extern const uint32_t fah_len;   // number of samples

extern const uint32_t spongebob_fail[];
extern const uint32_t spongebob_fail_len;   // number of samples

#endif // AUDIO_H
