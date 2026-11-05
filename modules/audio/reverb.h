// SPDX-License-Identifier: MPL-2.0
#ifndef SOTN_AUDIO_REVERB_H
#define SOTN_AUDIO_REVERB_H
#include "voice.h"
#include <sotn/game_data.h>

// Actual SPU state. The processor borrows the existing512KB sound RAM.
typedef struct SotnReverb
{
    Uint16 parameters[32];
    Uint32 base, cursor;
    Sint16 depth[2], input[2], previous[2];
    bool configured, enabled, phase;
} SotnReverb;

/** Configure bounded SDK coefficients without touching RAM or advancing audio. */
bool SoundReverb_Configure(SotnReverb *reverb, Uint32 base, SotnDataView coefficients);
/** Apply original signed SsUtSetReverbDepth conversion. Requires configured ownership. */
bool SoundReverb_SetDepth(SotnReverb *reverb, Sint16 left, Sint16 right);
/** Process one actual44.1kHz sample, retaining the original half-rate phase and work cursor. */
void SoundReverb_Tick(SotnReverb *reverb, Uint8 ram[SOTN_SPU_RAM_SIZE], Sint32 left, Sint32 right,
                      Sint32 output[2]);
#endif
