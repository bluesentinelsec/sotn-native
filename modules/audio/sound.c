// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected event/script/channel paths adapted from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, src/dra/sound.c.
// VAB access uses checked serialized fields, never SDK structs or PS1 host pointers.
#include "voice.h"
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
} SoundScript;

struct SotnSound
{
    SotnDataView dra;
    Uint8 ram[SOTN_SPU_RAM_SIZE];
    SoundBank banks[4];
    SotnVoice voices[24];
    SotnVoiceRegisters registers[24];
    SoundScript scripts[3];
    Uint16 queue[256], queued;
    Uint8 priority[4];
};

static Uint16 U16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 U32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static bool View(SotnDataView image, size_t offset, size_t count, SotnDataView *out)
{
    if (image.bytes == NULL || offset > image.size || count > image.size - offset)
        return SDL_SetError("Sound table is outside its bounded private image");
    *out = (SotnDataView){image.bytes + offset, count};
    return true;
}

static bool Dra(SotnSound *sound, Uint64 address, size_t count, SotnDataView *out)
{
    if (address < 0x800a0000 || address > SDL_MAX_UINT32)
        return SDL_SetError("Sound script references an invalid US DRA address");
    return View(sound->dra, (size_t)(address - 0x800a0000), count, out);
}

static bool Bank(SotnSound *sound, Uint32 id, SotnDataView header, SotnDataView body)
{
    SotnDataView record = {0};
    if (!View(header, 0, 2080, &record) || U32(record.bytes) != 0x56414270 ||
        U32(record.bytes + 4) != 7 || U16(record.bytes + 18) > 128 ||
        U16(record.bytes + 22) > 255 || record.bytes[24] > 127 || record.bytes[25] > 127 ||
        !View(sound->dra, 0x1d1c8 + id * 4, 4, &record))
        return SDL_SetError("Sound bank has an invalid VAB header or SPU address table");
    SoundBank *bank = &sound->banks[id];
    bank->address = U32(record.bytes);
    const Uint16 program_count = U16(header.bytes + 18), sample_count = U16(header.bytes + 22);
    const size_t size_table = 2080 + (size_t)program_count * 512;
    if (bank->address % 16 != 0 || bank->address > SOTN_SPU_RAM_SIZE || body.bytes == NULL ||
        body.size > SOTN_SPU_RAM_SIZE - bank->address || !View(header, size_table, 512, &record))
        return SDL_SetError("VAB tones/samples exceed bounded sound memory");
    Uint16 used = 0;
    for (Uint32 i = 0; i < 128; ++i)
    {
        bank->programs[i] = SDL_MAX_UINT16;
        const Uint8 *program = header.bytes + 32 + i * 16;
        if (program[0] > 16 || program[1] > 127 || program[4] > 127)
            return SDL_SetError("VAB program attributes are invalid");
        if (program[0] != 0)
            bank->programs[i] = used++;
    }
    if (used != program_count)
        return SDL_SetError("VAB compact program count does not match its header");
    size_t cursor = (size_t)U16(record.bytes) * 8;
    for (Uint32 i = 1; i <= sample_count; ++i)
    {
        const size_t size = (size_t)U16(record.bytes + i * 2) * 8;
        if (cursor > body.size || size > body.size - cursor || size % 16 != 0)
            return SDL_SetError("VAB sample sizes exceed the private body or block alignment");
        bank->samples[i] = bank->address + (Uint32)cursor;
        bank->sizes[i] = (Uint32)size;
        cursor += size;
    }
    SDL_memcpy(sound->ram + bank->address, body.bytes, body.size);
    bank->header = header;
    bank->sample_count = sample_count;
    bank->loaded = true;
    return true;
}

static bool Key(SotnSound *sound, Uint32 voice, Uint8 bank_id, Uint8 program, Uint8 tone,
                Uint8 note, Uint8 volume)
{
    if (voice >= 24 || bank_id >= 4 || !sound->banks[bank_id].loaded || program >= 128 ||
        tone >= 16 || note > 127 || volume > 127)
        return SDL_SetError("Sound event requests an unsupported voice, bank, or note");
    SoundBank *bank = &sound->banks[bank_id];
    const Uint16 compact = bank->programs[program];
    if (compact == SDL_MAX_UINT16)
        return SDL_SetError("Sound event references an absent VAB program");
    const Uint8 *attributes = bank->header.bytes + 2080 + (size_t)compact * 512 + tone * 32;
    const Uint16 sample = U16(attributes + 22);
    if (sample == 0)
        return true;
    if (sample > bank->sample_count || bank->sizes[sample] == 0 || attributes[2] > 127 ||
        attributes[3] > 127 || attributes[4] > 127 || attributes[5] > 127)
        return SDL_SetError("Sound tone references an invalid sample or pitch/volume attribute");
    const Sint32 semitones = (Sint32)note + 60 - attributes[4];
    if (semitones < 0 || semitones > 187)
        return SDL_SetError("Sound tone pitch is outside the supported original note range");
    const Sint32 octave = semitones / 12;
    Uint32 pitch = (Uint32)SDL_floor(
        4096.0 * SDL_pow(2.0, ((semitones % 12) * 16 + attributes[5] / 8) / 192.0));
    pitch = octave >= 5 ? (pitch << (octave - 5)) & 65535U : pitch >> (5 - octave);
    const Uint8 *prog = bank->header.bytes + 32 + program * 16;
    Uint32 seed = (Uint32)volume * bank->header.bytes[24] * 16383U / 16129U;
    seed = seed * prog[1] * attributes[2] / 16129U;
    Uint32 left = seed, right = seed;
    const Uint8 pans[3] = {attributes[3], prog[4], 64};
    for (Uint32 i = 0; i < 3; ++i)
    {
        if (pans[i] < 64)
            left = right * pans[i] / 63;
        else
            right = left * (127U - pans[i]) / 63;
    }
    SotnVoiceRegisters *reg = &sound->registers[voice];
    reg->start_addr = (Uint16)(bank->samples[sample] / 8);
    reg->loop_addr = reg->start_addr;
    reg->pitch = (Uint16)pitch;
    reg->adsr[0] = U16(attributes + 16);
    reg->adsr[1] = U16(attributes + 18);
    reg->left = (Sint32)(left * left / 16383);
    reg->right = (Sint32)(right * right / 16383);
    spu_voice_key_on(&sound->voices[voice], reg);
    return true;
}

static bool Pair(SotnSound *sound, Uint32 voice, Uint8 bank, Uint8 program, Uint8 tone, Uint8 note,
                 Uint8 volume)
{
    if (tone > 14)
        return SDL_SetError("Stereo sound requires a bounded pair of VAB tones");
    return Key(sound, voice, bank, program, tone, note, volume) &&
           Key(sound, voice + 1, bank, program, (Uint8)(tone + 1), note, volume);
}

static bool Event(SotnSound *sound, Uint16 id)
{
    SotnDataView definition = {0};
    if (!View(sound->dra, 0x1f554 + (size_t)(id - 0x600) * 7, 7, &definition))
        return false;
    const Uint8 *data = definition.bytes;
    const Uint8 volume = (Uint8)((127U * data[3]) >> 7);
    if (data[3] > 127)
        return SDL_SetError("Sound event has an invalid original volume");
    if (data[0] == 9)
    {
        SotnDataView pointer = {0};
        if (data[1] >= 171 || !View(sound->dra, 0x21ecc + ((size_t)data[1] + 1) * 4, 4, &pointer))
            return SDL_SetError("Sound event has an invalid script index");
        Uint32 selected = 3;
        for (Uint32 i = 0; i < 3; ++i)
            if (sound->scripts[i].id == id)
            {
                selected = i;
                break;
            }
        if (selected == 3)
            for (Uint32 i = 0; i < 3; ++i)
                if (sound->scripts[i].id == 0)
                {
                    selected = i;
                    break;
                }
        if (selected == 3)
            for (Uint32 i = 0; i < 3; ++i)
                if (data[6] >= sound->scripts[i].priority)
                {
                    selected = i;
                    break;
                }
        if (selected < 3)
        {
            for (Uint32 voice = selected * 4; voice < selected * 4 + 4; ++voice)
                sound->voices[voice].key_off = 1;
            sound->scripts[selected] = (SoundScript){U32(pointer.bytes), id, 0, 0, volume, data[6]};
        }
        return true;
    }
    Uint32 voice = 22;
    if (data[4] == 1)
        voice = 20;
    else if (data[4] == 2)
    {
        sound->voices[22].key_off = sound->voices[23].key_off = 1;
        return true;
    }
    else if (data[4] == 4)
    {
        Uint32 slot = 4;
        for (Uint32 i = 0; i < 4; ++i)
            if (!sound->voices[12 + i * 2].active && !sound->voices[13 + i * 2].active)
            {
                slot = i;
                break;
            }
        if (slot == 4)
            for (Uint32 i = 0; i < 4; ++i)
                if (data[6] >= sound->priority[i])
                {
                    slot = i;
                    break;
                }
        if (slot == 4)
            return true;
        sound->priority[slot] = data[6];
        voice = 12 + slot * 2;
    }
    else if (data[4] != 0)
        return SDL_SetError("Sound event requests an unintegrated channel mode");
    return Pair(sound, voice, data[0], data[1], data[5], data[2], volume);
}

bool SotnSound_Create(SotnSoundSources sources, SotnSound **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnSound *sound = SDL_calloc(1, sizeof(*sound));
    if (sound == NULL)
        return false;
    sound->dra = sources.dra;
    SotnDataView common_header = {0};
    if (!View(sources.dra, 0x1107c, 0x3000, &common_header) ||
        !Bank(sound, 0, common_header, sources.common_body) ||
        !Bank(sound, 1, sources.player_header, sources.player_body) ||
        !Bank(sound, 3, sources.stage_header, sources.stage_body))
    {
        SotnSound_Destroy(sound);
        return false;
    }
    *out = sound;
    return true;
}

bool SotnSound_Queue(SotnSound *sound, Uint16 id)
{
    if (sound == NULL || id <= 0x600 || id > 0x8e0 || sound->queued >= SDL_arraysize(sound->queue))
        return SDL_SetError("Sound event is invalid or the bounded queue is full");
    sound->queue[sound->queued++] = id;
    return true;
}

bool SotnSound_Tick(SotnSound *sound, Sint16 *pcm, size_t count)
{
    if (sound == NULL || pcm == NULL || count < SOTN_SOUND_FRAMES * 2)
        return SDL_SetError("Sound tick requires state and a full stereo PCM buffer");
    for (Uint32 i = 0; i < sound->queued; ++i)
        if (!Event(sound, sound->queue[i]))
            return false;
    sound->queued = 0;
    for (Uint32 i = 0; i < 3; ++i)
    {
        SoundScript *script = &sound->scripts[i];
        if (script->id == 0)
            continue;
        if (script->timer != 0)
            --script->timer;
        else
        {
            SotnDataView entry = {0};
            if (++script->steps > 512 || !Dra(sound, script->address, 1, &entry))
                return SDL_SetError("Sound script exceeded its bounded record stream");
            if (entry.bytes[0] == 255)
                script->id = 0;
            else
            {
                if (!Dra(sound, script->address, 6, &entry))
                    return false;
                Uint32 voice = i * 4;
                if (sound->voices[voice].active)
                    voice += 2;
                if (!Pair(sound, voice, entry.bytes[0], entry.bytes[1], entry.bytes[3],
                          entry.bytes[2], (Uint8)((script->volume * entry.bytes[4]) >> 7)))
                    return false;
                script->timer = entry.bytes[5];
                script->address += 6;
            }
        }
    }
    for (Uint32 frame = 0; frame < SOTN_SOUND_FRAMES; ++frame)
    {
        Sint32 left = 0, right = 0;
        for (Uint32 v = 0; v < 24; ++v)
        {
            SotnVoice *voice = &sound->voices[v];
            if (!voice->active)
                continue;
            Sint32 sample = spu_voice_step(voice, sound->registers[v].pitch, sound->ram);
            spu_voice_envelope_step(voice);
            if (voice->env_state == ADSR_OFF)
                sample = 0;
            sample = (sample * voice->env_vol) >> 15;
            left += (sample * (sound->registers[v].left * 2)) >> 15;
            right += (sample * (sound->registers[v].right * 2)) >> 15;
        }
        pcm[frame * 2] = clamp16(left);
        pcm[frame * 2 + 1] = clamp16(right);
    }
    return true;
}

void SotnSound_Destroy(SotnSound *sound)
{
    SDL_free(sound);
}
