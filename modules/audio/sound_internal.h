#ifndef SOTN_SOUND_INTERNAL_H
#define SOTN_SOUND_INTERNAL_H
#include "voice.h"
#include <sotn/sequence.h>
#include <sotn/sound.h>
typedef struct SoundBank
{
    SotnDataView header;
    Uint32 address, samples[256], sizes[256];
    Uint16 programs[128], sample_count;
    bool loaded;
} SoundBank;

typedef struct SoundScript
{
    Uint32 address;
    Uint16 id, steps;
    Uint8 timer, volume, priority;
    Sint8 pan;
} SoundScript;

typedef struct SoundSeqVoice
{
    Uint16 age;
    Uint8 bank, program, tone, note, velocity, volume, pan, priority;
    bool claimed, sequence;
} SoundSeqVoice;

struct SotnSound
{
    SotnDataView dra, executable, sequence_data;
    SotnSequence sequence;
    SoundSeqVoice sequence_voices[16];
    Uint8 sequence_volume, sequence_flush;
    Uint16 sequence_silence[16];
    Uint8 ram[SOTN_SPU_RAM_SIZE];
    SoundBank banks[4];
    SotnVoice voices[24];
    SotnVoiceRegisters registers[24];
    SoundScript scripts[3];
    struct
    {
        Uint16 id, volume;
        Sint8 pan;
        bool ambience_volume;
    } queue[256];
    Uint16 queued, ambience_id;
    Uint8 priority[4];
};

/** @brief Start one validated voice, invalidating any previous sequence owner. */
bool SoundKey(SotnSound *sound, Uint32 voice, Uint8 bank, Uint8 program, Uint8 tone, Uint8 note,
              Uint8 volume, Uint8 pan);
/** @brief Flush selected original envelope retirement and advance sequence scheduling. */
bool SoundSequenceTick(SotnSound *sound);
#endif
