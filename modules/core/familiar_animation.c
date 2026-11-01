// SPDX-License-Identifier: AGPL-3.0-or-later
// servant/servant_update_anim.h and tt_002/faerie.c:SetAnimationFrame,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Timelines remain private.
#include <SDL3/SDL.h>
#include <sotn/familiar_animation.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static bool View(SotnDataView overlay, Uint64 address, size_t size, const Uint8 **out)
{
    if (!overlay.bytes || address < 0x80170000U || address > SDL_MAX_UINT32 ||
        address - 0x80170000U > overlay.size || size > overlay.size - (address - 0x80170000U))
        return SDL_SetError("Familiar animation references truncated private timeline data");
    *out = overlay.bytes + (size_t)(address - 0x80170000U);
    return true;
}
static bool Timeline(SotnFamiliarAnimationSource source, Uint16 index, Uint32 *address)
{
    const Uint8 *pointer = NULL, *frame = NULL;
    if (!source.count || source.count > 512 || index >= source.count ||
        !View(source.overlay, source.table, (size_t)source.count * 4U, &pointer))
        return SDL_SetError("Familiar animation requires a bounded original pointer table");
    const Uint32 next = Read32(pointer + index * 4U);
    if (!View(source.overlay, next, 4, &frame))
        return false;
    *address = next;
    return true;
}
bool SotnFamiliarAnimation_Set(SotnFamiliarAnimation *animation, SotnFamiliarAnimationSource source,
                               Uint16 index, bool restart)
{
    Uint32 address = 0;
    if (!animation || !Timeline(source, index, &address))
        return SDL_SetError("Familiar animation selection lost its actual cursor or timeline");
    if (restart || animation->address != address)
    {
        animation->address = address;
        animation->pose = 0;
        animation->remaining = 0;
    }
    return true;
}
bool SotnFamiliarAnimation_Tick(SotnFamiliarAnimation *animation,
                                SotnFamiliarAnimationSource source, Sint32 *status)
{
    if (!animation || (source.properties.size && !source.properties.bytes))
        return SDL_SetError("Familiar animation requires an actual cursor and frame properties");
    SotnFamiliarAnimation next = *animation;
    Sint32 result = 0;
    const Uint8 *frame = NULL;
    if (!View(source.overlay, (Uint64)next.address + next.pose * 4U, 4, &frame))
        return false;
    if (next.remaining == -1)
        result = -1;
    else if (next.remaining == 0)
        next.remaining = Word(Read16(frame));
    else
    {
        next.remaining = Word(next.remaining - 1);
        if (!next.remaining)
        {
            ++next.pose;
            if (!View(source.overlay, (Uint64)next.address + next.pose * 4U, 4, &frame))
                return false;
            const Uint16 duration = Read16(frame), value = Read16(frame + 2);
            if (!duration)
            {
                next.pose = value;
                if (!View(source.overlay, (Uint64)next.address + next.pose * 4U, 4, &frame))
                    return false;
                next.remaining = Word(Read16(frame));
            }
            else if (duration == 0xffff)
            {
                --next.pose;
                next.remaining = -1;
                result = -1;
            }
            else if (duration == 0xfffe)
            {
                if (!Timeline(source, value, &next.address))
                    return false;
                next.pose = 0;
                if (!View(source.overlay, next.address, 4, &frame))
                    return false;
                next.remaining = Word(Read16(frame));
                result = -2;
            }
            else
                next.remaining = Word(duration);
        }
    }
    if (!View(source.overlay, (Uint64)next.address + next.pose * 4U, 4, &frame))
        return false;
    const Uint16 packed = Read16(frame + 2);
    if (source.properties.bytes)
    {
        const size_t offset = ((packed >> 9) & 127U) * 4U;
        if (offset > source.properties.size || source.properties.size - offset < 4)
            return SDL_SetError("Familiar animation lost its private frame property record");
        const Uint8 *properties = source.properties.bytes + offset;
        next.hitbox_x = (Sint8)(properties[0] < 128 ? properties[0] : properties[0] - 256);
        next.hitbox_y = (Sint8)(properties[1] < 128 ? properties[1] : properties[1] - 256);
        next.hitbox_width = properties[2];
        next.hitbox_height = properties[3];
    }
    next.sprite = packed & 511U;
    *animation = next;
    if (status)
        *status = result;
    return true;
}
