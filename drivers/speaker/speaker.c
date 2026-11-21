
#include "speaker.h"
#include "audio.h"
#include "cybsp.h"
#include "cy_utils.h"
#include <stdint.h>
#include <stdbool.h>

static uint32_t audio[SPEAKER_MAX_SAMPLES];
void build_instruction_cards(speaker_t *speaker){
  static XMC_DMA_LLI_t blocks[SPEAKER_MAX_BLOCKS];
  speaker_config_t *speak_config = &speaker->config;
  //build 16 instruction cards
  for (unsigned i = 0; i < SPEAKER_MAX_BLOCKS; ++i){
  	// First sample for each block
  	blocks[i].src_addr = (uint32_t)&audio[i * DMA_MAX_BLOCK_SIZE];
  	// Create a linked list of all the blocks
  	blocks[i].llp = (i < SPEAKER_MAX_BLOCKS - 1) ? &blocks[i + 1] : NULL;
  	// Transfer settings from Device Configurator
  	blocks[i].control = speak_config->dma_config->control;
   	blocks[i].block_size = DMA_MAX_BLOCK_SIZE;
     	// The last block should not link to anything
     	blocks[i].enable_src_linked_list = (i < SPEAKER_MAX_BLOCKS - 1);
  }
  // Point DMA at the first instruction card.
  XMC_DMA_CH_SetLinkedListPointer(speak_config->dma_number, speak_config->dma_channel, blocks);
}


bool speaker_init(speaker_t *speaker, speaker_config_t *cfg) {
    // make sure the corresponding DAC is in unsigned mode
    if (speaker == NULL || cfg == NULL || cfg->dma_config == NULL || cfg->dac_number == NULL) {
        return false;
    }

    // DMA channel must be GPDMA0 channel 0 or 1 (the only multi-block/linked-list channels)
    if (cfg->dma_number != XMC_DMA0 || cfg->dma_channel > 1U) {
        return false;
    }

    // DAC must be a real channel, and powered on so its config registers can be read
    if (cfg->dac_number != XMC_DAC0 || cfg->dac_channel >= XMC_DAC_NO_CHANNELS || !XMC_DAC_IsEnabled(cfg->dac_number)) {
        return false;
    }

    // DAC must be in unsigned mode (samples are unsigned 12-bit, silence = 2048)
    if ((cfg->dac_number->DACCFG[cfg->dac_channel].low & DAC_DAC0CFG0_SIGN_Msk) != 0U) {
        return false;
    }

    // DAC must not scale the samples (they're already full-range 12-bit)
    if ((cfg->dac_number->DACCFG[cfg->dac_channel].high & (DAC_DAC0CFG1_MULDIV_Msk | DAC_DAC0CFG1_SCALE_Msk))
            != XMC_DAC_CH_OUTPUT_SCALE_NONE) {
        return false;
    }

    // DMA must write into this DAC channel's data register
    if (cfg->dma_config->dst_addr != (uint32_t)&cfg->dac_number->DACDATA[cfg->dac_channel]) {
        return false;
    }

    // DMA transfer widths must be 32-bit (audio[] is uint32_t, DAC data register is 32-bit)
    if (cfg->dma_config->src_transfer_width != XMC_DMA_CH_TRANSFER_WIDTH_32 ||
        cfg->dma_config->dst_transfer_width != XMC_DMA_CH_TRANSFER_WIDTH_32) {
        return false;
    }

    // make sure no clip is playing (skip if GPDMA0 is off -- channel can't be running then)
    if (XMC_DMA_IsEnabled(cfg->dma_number)) {
        XMC_DMA_CH_Disable(cfg->dma_number, cfg->dma_channel);
        while (XMC_DMA_CH_IsEnabled(cfg->dma_number, cfg->dma_channel)) {}
    }

    // assign all config stuff into the speaker member.
    speaker->config = *cfg;
    return true;
}

bool play_sound(speaker_t *speaker, sound_name_t sfx_name) {    
    // if the current clip hasn't finished playing
    speaker_config_t *speak_config = &speaker->config;
    if (XMC_DMA_CH_IsEnabled(speak_config->dma_number, speak_config->dma_channel)) {
        // stop the current clip before starting a new clip (avoids undef behavior)
        XMC_DMA_CH_Disable(speak_config->dma_number, speak_config->dma_channel);
        while (XMC_DMA_CH_IsEnabled(speak_config->dma_number, speak_config->dma_channel)) {}
    }
    // build the instruction cards
    // must occur afterward because this moves linked list pointer back to front of
    // the audio array, so that the new sound is played from the beginning
    build_instruction_cards(speaker);
    // fill audio array with silence:
    for(int i = 0; i < SPEAKER_MAX_SAMPLES; ++i) {
        audio[i] = 2048;
    }

    // for each case we gotta check that the sound length is less than
    // the audio length
    switch (sfx_name) {
        case SOUND_NAME_FAH:
            // copy the array into audio
            for (uint32_t i = 0; i < fah_len; ++i) {
                audio[i] = fah[i];
            }
            break;
        case SOUND_NAME_SPONGEBOB_FAIL:
            // copy the array into audio
            for (uint32_t i = 0; i < spongebob_fail_len; ++i) {
                audio[i] = spongebob_fail[i];
            }
            break;
        default:
            return false;
    }
    XMC_DMA_CH_Enable(speak_config->dma_number, speak_config->dma_channel);
    return true;
}