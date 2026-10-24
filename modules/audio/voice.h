// SPDX-License-Identifier: MPL-2.0
// Selected voice decoding, Gaussian resampling, and envelope paths adapted from
// PSY-Z b52003e55c0d1b20719e984dd19bd4d760eeb14a,
// psyz/src/psyz/spu_voice.h. SDK headers and platform services are not imported.
#ifndef SOTN_AUDIO_VOICE_H
#define SOTN_AUDIO_VOICE_H
#include "gauss.h"
#include <SDL3/SDL.h>
#define SOTN_SPU_RAM_SIZE 524288U
typedef struct SotnVoiceRegisters
{
    Uint16 start_addr, loop_addr, pitch, adsr[2], volumex;
    Sint32 left, right;
} SotnVoiceRegisters;
// One ADPCM block decodes to 28 samples
#define ADPCM_BLOCK_BYTES 16
#define ADPCM_BLOCK_SAMPLES 28
#define ENV_KEYON_DELAY_TICKS 6 // latency to simulate PS1 SPU latching a key-on

static inline Sint16 clamp16(Sint32 v)
{
    if (v < -32768)
        return -32768;
    if (v > 32767)
        return 32767;
    return (Sint16)v;
}

static inline Sint32 sign4(Sint32 n)
{
    return (n & 0x8) ? (n - 0x10) : n;
}

// Decode one 16-byte SPU ADPCM block into 28 Sint16 samples. PS1 SPU ADPCM:
//   block[0]    = shift_filter: low nibble = shift (12-shift_in, or 9 if >12)
//                               high nibble = filter index (clamped 0..4)
//   block[1]    = flags: bit0 = loop-end, bit1 = repeat, bit2 = loop-start
//   block[2..15]= 14 bytes of 4-bit nibbles (low nibble first)
static inline void spu_adpcm_decode_block(const Uint8 block[ADPCM_BLOCK_BYTES], Sint16 *hist1,
                                          Sint16 *hist2, Sint16 out[ADPCM_BLOCK_SAMPLES],
                                          Uint8 *flags_out)
{
    static const Sint32 pos[5] = {0, 60, 115, 98, 122};
    static const Sint32 neg[5] = {0, 0, -52, -55, -60};

    Sint32 shift_in = block[0] & 0x0F;
    Sint32 shift = (shift_in > 12) ? 9 : (12 - shift_in);
    Sint32 filter = (block[0] >> 4) & 0x07;
    if (filter > 4)
        filter = 4;
    Sint32 f0 = pos[filter];
    Sint32 f1 = neg[filter];
    if (flags_out)
        *flags_out = block[1];

    Sint16 prev = *hist1;
    Sint16 prev2 = *hist2;
    for (Sint32 i = 0; i < 14; i++)
    {
        Uint16 byte = block[2 + i];
        for (Sint32 n = 0; n < 2; n++)
        {
            Sint32 t = sign4((byte >> (n * 4)) & 0x0F);
            Sint32 s = t * (1 << shift) + ((prev * f0) >> 6) + ((prev2 * f1) >> 6);
            Sint16 final = clamp16(s);
            out[i * 2 + n] = final;
            prev2 = prev;
            prev = final;
        }
    }
    *hist1 = prev;
    *hist2 = prev2;
}

typedef enum
{
    ADSR_ATTACK = 0,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    ADSR_OFF,
} AdsrState;

typedef struct
{
    Uint32 cur_addr;    // current 16-byte block address in SPU RAM
    Uint32 repeat_addr; // from voice loop_addr reg, or block flag bit 2
    Sint16 hist1, hist2;
    Sint16 samples[ADPCM_BLOCK_SAMPLES];
    Uint8 block_flags; // flags byte of the most-recently-decoded block
    Uint8 sample_idx;  // next sample to consume from samples[] (0..28)
    Uint32 spos;       // 16.16 fixed-point counter (1.0 = 0x10000)
    Uint32 sinc;       // pitch_reg << 4 is the per-output-tick increment
    Sint16 gwin[4];    // GAUSS interpolation window
    Uint8 gpos;        // index to next decoded sample in gwin
    Uint8 active;
    Uint8 needs_decode;
    Sint32 env_vol;      // mirrors SotnVoiceRegisters::volumex
    Uint32 env_counter;  // rate divider, increases by 1 for all voice duration
    AdsrState env_state; // voice phase, works like a state machine
    Uint8 key_off;       // set by key-off; drives the release phase
    Uint8 delay_ticks;   // key-on -> envelope-start pipeline latency
    Uint16 adsr_lo;      // mirrors SotnVoiceRegisters::adsr[0]
    Uint16 adsr_hi;      // mirrors SotnVoiceRegisters::adsr[1]
} SotnVoice;

static inline void spu_voice_key_on(SotnVoice *vs, volatile SotnVoiceRegisters *reg)
{
    vs->cur_addr = ((Uint32)reg->start_addr << 3) & (SOTN_SPU_RAM_SIZE - 1);
    vs->repeat_addr = ((Uint32)reg->loop_addr << 3) & (SOTN_SPU_RAM_SIZE - 1);
    vs->hist1 = vs->hist2 = 0;
    vs->sample_idx = ADPCM_BLOCK_SAMPLES; // force decode on first consume
    vs->block_flags = 0;
    vs->needs_decode = 1;
    // sinc = pitch << 4; pitch=0x1000 -> sinc=0x10000 (1.0 in 16.16)
    Uint32 pitch = reg->pitch;
    vs->sinc = (pitch & 0x3FFF) << 4;
    if (vs->sinc == 0)
        vs->sinc = 1;

    // Start spos at 1.0 so the first voice_step consumes exactly one sample.
    // The gauss window stays mostly zero-padded for several ticks, producing
    // the silent warmup-then-ringing pattern observed in PS1 captures
    vs->spos = 0x10000;

    // We do not reset vs->gpos at key-on for accuracy. On real PS1 hardware,
    // a voice on will carry the residual state from prior voice activity
    vs->active = 1;

    // reset the envelope to a fresh attack from zero
    vs->adsr_lo = reg->adsr[0];
    vs->adsr_hi = reg->adsr[1];
    vs->env_vol = 0;
    vs->env_counter = 0;
    vs->env_state = ADSR_ATTACK;
    vs->key_off = 0;
    vs->delay_ticks = ENV_KEYON_DELAY_TICKS;
    reg->volumex = 0;
}

static inline Sint32 spu_voice_decode_one_sample(SotnVoice *vs, const Uint8 *ram)
{
    if (vs->sample_idx >= ADPCM_BLOCK_SAMPLES)
    {
        // end of block, and handle loop/end flags
        if (!vs->needs_decode && (vs->block_flags & 0x01))
        {
            if (!(vs->block_flags & 0x02))
            {
                vs->active = 0;
                vs->gwin[vs->gpos] = 0;
                vs->gpos = (Uint8)((vs->gpos + 1) & 3);
                return 0;
            }
            vs->cur_addr = vs->repeat_addr;
        }
        vs->needs_decode = 1;
        vs->sample_idx = 0;
    }
    if (vs->needs_decode)
    {
        const Uint8 *block = &ram[vs->cur_addr];
        Uint8 wrapped[ADPCM_BLOCK_BYTES];
        if (vs->cur_addr > SOTN_SPU_RAM_SIZE - ADPCM_BLOCK_BYTES)
        {
            for (Sint32 i = 0; i < ADPCM_BLOCK_BYTES; i++)
                wrapped[i] = ram[(vs->cur_addr + i) & (SOTN_SPU_RAM_SIZE - 1)];
            block = wrapped;
        }
        spu_adpcm_decode_block(block, &vs->hist1, &vs->hist2, vs->samples, &vs->block_flags);
        if (vs->block_flags & 0x04)
        {
            vs->repeat_addr = vs->cur_addr;
        }
        vs->cur_addr = (vs->cur_addr + ADPCM_BLOCK_BYTES) & (SOTN_SPU_RAM_SIZE - 1);
        vs->needs_decode = 0;
    }
    Sint16 s = vs->samples[vs->sample_idx++];
    vs->gwin[vs->gpos] = s;
    vs->gpos = (Uint8)((vs->gpos + 1) & 3);
    return 1;
}

// Advance the voice by one output-rate tick (44.1 kHz) and return its
// pitch-resampled, gauss-interpolated Sint16 sample.
static inline Sint16 spu_voice_step(SotnVoice *vs, Uint32 pitch_reg, const Uint8 *ram)
{
    // for pitch changes during voice on, enable vibrato or bends
    Uint32 pitch = pitch_reg & 0x3FFF;
    vs->sinc = pitch ? pitch << 4 : 1;

    // consume decoded samples until the pitch counter is below 1.0
    while (vs->spos >= 0x10000)
    {
        if (!spu_voice_decode_one_sample(vs, ram))
        {
            // Voice stopped mid-decode; flush remaining ticks as zero.
            vs->spos = 0;
            break;
        }
        vs->spos -= 0x10000;
    }

    // GAUSS interpolation
    Sint32 vl = (vs->spos >> 6) & ~3;
    Sint32 g0 = vs->gwin[vs->gpos & 3];
    Sint32 g1 = vs->gwin[(vs->gpos + 1) & 3];
    Sint32 g2 = vs->gwin[(vs->gpos + 2) & 3];
    Sint32 g3 = vs->gwin[(vs->gpos + 3) & 3];
    Sint32 acc = (spu_gauss_tbl[vl + 0] * g0) & ~2047;
    acc += (spu_gauss_tbl[vl + 1] * g1) & ~2047;
    acc += (spu_gauss_tbl[vl + 2] * g2) & ~2047;
    acc += (spu_gauss_tbl[vl + 3] * g3) & ~2047;
    vs->spos += vs->sinc;
    // each tap keeps 11 fractional bits and the taps sum to 0x800: unity gain
    return clamp16(acc >> 11);
}

static inline Uint32 adsr_denominator(Sint32 rate)
{
    return rate < 48 ? 1u : (1u << ((rate >> 2) - 11));
}

static inline Sint32 adsr_num_increase(Sint32 rate)
{
    return rate < 48 ? (7 - (rate & 3)) << (11 - (rate >> 2)) : (7 - (rate & 3));
}

static inline Sint32 adsr_num_decrease(Sint32 rate)
{
    // (-8 + (rate & 3)) is always negative; shifting it left is undefined,
    // so scale by the equivalent power of two instead.
    return rate < 48 ? (-8 + (rate & 3)) * (1 << (11 - (rate >> 2))) : (-8 + (rate & 3));
}

// process voice ADSR envelope by one sample, calculate ADSR envelope volume
static inline void spu_voice_envelope_step(SotnVoice *vs)
{
    if (vs->delay_ticks)
    { // simulate PS1 SPU key on latency
        vs->delay_ticks--;
        return;
    }
    if (vs->key_off && vs->env_state != ADSR_OFF)
    {
        vs->env_state = ADSR_RELEASE;
    }

    // the PS1 SPU has an internal counter, shared across all states, that
    // allows to trigger a change based on the selected ADSR rate
    Uint32 ctr = ++vs->env_counter;
#define ADSR_FIRES(rate) ((ctr % adsr_denominator(rate)) == 0)

    switch (vs->env_state)
    {
    case ADSR_ATTACK: {
        Sint32 attack_rate = (vs->adsr_lo >> 8) & 0x7F;
        Sint32 attack_exp = (vs->adsr_lo >> 15) & 1;
        Sint32 rate = attack_rate;
        if (attack_exp && vs->env_vol >= 0x6000)
        {
            rate += 8;
        }
        if (ADSR_FIRES(rate))
        {
            vs->env_vol += adsr_num_increase(rate);
            if (vs->env_vol >= 0x7FFF)
            {
                vs->env_vol = 0x7FFF;
                vs->env_state = ADSR_DECAY;
            }
        }
        break;
    }
    case ADSR_DECAY: {
        Sint32 decay_rate = (vs->adsr_lo >> 4) & 0x0F;
        Sint32 rate = decay_rate * 4; // decay is always exponential decrease
        if (ADSR_FIRES(rate))
        {
            vs->env_vol += (adsr_num_decrease(rate) * vs->env_vol) >> 15;
            if (vs->env_vol < 0)
            {
                vs->env_vol = 0;
            }
            Sint32 sustain_level = vs->adsr_lo & 0x0F;
            if (((vs->env_vol >> 11) & 0xF) <= sustain_level)
            {
                vs->env_state = ADSR_SUSTAIN;
            }
        }
        break;
    }
    case ADSR_SUSTAIN: {
        Sint32 sustain_rate = (vs->adsr_hi >> 6) & 0x7F;
        Sint32 sustain_dec = (vs->adsr_hi >> 14) & 1;
        Sint32 sustain_exp = (vs->adsr_hi >> 15) & 1;
        Sint32 rate = sustain_rate;
        if (!sustain_dec)
        {
            if (sustain_exp && vs->env_vol >= 0x6000)
            {
                rate += 8;
            }
            if (ADSR_FIRES(rate))
            {
                vs->env_vol += adsr_num_increase(rate);
                if (vs->env_vol > 0x7FFF)
                {
                    vs->env_vol = 0x7FFF;
                }
            }
        }
        else
        {
            if (ADSR_FIRES(rate))
            {
                if (sustain_exp)
                {
                    vs->env_vol += (adsr_num_decrease(rate) * vs->env_vol) >> 15;
                }
                else
                {
                    vs->env_vol += adsr_num_decrease(rate);
                }
                if (vs->env_vol < 0)
                {
                    vs->env_vol = 0;
                }
            }
        }
        break;
    }
    case ADSR_RELEASE: {
        Sint32 release_rate = vs->adsr_hi & 0x1F;
        Sint32 rate = release_rate * 4;
        if (ADSR_FIRES(rate))
        {
            Sint32 release_exp = (vs->adsr_hi >> 5) & 1;
            if (release_exp)
            {
                vs->env_vol += (adsr_num_decrease(rate) * vs->env_vol) >> 15;
            }
            else
            {
                vs->env_vol += adsr_num_decrease(rate);
            }
            if (vs->env_vol <= 0)
            {
                vs->env_vol = 0;
                vs->env_state = ADSR_OFF;
                vs->active = 0;
            }
        }
        break;
    }
    default:
        break;
    }
#undef ADSR_FIRES
}

#endif
