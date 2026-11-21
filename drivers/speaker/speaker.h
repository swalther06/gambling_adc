#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>
#include <stdbool.h>
#include "xmc_dma.h"
#include "xmc_dac.h"
#include "hw_config.h"

#define SPEAKER_MAX_BLOCKS 16U
#define SPEAKER_MAX_SAMPLES (SPEAKER_MAX_BLOCKS * DMA_MAX_BLOCK_SIZE)

typedef enum {
    SOUND_NAME_FAH,
    SOUND_NAME_SPONGEBOB_FAIL,
} sound_name_t;

typedef struct {
    const XMC_DMA_CH_CONFIG_t *dma_config;
    XMC_DMA_t *dma_number;
    uint8_t dma_channel;
    XMC_DAC_t *dac_number;
    uint8_t dac_channel;
} speaker_config_t;

typedef struct {
    speaker_config_t config;
} speaker_t;

// requires: the DAC module is already enabled and its channel configured
// modifies: disables the speaker's DMA channel (only if init is valid)
// returns: true if ... initiation was valid
// false if ...
//   speaker, cfg, cfg->dma_config, or cfg->dac is NULL
//   the DAC is not XMC_DAC0 channel 0 or 1, or the DAC module is not enabled
//   the DAC is signed mode
//   the destination address for the DMA is NOT the configured DAC
//   the DMA channel is NOT GPDMA0 Channel 0 or 1
//   the DAC scale setting is not "no scaling" (samples are already 12-bit)
//   the DMA transfer widths are not 32-bit
bool speaker_init(speaker_t *speaker, speaker_config_t *cfg);

// input: name of desired sound effect, only valid choices are defined
// above in sound_name_t enumerated type
// modifies: dac data register --> outputs the chosen sound to speaker
// returns: true if inputted sound effect is valid
// false if the selected sound effect is too large to store
bool play_sound(speaker_t *speaker, sound_name_t sfx_name);

#endif