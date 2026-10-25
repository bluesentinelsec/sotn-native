// SPDX-License-Identifier: MIT
// Selected reconstructed SDK SEQ scheduling/parsing from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, main/psxsdk/libsnd/seqinit.c, seqread.c.
// No sequence bytes, instrument definitions or lookup tables are embedded.
#include <SDL3/SDL.h>
#include <sotn/sequence.h>
static bool Byte(SotnSequence *s, Uint8 *out)
{
    if (s->cursor >= s->data.size)
        return SDL_SetError("SEQ event is truncated at byte %zu", s->cursor);
    *out = s->data.bytes[s->cursor++];
    return true;
}
static bool Delta(SotnSequence *s)
{
    Uint32 value = 0;
    for (Uint16 i = 0; i < 4; ++i)
    {
        Uint8 byte = 0;
        if (!Byte(s, &byte))
            return false;
        value = value * 128U + (byte & 127U);
        if (!(byte & 128U))
        {
            if (value > SDL_MAX_SINT32 / 10U)
                return SDL_SetError("SEQ delta exceeds the original signed scheduling range");
            s->delta = (Sint32)(value * 10U);
            return true;
        }
    }
    return SDL_SetError("SEQ delta has an unterminated variable-length quantity");
}
static bool Tempo(SotnSequence *s, Uint32 microseconds, bool initialize)
{
    if (microseconds == 0)
        return SDL_SetError("SEQ tempo cannot be zero");
    Uint32 bpm = 60000000U / microseconds;
    if (initialize && microseconds / 2 < 60000000U % microseconds)
        ++bpm;
    if (bpm == 0 || bpm > 1000)
        return SDL_SetError("SEQ tempo exceeds its bounded SDK scheduling range");
    const Uint32 speed = (Uint32)s->resolution * bpm * 10U;
    if (speed < 3600)
    {
        s->step = s->period = (Sint32)(36000U / ((Uint32)s->resolution * bpm));
    }
    else
    {
        s->period = -1;
        s->step = (Sint32)(speed / 3600U + (speed % 3600U > 1800U ? 1U : 0U));
    }
    return true;
}
bool SotnSequence_Open(SotnDataView data, Uint8 bank, Uint16 repeats, SotnSequence *out)
{
    if (!out || !data.bytes || data.size < 16 || data.size > 1024U * 1024U || bank > 3 ||
        SDL_memcmp(data.bytes, "pQES", 4) != 0 || data.bytes[4] || data.bytes[5] || data.bytes[6] ||
        data.bytes[7] != 1)
        return SDL_SetError("SEQ requires bounded version-one private data and a valid VAB bank");
    SotnSequence next = {
        .data = data, .cursor = 15, .bank = bank, .repeats = repeats, .playing = true};
    next.resolution = (Uint16)((Uint16)data.bytes[8] << 8 | data.bytes[9]);
    if (next.resolution == 0 || next.resolution > 32767)
        return SDL_SetError("SEQ resolution is outside the original signed range");
    for (Uint8 i = 0; i < 16; ++i)
    {
        next.program[i] = i;
        next.volume[i] = 127;
        next.pan[i] = 64;
    }
    const Uint32 tempo =
        (Uint32)data.bytes[10] << 16 | (Uint32)data.bytes[11] << 8 | data.bytes[12];
    if (!Tempo(&next, tempo, true) || !Delta(&next))
        return false;
    next.start = next.loop = next.cursor;
    *out = next;
    return true;
}
static bool Emit(SotnSequence *s, SotnSequenceSink sink, void *ctx, SotnSequenceEventKind kind,
                 Uint8 note, Uint8 value)
{
    return sink(ctx, (SotnSequenceEvent){kind, s->channel, s->bank, s->program[s->channel], note,
                                         value, s->volume[s->channel], s->pan[s->channel]});
}
static bool Control(SotnSequence *s, SotnSequenceSink sink, void *ctx, Uint8 code, Uint8 value)
{
    if (!Delta(s))
        return false;
    switch (code)
    {
    case 0:
        if (value > 3)
            return SDL_SetError("SEQ selected an unavailable VAB bank");
        s->bank = value;
        return true;
    case 7:
        s->volume[s->channel] = value;
        return Emit(s, sink, ctx, SOTN_SEQUENCE_VOLUME, 0, value);
    case 10:
        s->pan[s->channel] = value;
        return Emit(s, sink, ctx, SOTN_SEQUENCE_PAN, 0, value);
    case 99:
        s->nrpn = value;
        if (value == 20)
        {
            s->loop_enabled = true;
            s->loop = s->cursor;
        }
        else if (value == 30)
        {
            if (s->loop_count == 0)
                s->loop_count_set = false;
            else if (s->loop_count < 127)
            {
                if (--s->loop_count)
                    s->cursor = s->loop;
                else
                    s->loop_count_set = false;
            }
            else
            {
                s->delta = 0;
                s->cursor = s->loop;
            }
        }
        else
            return SDL_SetError("SEQ uses an unsupported NRPN parameter %u", value);
        return true;
    case 98:
        if (!s->loop_enabled || s->loop_count_set)
            return SDL_SetError("SEQ uses an unsupported non-loop NRPN data entry");
        s->loop_count = value;
        s->loop_count_set = true;
        return true;
    case 6:
        // The original LIB stream sets its repeat count through Data Entry after NRPN 20.
        if (!s->loop_enabled)
            return SDL_SetError("SEQ uses unsupported instrument Data Entry");
        if (!s->loop_count_set)
        {
            s->loop_count = value;
            s->loop_count_set = true;
        }
        return true;
    default:
        return SDL_SetError("SEQ uses an unsupported control %u", code);
    }
}
static bool Event(SotnSequence *s, SotnSequenceSink sink, void *ctx)
{
    Uint8 first = 0, value = 0;
    if (!Byte(s, &first))
        return false;
    if (first & 128U)
    {
        s->channel = first & 15U;
        s->running = first & 240U;
        if (s->running != 0x90 && s->running != 0xb0 && s->running != 0xc0 && s->running != 0xe0 &&
            first != 0xff)
            return SDL_SetError("SEQ uses an unsupported status 0x%02x", first);
        if (first == 0xff)
            s->running = 0xff;
        if (!Byte(s, &first))
            return false;
    }
    else if (!s->running)
        return SDL_SetError("SEQ running data has no preceding status");
    if (first > 127)
        return SDL_SetError("SEQ event data exceeds seven bits");
    if (s->running == 0xff)
    {
        if (first == 0x51)
        {
            Uint8 middle = 0, low = 0;
            if (!Byte(s, &value) || !Byte(s, &middle) || !Byte(s, &low) ||
                !Tempo(s, (Uint32)value << 16 | (Uint32)middle << 8 | low, false))
                return false;
            return Delta(s);
        }
        if (first != 0x2f)
            return SDL_SetError("SEQ uses an unsupported meta event 0x%02x", first);
        ++s->completed;
        if (s->repeats == 0 || s->completed < s->repeats)
        {
            s->cursor = s->start;
            s->loop_enabled = false;
            s->loop_count_set = false;
            s->loop = s->start;
            s->delta = 0;
            return true;
        }
        s->playing = false;
        s->delta = s->step;
        return Emit(s, sink, ctx, SOTN_SEQUENCE_STOP, 0, 0);
    }
    if (s->running == 0xc0)
    {
        s->program[s->channel] = first;
        return Delta(s);
    }
    if (!Byte(s, &value) || value > 127)
        return SDL_SetError("SEQ channel event is truncated or exceeds seven bits");
    if (s->running == 0xb0)
        return Control(s, sink, ctx, first, value);
    if (!Delta(s))
        return false;
    return Emit(s, sink, ctx, s->running == 0x90 ? SOTN_SEQUENCE_NOTE : SOTN_SEQUENCE_BEND,
                s->running == 0x90 ? first : 0, value);
}
bool SotnSequence_Tick(SotnSequence *s, SotnSequenceSink sink, void *ctx)
{
    if (!s || !sink || !s->data.bytes || !s->resolution || s->step <= 0 || s->channel >= 16 ||
        s->cursor > s->data.size || s->start > s->data.size || s->loop > s->data.size)
        return SDL_SetError("SEQ tick requires initialized bounded state and an event receiver");
    if (!s->playing)
        return true;
    if (s->delta > s->step)
    {
        if (s->period > 0)
        {
            --s->period;
        }
        else if (s->period == 0)
        {
            s->period = s->step;
            --s->delta;
        }
        else
            s->delta -= s->step;
        return true;
    }
    Sint64 total = s->delta;
    for (Uint32 budget = 0; budget < 1024; ++budget)
    {
        if (!Event(s, sink, ctx))
            return false;
        if (s->delta == 0)
            continue;
        total += s->delta;
        if (!s->playing || total >= s->step)
        {
            if (total - s->step > SDL_MAX_SINT32)
                return SDL_SetError("SEQ tick exceeds signed delta capacity");
            s->delta = (Sint32)(total - s->step);
            return true;
        }
    }
    return SDL_SetError("SEQ tick exceeds its bounded event budget");
}
