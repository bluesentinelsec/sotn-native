// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected reconstructed SDK voice allocation, note2pitch, pan/volume and bend rules:
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, main/psxsdk/libsnd/vmanager.c.
// SDK algorithms are MIT-licensed; selected playback selection follows dra/sound.c
// (AGPL-3.0-or-later). Original pitch/SEQ/VAB tables remain private inputs. Reverb is not
// implemented here.
#include "sound_internal.h"
static Uint16 U16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static bool Tone(SotnSound *s, Uint8 bank_id, Uint8 program, Uint8 index, const Uint8 **out)
{
    if (bank_id > 3 || program > 127 || index > 15 || !s->banks[bank_id].loaded)
        return SDL_SetError("SEQ tone requires an initialized VAB bank, program and tone");
    const SoundBank *bank = &s->banks[bank_id];
    const Uint16 compact = bank->programs[program];
    if (compact == SDL_MAX_UINT16)
        return SDL_SetError("SEQ references an absent VAB program %u", program);
    const size_t offset = 2080 + (size_t)compact * 512 + index * 32U;
    if (offset > bank->header.size || 32 > bank->header.size - offset)
        return SDL_SetError("SEQ tone is outside its bounded private VAB");
    *out = bank->header.bytes + offset;
    return true;
}
static bool Pitch(SotnSound *s, Uint16 voice, const Uint8 *tone, Sint32 note, Uint16 fine)
{
    Sint32 fraction = ((Sint32)fine + tone[5]) / 8, carry = 0;
    if (fraction >= 16)
    {
        carry = 1;
        fraction -= 16;
    }
    const Sint32 semitones = note + 60 - tone[4] + carry;
    if (fraction < 0 || fraction > 15 || semitones < 0 || semitones > 255)
        return SDL_SetError("SEQ note/bend exceeds the original bounded pitch table");
    const Sint32 octave = semitones / 12 - 5;
    if (octave > 15 || octave < -15)
        return SDL_SetError("SEQ pitch shift exceeds the original register range");
    const Uint32 index = (Uint32)(semitones % 12) * 16U + (Uint32)fraction;
    const size_t offset = 0x23714 + index * 2U;
    if (!s->executable.bytes || offset > s->executable.size || 2 > s->executable.size - offset)
        return SDL_SetError("SEQ SDK pitch table is truncated");
    Uint32 pitch = U16(s->executable.bytes + offset);
    if (pitch < 4096 || pitch >= 8192)
        return SDL_SetError("SEQ SDK pitch entry is outside its original octave");
    pitch = octave > 0 ? pitch << octave : pitch >> -octave;
    s->registers[voice].pitch = (Uint16)pitch;
    return true;
}
static bool Volume(SotnSound *s, Uint16 voice, const Uint8 *tone)
{
    const SoundSeqVoice *note = &s->sequence_voices[voice];
    const SoundBank *bank = &s->banks[note->bank];
    const Uint8 *program = bank->header.bytes + 32 + note->program * 16U;
    if (note->volume > 127 || note->pan > 127 || program[1] > 127 || program[4] > 127 ||
        tone[2] > 127 || tone[3] > 127 || s->sequence_volume > 127)
        return SDL_SetError("SEQ volume/pan attributes exceed seven bits");
    const Uint32 velocity = (Uint32)note->velocity * note->volume / 127U;
    Uint32 level = (velocity * bank->header.bytes[24] * 16383U / 127U) / 127U;
    level = ((level * program[1] * tone[2]) / 127U) / 127U;
    level = level * s->sequence_volume / 127U;
    Uint32 left = level, right = level;
    const Uint8 pans[] = {tone[3], program[4], note->pan};
    for (Uint16 i = 0; i < 3; ++i)
        if (pans[i] < 64)
            right = right * pans[i] / 63U;
        else
            left = left * (127U - pans[i]) / 63U;
    s->registers[voice].left = (Sint32)(left * left / 16383U);
    s->registers[voice].right = (Sint32)(right * right / 16383U);
    return true;
}
static Uint16 Allocate(SotnSound *s, Uint8 priority)
{
    Uint16 selected = 16, level = SDL_MAX_UINT16, age = 0;
    for (Uint16 i = 0; i < 16; ++i)
    {
        SoundSeqVoice *a = &s->sequence_voices[i];
        if (!a->claimed && s->voices[i].env_vol == 0)
        {
            selected = i;
            break;
        }
        const Uint16 envelope = (Uint16)s->voices[i].env_vol;
        if (a->priority < priority)
        {
            selected = i;
            priority = a->priority;
            level = envelope;
            age = a->age;
        }
        else if (a->priority == priority &&
                 (envelope < level || (envelope == level && a->age > age)))
        {
            selected = i;
            level = envelope;
            age = a->age;
        }
    }
    if (selected < 16)
    {
        for (Uint16 i = 0; i < 16; ++i)
            s->sequence_voices[i].age = (Uint16)(s->sequence_voices[i].age + 1U);
        s->sequence_voices[selected].age = 0;
    }
    return selected;
}
static bool Note(SotnSound *s, SotnSequenceEvent event)
{
    if (!event.value)
    {
        for (Uint16 i = 0; i < 16; ++i)
        {
            SoundSeqVoice *a = &s->sequence_voices[i];
            if (a->sequence && a->bank == event.bank && a->program == event.program &&
                a->note == event.note)
            {
                a->claimed = false;
                s->voices[i].key_off = 1;
            }
        }
        return true;
    }
    if (event.bank > 3 || event.program > 127 || !s->banks[event.bank].loaded)
        return SDL_SetError("SEQ note references an unavailable bank/program");
    const SoundBank *bank = &s->banks[event.bank];
    const Uint8 count = bank->header.bytes[32 + event.program * 16U];
    if (count > 16 || bank->programs[event.program] == SDL_MAX_UINT16)
        return SDL_SetError("SEQ note references an invalid instrument tone count");
    for (Uint8 i = 0; i < count; ++i)
    {
        const Uint8 *tone = NULL;
        if (!Tone(s, event.bank, event.program, i, &tone))
            return false;
        if (tone[6] > tone[7])
            return SDL_SetError("SEQ tone has an invalid note range");
        if (event.note < tone[6] || event.note > tone[7] || U16(tone + 22) == 0)
            continue;
        const Uint16 voice = Allocate(s, tone[0]);
        if (voice == 16)
            continue;
        if (!SoundKey(s, voice, event.bank, event.program, i, event.note, event.value, event.pan))
            return false;
        s->sequence_voices[voice] = (SoundSeqVoice){.bank = event.bank,
                                                    .program = event.program,
                                                    .tone = i,
                                                    .note = event.note,
                                                    .velocity = event.value,
                                                    .volume = event.volume,
                                                    .pan = event.pan,
                                                    .priority = tone[0],
                                                    .claimed = true,
                                                    .sequence = true};
        if (!Pitch(s, voice, tone, event.note, 0) || !Volume(s, voice, tone))
            return false;
    }
    return true;
}
static bool Event(void *context, SotnSequenceEvent event)
{
    SotnSound *s = context;
    if (event.kind == SOTN_SEQUENCE_NOTE)
        return Note(s, event);
    if (event.kind == SOTN_SEQUENCE_STOP)
    {
        SotnSound_StopSequence(s);
        return true;
    }
    for (Uint16 i = 0; i < 16; ++i)
    {
        SoundSeqVoice *a = &s->sequence_voices[i];
        if (!a->sequence || a->bank != event.bank || a->program != event.program)
            continue;
        const Uint8 *tone = NULL;
        if (!Tone(s, a->bank, a->program, a->tone, &tone))
            return false;
        if (event.kind == SOTN_SEQUENCE_BEND)
        {
            const Sint32 bend = (Sint32)event.value - 64;
            Sint32 note = a->note;
            Uint16 fine = 0;
            if (bend > 0)
            {
                note += bend * tone[13] / 63;
                fine = (Uint16)((bend * tone[13] % 63) * 2);
            }
            else if (bend < 0)
            {
                note += bend * tone[12] / 64 - 1;
                fine = (Uint16)((bend * tone[12] % 64) * 2 + 127);
            }
            if (!Pitch(s, i, tone, note, fine))
                return false;
        }
        else
        {
            if (event.kind == SOTN_SEQUENCE_VOLUME)
                a->volume = event.value;
            else if (event.kind == SOTN_SEQUENCE_PAN)
                a->pan = event.value;
            else
                return SDL_SetError("SEQ receiver has an invalid channel event");
            if (!Volume(s, i, tone))
                return false;
        }
    }
    return true;
}
bool SotnSound_SetSequence(SotnSound *s, SotnDataView executable, SotnDataView data)
{
    SotnSequence probe = {0};
    if (!s || !executable.bytes || executable.size < 0x23894 ||
        !SotnSequence_Open(data, 0, 1, &probe))
        return SDL_SetError(
            "Library SEQ attachment requires complete private event and pitch data");
    SotnSound_StopSequence(s);
    s->sequence = (SotnSequence){0};
    s->executable = executable;
    s->sequence_data = data;
    return true;
}
bool SotnSound_PlaySequence(SotnSound *s, Uint16 id)
{
    if (!s || id != 0x202 || !s->dra.bytes || s->dra.size < 0x1d1e9)
        return SDL_SetError(
            "Sequence playback requires initialized Librarian inputs and US ID 0x202");
    const Uint8 *config = s->dra.bytes + 0x1d1e6;
    if (config[0] > 127 || config[2] >> 4 > 3 || !s->banks[config[2] >> 4].loaded ||
        (config[2] & 15U) > 1)
        return SDL_SetError(
            "Librarian sequence settings reference unsupported volume/bank/loop data");
    SotnSequence next = {0};
    if (!SotnSequence_Open(s->sequence_data, (Uint8)(config[2] >> 4),
                           (Uint16)((config[2] & 15U) ? 0 : 1), &next))
        return false;
    SotnSound_StopSequence(s);
    s->sequence_volume = config[0];
    s->sequence = next;
    return true;
}
void SotnSound_StopSequence(SotnSound *s)
{
    if (!s)
        return;
    s->sequence.playing = false;
    for (Uint16 i = 0; i < 16; ++i)
        if (s->sequence_voices[i].sequence)
        {
            s->sequence_voices[i].claimed = false;
            s->voices[i].key_off = 1;
        }
}
bool SoundSequenceTick(SotnSound *s)
{
    s->sequence_flush = (Uint8)((s->sequence_flush + 1U) & 15U);
    Uint16 silent = 0;
    for (Uint16 i = 0; i < 16; ++i)
        if (s->voices[i].env_vol == 0)
            silent |= (Uint16)(1U << i);
    s->sequence_silence[s->sequence_flush] = silent;
    silent = SDL_MAX_UINT16;
    // Preserve the original SDK's fifteen-bin intersection, including its unused sixteenth bin.
    for (Uint16 i = 0; i < 15; ++i)
        silent &= s->sequence_silence[i];
    for (Uint16 i = 0; i < 16; ++i)
        if (silent & (1U << i))
            s->sequence_voices[i].claimed = false;
    return !s->sequence.data.bytes || SotnSequence_Tick(&s->sequence, Event, s);
}
