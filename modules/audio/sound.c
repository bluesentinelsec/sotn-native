// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected event/script/channel paths adapted from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, src/dra/sound.c.
// VAB access uses checked serialized fields, never SDK structs or PS1 host pointers.
#include "sound_internal.h"

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
    SoundBank candidate = {0};
    SoundBank *bank = &candidate;
    bank->address = U32(record.bytes);
    const Uint16 program_count = U16(header.bytes + 18), sample_count = U16(header.bytes + 22);
    const size_t size_table = 2080 + (size_t)program_count * 512;
    if (bank->address % 16 != 0 || bank->address > SOTN_SPU_RAM_SIZE || body.bytes == NULL ||
        body.size > SOTN_SPU_RAM_SIZE - bank->address || !View(header, size_table, 512, &record))
        return SDL_SetError("VAB tones/samples exceed bounded sound memory");
    if (sound->reverb.configured && body.size &&
        (bank->address >= sound->reverb.base || body.size > sound->reverb.base - bank->address))
        return SDL_SetError("VAB upload would overwrite the actual SPU reverb work area");
    bank->body_size = (Uint32)body.size;
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
    sound->banks[id] = candidate;
    return true;
}

bool SotnSound_SetFamiliarBank(SotnSound *sound, SotnDataView header, SotnDataView body)
{
    if (!sound)
        return SDL_InvalidParamError("sound");
    return Bank(sound, 2, header, body);
}

bool SotnSound_ConfigureReverb(SotnSound *sound, SotnDataView executable)
{
    // Original US SDK _spu_rev_startaddr and _spu_rev_param, Studio B selected by SoundInit.
    // The borrowed MAIN image includes its original0x800-byte executable header.
    const size_t start = 0x23d68 + 3 * 4, record = 0x23db8 + 3 * 68;
    if (!sound || sound->reverb.configured || !executable.bytes || record > executable.size ||
        68 > executable.size - record || U32(executable.bytes + record) != 0)
        return SDL_SetError("SPU initialization requires its bounded private US Studio B profile");
    const Uint32 units = U32(executable.bytes + start);
    if (units >= SOTN_SPU_RAM_SIZE / 8)
        return SDL_SetError("Private SDK reverb work address exceeds actual SPU RAM");
    const Uint32 base = units * 8;
    for (Uint32 i = 0; i < SDL_arraysize(sound->banks); ++i)
        if (sound->banks[i].loaded && sound->banks[i].body_size &&
            (sound->banks[i].address >= base ||
             sound->banks[i].body_size > base - sound->banks[i].address))
            return SDL_SetError("Private SDK reverb work area overlaps an actual loaded VAB");
    SotnReverb next = {0};
    if (!SoundReverb_Configure(&next, base, (SotnDataView){executable.bytes + record + 4, 64}) ||
        !SoundReverb_SetDepth(&next, 10, 10))
        return false;
    SDL_memset(sound->ram + base, 0, SOTN_SPU_RAM_SIZE - base);
    sound->reverb = next;
    return true;
}

bool SotnSound_SetReverbDepth(SotnSound *sound, Sint16 left, Sint16 right)
{
    if (!sound)
        return SDL_InvalidParamError("sound");
    return SoundReverb_SetDepth(&sound->reverb, left, right);
}

void SoundReleaseVoices(SotnSound *sound, Uint32 mask, Uint8 rate)
{
    for (Uint16 i = 0; i < 24; ++i)
    {
        if (!(mask & (1U << i)))
            continue;
        // SDK RR-only writes clear the exponential-release bit as well as the old rate.
        sound->registers[i].adsr[1] = (Uint16)((sound->registers[i].adsr[1] & 0xffc0U) | rate);
        sound->voices[i].adsr_hi = sound->registers[i].adsr[1];
        sound->voices[i].key_off = 1;
    }
}

void SoundReleaseAmbience20(SotnSound *sound)
{
    SoundReleaseVoices(sound, 0x300000, 14);
    sound->ambience_id20 = 0;
}

static void ReleaseAll(SotnSound *sound, Uint8 rate)
{
    SoundReleaseVoices(sound, 0xffffff, rate);
    SDL_memset(sound->scripts, 0, sizeof(sound->scripts));
    SDL_memset(sound->priority, 0, sizeof(sound->priority));
    SDL_memset(sound->effect_ids, 0, sizeof(sound->effect_ids));
    sound->ambience_id = sound->ambience_id20 = 0;
    sound->scripts_paused = false;
}

void SoundReleaseAll(SotnSound *sound)
{
    ReleaseAll(sound, 8);
}

void SoundResetStopped(SotnSound *sound)
{
    SDL_memset(sound->scripts, 0, sizeof(sound->scripts));
    SDL_memset(sound->priority, 0, sizeof(sound->priority));
    SDL_memset(sound->effect_ids, 0, sizeof(sound->effect_ids));
    SDL_memset(sound->queue, 0, sizeof(sound->queue));
    sound->queued = sound->ambience_id = sound->ambience_id20 = 0;
    sound->ambience_volume20 = sound->ambience_volume22 = 0;
    sound->ambience_pan20 = sound->ambience_pan22 = 0;
    sound->queue_read = sound->queue_write = 0;
    sound->scripts_paused = false;
    sound->sequence = (SotnSequence){0};
    sound->sequence_data = (SotnDataView){0};
    sound->sequence_id = 0;
    sound->sequence_volume = 0x70;
}

bool SotnSound_PrepareFamiliarBank(const SotnSound *sound, SotnDataView header, SotnDataView body,
                                   SotnSound **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!sound)
        return SDL_InvalidParamError("sound");
    SotnSound *candidate = SDL_malloc(sizeof(*candidate));
    if (!candidate)
        return false;
    *candidate = *sound;
    if (!SotnSound_SetFamiliarBank(candidate, header, body))
    {
        SDL_free(candidate);
        return false;
    }
    *out = candidate;
    return true;
}

bool SoundKey(SotnSound *sound, Uint32 voice, Uint8 bank_id, Uint8 program, Uint8 tone, Uint8 note,
              Uint8 volume, Uint8 pan)
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
    const Uint8 pans[3] = {attributes[3], prog[4], pan};
    for (Uint32 i = 0; i < 3; ++i)
    {
        if (pans[i] < 64)
            right = right * pans[i] / 63;
        else
            left = left * (127U - pans[i]) / 63;
    }
    SotnVoiceRegisters *reg = &sound->registers[voice];
    reg->start_addr = (Uint16)(bank->samples[sample] / 8);
    reg->loop_addr = reg->start_addr;
    reg->pitch = (Uint16)pitch;
    reg->adsr[0] = U16(attributes + 16);
    reg->adsr[1] = U16(attributes + 18);
    reg->left = (Sint32)(left * left / 16383);
    reg->right = (Sint32)(right * right / 16383);
    if (attributes[1] & 4)
        sound->reverb_voices |= 1U << voice;
    else
        sound->reverb_voices &= ~(1U << voice);
    spu_voice_key_on(&sound->voices[voice], reg);
    if (voice < 16)
        sound->sequence_voices[voice] = (SoundSeqVoice){.claimed = true, .priority = attributes[0]};
    return true;
}

static bool Pair(SotnSound *sound, Uint32 voice, Uint8 bank, Uint8 program, Uint8 tone, Uint8 note,
                 Uint8 volume, Sint8 pan)
{
    if (tone > 14 || pan < -8 || pan > 8)
        return SDL_SetError("Stereo sound requires bounded VAB tones and pan distance");
    Uint8 key_pan = 64;
    if (pan != 0)
    {
        SotnDataView coefficients = {0};
        if (!View(sound->dra, 0x1d17c + (size_t)(pan + 8) * 4, 4, &coefficients))
            return false;
        const Uint16 right_coefficient = U16(coefficients.bytes),
                     left_coefficient = U16(coefficients.bytes + 2);
        if (left_coefficient > 127 || right_coefficient > 127)
            return SDL_SetError("Private sound pan coefficient exceeds its original range");
        const Uint16 left = (Uint16)((volume * left_coefficient) >> 7),
                     right = (Uint16)((volume * right_coefficient) >> 7);
        key_pan = left == right  ? 64
                  : right < left ? (Uint8)(right * 64 / left)
                                 : (Uint8)(127 - left * 64 / right);
        volume = (Uint8)SDL_max(left, right);
    }
    return SoundKey(sound, voice, bank, program, tone, note, volume, key_pan) &&
           SoundKey(sound, voice + 1, bank, program, (Uint8)(tone + 1), note, volume, key_pan);
}

static bool SetAmbienceVolume(SotnSound *sound, Uint16 multiplier, Sint8 pan)
{
    SotnDataView definition = {0}, coefficients = {0};
    const Uint16 index = sound->ambience_id ? (Uint16)(sound->ambience_id - 0x600U) : 0;
    if (!View(sound->dra, 0x1f554 + (size_t)index * 7, 7, &definition) ||
        !View(sound->dra, 0x1d17c + (size_t)(pan + 8) * 4, 4, &coefficients))
        return false;
    const Uint16 right = U16(coefficients.bytes), left = U16(coefficients.bytes + 2);
    if (right > 127 || left > 127 || definition.bytes[3] > 127)
        return SDL_SetError("Ambience volume references invalid original coefficients");
    sound->ambience_volume22 = multiplier;
    sound->ambience_pan22 = pan;
    const Uint32 volume = (((127U * definition.bytes[3]) >> 7) * multiplier) >> 7;
    for (Uint32 voice = 22; voice <= 23; ++voice)
    {
        // Original SsUtSetVVol overwrites register gain, without retriggering ADSR/decoding.
        sound->registers[voice].left = (Sint32)(((volume * left) >> 8) * 129U);
        sound->registers[voice].right = (Sint32)(((volume * right) >> 8) * 129U);
    }
    return true;
}

static bool AmbienceVolume(SotnSound *sound, Uint16 multiplier, Sint8 pan)
{
    return !sound->ambience_id || SetAmbienceVolume(sound, multiplier, pan);
}

bool SoundAmbienceControl(SotnSound *sound, Uint16 command)
{
    if (command == 1)
        return SetAmbienceVolume(sound, sound->ambience_volume22, (Sint8)sound->ambience_pan22);
    if (command == 0xa0)
    {
        ReleaseAll(sound, 14);
        return true;
    }
    const bool high = command == 0xa1 || command == 0xa5;
    const bool second = command >= 0xa5;
    Uint16 *id = second ? &sound->ambience_id : &sound->ambience_id20;
    if (high || command == 0xa2 || command == 0xa6)
    {
        SoundReleaseVoices(sound, second ? 0xc00000 : 0x300000, high ? 14 : 8);
        *id = 0;
        return true;
    }
    if (!*id)
        return true;
    if (command == 0xa3 || command == 0xa7)
    {
        SoundReleaseVoices(sound, second ? 0xc00000 : 0x300000, 8);
        return true;
    }
    SotnDataView definition = {0};
    if (!View(sound->dra, 0x1f554 + (size_t)(*id - 0x600U) * 7, 7, &definition))
        return false;
    const Uint8 *data = definition.bytes;
    const Uint16 multiplier = second ? sound->ambience_volume22 : sound->ambience_volume20;
    const Sint8 pan = (Sint8)(second ? sound->ambience_pan22 : sound->ambience_pan20);
    const Uint8 volume = (Uint8)((((127U * data[3]) >> 7) * multiplier) >> 7);
    return Pair(sound, second ? 22 : 20, data[0], data[1], data[5], data[2], volume, pan) &&
           (!second || SetAmbienceVolume(sound, multiplier, pan));
}

static bool ScriptPair(SotnSound *sound, Uint32 first, Uint8 bank, Uint8 program, Uint8 tone,
                       Uint8 note, Uint8 volume, Sint8 pan)
{
    Uint32 voice = sound->script_voice_cursor;
    for (Uint32 i = first; i < first + 4; i += 2)
        if (!sound->key_status[i])
        {
            voice = i;
            sound->script_voice_cursor = (Uint16)(i == first + 2 ? first : i + 2);
            break;
        }
    // KeyOnRange deliberately retains one shared fallback across all four ranges.
    return Pair(sound, voice, bank, program, tone, note, volume, pan);
}

static bool Event(SotnSound *sound, Uint16 id, Uint16 multiplier, Sint8 pan)
{
    SotnDataView definition = {0};
    if (!View(sound->dra, 0x1f554 + (size_t)(id - 0x600) * 7, 7, &definition))
        return false;
    const Uint8 *data = definition.bytes;
    Uint8 volume = (Uint8)((127U * data[3]) >> 7);
    if (multiplier != SDL_MAX_UINT16)
        volume = (Uint8)((volume * multiplier) >> 7);
    else if (data[0] != 9 && (data[4] == 1 || data[4] == 3))
        volume = (Uint8)((volume * 127U) >> 7);
    if (data[3] > 127)
        return SDL_SetError("Sound event has an invalid original volume");
    if (data[0] == 9)
    {
        SotnDataView pointer = {0};
        if (data[1] >= 171 || !View(sound->dra, 0x21ecc + ((size_t)data[1] + 1) * 4, 4, &pointer))
            return SDL_SetError("Sound event has an invalid script index");
        if (data[4] > 5)
            return SDL_SetError("Sound script has an unsupported original mode");
        Uint32 selected = 4;
        bool replace = false;
        if (sound->sequence_open)
        {
            if (data[6] >= sound->scripts[3].priority)
            {
                selected = 3;
                replace = true;
            }
        }
        else
        {
            for (Uint32 i = 0; i < 3; ++i)
                if (sound->scripts[i].allocation_id == id)
                {
                    selected = i;
                    replace = true;
                    break;
                }
            if (selected == 4)
                for (Uint32 i = 0; i < 3; ++i)
                    if (!sound->scripts[i].allocation_id)
                    {
                        selected = i;
                        break;
                    }
            for (Uint16 pass = 0; selected == 4 && pass < 2; ++pass)
                for (Uint32 i = 0; i < 3; ++i)
                    if (pass ? data[6] == sound->scripts[i].priority
                             : data[6] > sound->scripts[i].priority)
                    {
                        selected = i;
                        replace = true;
                        break;
                    }
        }
        if (selected < 4)
        {
            // Original group3 releases12..15 even though its playback range is14..17.
            const Uint32 first_voice = selected * 4;
            if (replace)
                for (Uint32 voice = first_voice; voice < first_voice + 4; ++voice)
                    sound->voices[voice].key_off = 1;
            sound->scripts[selected] = (SoundScript){.address = U32(pointer.bytes),
                                                     .id = id,
                                                     .allocation_id = id,
                                                     .mode = data[4],
                                                     .volume = volume,
                                                     .priority = data[6],
                                                     .pan = pan};
        }
        return true;
    }
    Uint32 voice = 22;
    if (data[4] == 3)
    {
        voice = 20;
        sound->ambience_id20 = id;
        sound->ambience_volume20 = multiplier == SDL_MAX_UINT16 ? 127 : multiplier;
        sound->ambience_pan20 = pan;
    }
    else if (data[4] == 2)
    {
        SoundReleaseVoices(sound, 0xc00000, 8);
        sound->ambience_id = 0;
        return true;
    }
    else if (data[4] == 0)
    {
        Uint32 slot = 4;
        bool replace = false;
        if (sound->sequence_open)
        {
            if (!sound->effect_ids[3] || data[6] >= sound->priority[3])
            {
                slot = 3;
                replace = sound->effect_ids[3] != 0;
            }
        }
        else
        {
            for (Uint32 i = 0; i < 4; ++i)
                if (sound->effect_ids[i] == id)
                {
                    slot = i;
                    replace = true;
                    break;
                }
            if (slot == 4)
                for (Uint32 i = 0; i < 4; ++i)
                    if (!sound->effect_ids[i])
                    {
                        slot = i;
                        break;
                    }
            // Original equal-priority replacement intentionally excludes group3.
            for (Uint16 pass = 0; slot == 4 && pass < 2; ++pass)
                for (Uint32 i = 0; i < (pass ? 3U : 4U); ++i)
                    if (pass ? data[6] == sound->priority[i] : data[6] > sound->priority[i])
                    {
                        slot = i;
                        replace = true;
                        break;
                    }
        }
        if (slot == 4)
            return true;
        sound->effect_ids[slot] = id;
        sound->priority[slot] = data[6];
        voice = 12 + slot * 2;
        if (replace)
            sound->voices[voice].key_off = sound->voices[voice + 1].key_off = 1;
    }
    else if (data[4] != 1)
        return SDL_SetError("Sound event requests an unintegrated channel mode");
    if (!Pair(sound, voice, data[0], data[1], data[5], data[2], volume, pan))
        return false;
    if (voice == 22)
    {
        sound->ambience_id = id;
        return AmbienceVolume(sound, multiplier == SDL_MAX_UINT16 ? 127 : multiplier, pan);
    }
    return true;
}

static void Reclaim(SotnSound *sound)
{
    // Original SpuGetAllKeysStatus: the selected SDK software key mask starts
    // at zero and this owner only clears it. SDK note-on flushes write hardware
    // directly; a nonzero actual envelope word therefore reports status2.
    for (Uint32 i = 0; i < 24; ++i)
        sound->key_status[i] = sound->registers[i].volumex ? 2 : 0;
    const Uint32 first_effect = sound->sequence_open ? 3 : 0;
    for (Uint32 i = first_effect; i < 4; ++i)
        if (!sound->registers[12 + i * 2].volumex && !sound->registers[13 + i * 2].volumex)
        {
            sound->effect_ids[i] = 0;
            sound->priority[i] = 0;
        }
    const Uint32 first_script = sound->sequence_open ? 3 : 0,
                 end_script = sound->sequence_open ? 4 : 3;
    for (Uint32 i = first_script; i < end_script; ++i)
    {
        if (sound->scripts[i].id)
            continue;
        const Uint32 first = i == 3 ? 14 : i * 4;
        bool silent = true;
        for (Uint32 voice = first; voice < first + 4; ++voice)
            silent = silent && !sound->registers[voice].volumex;
        if (silent)
        {
            sound->scripts[i].allocation_id = 0;
            sound->scripts[i].priority = 0;
        }
    }
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
    if (sound == NULL || ((id <= 0x600 || id > 0x8e0) && id != 0xa1 && id != 0xe && id != 0xf))
        return SDL_SetError("Sound event is invalid");
    const Uint8 slot = sound->queue_write++;
    sound->queued = (Uint8)(sound->queue_write - sound->queue_read);
    sound->queue[slot].id = id;
    sound->queue[slot].volume = SDL_MAX_UINT16;
    sound->queue[slot].pan = 0;
    sound->queue[slot].ambience_volume = false;
    return true;
}

bool SotnSound_QueueVolumePan(SotnSound *sound, Uint16 id, Uint16 volume, Sint8 pan)
{
    if (id == 0xa1 || id == 0xe || id == 0xf || volume > 127 || pan < -8 || pan > 8)
        return SDL_SetError("Sound volume/pan event exceeds its original bounds");
    if (!SotnSound_Queue(sound, id))
        return false;
    const Uint8 slot = (Uint8)(sound->queue_write - 1);
    sound->queue[slot].volume = volume;
    sound->queue[slot].pan = pan;
    return true;
}

bool SotnSound_QueueAmbienceVolume(SotnSound *sound, Uint16 volume, Sint8 pan)
{
    if (sound == NULL || volume > 127 || pan < -8 || pan > 8)
        return SDL_SetError("Ambience gain exceeds its original bounds");
    const Uint8 slot = sound->queue_write++;
    sound->queued = (Uint8)(sound->queue_write - sound->queue_read);
    sound->queue[slot].id = 0;
    sound->queue[slot].ambience_volume = true;
    sound->queue[slot].volume = volume;
    sound->queue[slot].pan = pan;
    return true;
}

bool SotnSound_UpdateCommands(SotnSound *sound, SotnSoundFrame *out)
{
    if (!sound || !out)
        return SDL_SetError("Sound command update requires its actual owner and frame output");
    Reclaim(sound);
    while (sound->queue_read != sound->queue_write)
    {
        const Uint8 i = sound->queue_read;
        const Uint16 id = sound->queue[i].id;
        sound->queue[i].id = 0;
        bool duplicate = false;
        // ExecSfx clears the current ID before looking ahead. Only the last
        // matching effect runs, with its own volume/pan and relative order.
        if (id > 0x600)
            for (Uint8 next = (Uint8)(i + 1); next != sound->queue_write; ++next)
                if (!sound->queue[next].ambience_volume && sound->queue[next].id == id)
                {
                    duplicate = true;
                    break;
                }
        if (duplicate)
        {
            ++sound->queue_read;
            sound->queued = (Uint8)(sound->queue_write - sound->queue_read);
            continue;
        }
        if (sound->queue[i].ambience_volume)
        {
            if (!AmbienceVolume(sound, sound->queue[i].volume, sound->queue[i].pan))
                return false;
        }
        else if (id == 0xe)
            SoundPauseScripts(sound);
        else if (id == 0xf)
            SoundResumeScripts(sound);
        else if (id == 0xa1)
            SoundReleaseAmbience20(sound);
        else if (!Event(sound, id, sound->queue[i].volume, sound->queue[i].pan))
            return false;
        ++sound->queue_read;
        sound->queued = (Uint8)(sound->queue_write - sound->queue_read);
    }
    const Uint32 first_script = sound->sequence_open ? 3 : 0,
                 end_script = sound->sequence_open ? 4 : 3;
    for (Uint32 i = first_script; i < end_script; ++i)
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
                if (!ScriptPair(sound, i == 3 ? 14 : i * 4, entry.bytes[0], entry.bytes[1],
                                entry.bytes[3], entry.bytes[2],
                                (Uint8)((script->volume * entry.bytes[4]) >> 7), script->pan))
                    return false;
                script->timer = entry.bytes[5];
                script->address += 6;
            }
        }
    }
    if (!SoundSequenceTick(sound))
        return false;
    *out = (SotnSoundFrame){sound, (Uint8)first_script, (Uint8)end_script};
    return true;
}

bool SotnSound_RenderFrame(SotnSound *sound, SotnSoundFrame frame, Sint16 *pcm, size_t count)
{
    if (!sound || frame.owner != sound || !pcm || count < SOTN_SOUND_FRAMES * 2 ||
        !((frame.first_script == 0 && frame.end_script == 3) ||
          (frame.first_script == 3 && frame.end_script == 4)))
        return SDL_SetError("Sound render requires its actual command frame and full PCM output");
    for (Uint32 sample_frame = 0; sample_frame < SOTN_SOUND_FRAMES; ++sample_frame)
    {
        Sint32 left = 0, right = 0, wet_left = 0, wet_right = 0;
        for (Uint32 v = 0; v < 24; ++v)
        {
            SotnVoice *voice = &sound->voices[v];
            if (!voice->active)
                continue;
            Sint32 sample = spu_voice_step(voice, sound->registers[v].pitch, sound->ram);
            spu_voice_envelope_step(voice);
            if (voice->env_state == ADSR_OFF)
                sample = 0;
            sound->registers[v].volumex = (Uint16)voice->env_vol;
            sample = (sample * voice->env_vol) >> 15;
            const Sint32 voice_left = (sample * (sound->registers[v].left * 2)) >> 15;
            const Sint32 voice_right = (sample * (sound->registers[v].right * 2)) >> 15;
            left += voice_left;
            right += voice_right;
            if (sound->reverb_voices & (1U << v))
            {
                wet_left += voice_left;
                wet_right += voice_right;
            }
        }
        if (sound->reverb.configured)
        {
            Sint32 wet[2];
            SoundReverb_Tick(&sound->reverb, sound->ram, clamp16(wet_left), clamp16(wet_right),
                             wet);
            left += wet[0];
            right += wet[1];
        }
        pcm[sample_frame * 2] = clamp16(left);
        pcm[sample_frame * 2 + 1] = clamp16(right);
    }
    return true;
}

bool SotnSound_Tick(SotnSound *sound, Sint16 *pcm, size_t count)
{
    if (!sound || !pcm || count < SOTN_SOUND_FRAMES * 2)
        return SDL_SetError("Sound tick requires state and a full stereo PCM buffer");
    SotnSoundFrame frame = {0};
    return SotnSound_UpdateCommands(sound, &frame) &&
           SotnSound_RenderFrame(sound, frame, pcm, count);
}

void SotnSound_Destroy(SotnSound *sound)
{
    SDL_free(sound);
}
