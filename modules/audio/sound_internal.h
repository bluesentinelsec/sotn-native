#ifndef SOTN_SOUND_INTERNAL_H
#define SOTN_SOUND_INTERNAL_H
#include "reverb.h"
#include "voice.h"
#include <sotn/sequence.h>
#include <sotn/sound.h>
typedef struct SoundBank
{
    SotnDataView header;
    Uint32 address, body_size, samples[256], sizes[256];
    Uint16 programs[128], sample_count;
    bool loaded;
} SoundBank;

typedef struct SoundScript
{
    Uint32 address;
    Uint16 id, steps, allocation_id;
    Uint8 timer, volume, priority, mode;
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
    Uint16 sequence_id;
    bool sequence_open; /**< Original SEQ access remains open after its event cursor ends. */
    Uint16 sequence_silence[16];
    Uint8 ram[SOTN_SPU_RAM_SIZE];
    SoundBank banks[4];
    SotnVoice voices[24];
    SotnVoiceRegisters registers[24];
    SotnReverb reverb;
    Uint32 reverb_voices;
    Uint8
        reverb_restore_depth; /**< Original BSS g_ReverbDepth; distinct from temporary SDK gain. */
    SoundScript scripts[4], script_backup[4];
    bool scripts_paused;
    struct
    {
        Uint16 id, volume;
        Sint8 pan;
        bool ambience_volume;
    } queue[256];
    Uint16 queued, ambience_id;
    Uint8 priority[4];
};

/** @brief Apply original script-cursor pause, retaining already playing voices and mode-five
 * scripts.
 * @param sound Non-NULL actual sound owner. Backup slots retain original order/lifetime. */
void SoundPauseScripts(SotnSound *sound);
/** @brief Restore original copied cursors into free allocation channels without retriggering
 * voices.
 * @param sound Non-NULL actual owner. Preserves original unreserved restore/overwrite behavior. */
void SoundResumeScripts(SotnSound *sound);
/** @brief Original SetReleaseRate2/InitSoundVars2 on all24 live hardware voices.
 * Preserves envelopes, decoder/RAM state, pending effects and paused script copies. */
void SoundReleaseAll(SotnSound *sound);
/** @brief Actual SDK RR-only write/key-off for the selected hardware mask. */
void SoundReleaseVoices(SotnSound *sound, Uint32 mask, Uint8 rate);
/** @brief Original InitSoundVars1 metadata after actual StopSeq/CD shutdown.
 * Retains live envelopes, DSP/RAM and copied scripts; clears pending effects/SEQ attachment. */
void SoundResetStopped(SotnSound *sound);

/** @brief Start one validated voice, invalidating any previous sequence owner. */
bool SoundKey(SotnSound *sound, Uint32 voice, Uint8 bank, Uint8 program, Uint8 tone, Uint8 note,
              Uint8 volume, Uint8 pan);
/** @brief Flush selected original envelope retirement and advance sequence scheduling. */
bool SoundSequenceTick(SotnSound *sound);
#endif
