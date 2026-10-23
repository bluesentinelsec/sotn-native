// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/alu_anim.c:SetPlayerAnim, func_8010DA70, and PlayAnimation.
#include <SDL3/SDL_error.h>
#include <sotn/animation.h>

#define ANIMATION_TABLE 0x101b8U
#define SUBANIMATION_TABLE 0x10594U
#define FRAME_PROPERTIES 0x10130U
#define MAX_POSES 4096U

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static bool Resolve(SotnDataView dra, Uint32 address, size_t size, SotnDataView *out)
{
    if (dra.bytes == NULL || address < 0x800a0000 || (address & 1) != 0)
        return SDL_SetError("Animation references an invalid DRA address");
    const size_t offset = address - 0x800a0000;
    if (offset > dra.size || size > dra.size - offset)
        return SDL_SetError("Animation references bytes outside DRA");
    *out = (SotnDataView){dra.bytes + offset, size};
    return true;
}

static bool TablePointer(SotnDataView dra, Uint32 offset, Uint16 index, Uint16 count,
                         Uint32 *address)
{
    SotnDataView entry = {0};
    if (index >= count || !Resolve(dra, 0x800a0000 + offset + (Uint32)index * 4, 4, &entry))
        return SDL_SetError("Animation pointer-table index is invalid or truncated");
    *address = entry.bytes[0] | (Uint32)entry.bytes[1] << 8 | (Uint32)entry.bytes[2] << 16 |
               (Uint32)entry.bytes[3] << 24;
    return true;
}

static bool Frame(SotnDataView dra, Uint16 id, Uint16 pose, Uint16 *duration, Uint16 *packed)
{
    Uint32 address = 0;
    if (!TablePointer(dra, ANIMATION_TABLE, id, 247, &address))
        return false;
    Uint32 flat_index = 0;
    Uint32 visits = 0;
    for (Uint32 i = 0; visits < MAX_POSES; ++i, ++visits)
    {
        SotnDataView entry = {0};
        if (i > (SDL_MAX_UINT32 - address) / 4)
            return SDL_SetError("Animation address arithmetic overflow");
        if (!Resolve(dra, address + i * 4, 4, &entry))
            return false;
        const Uint16 time = Read16(entry.bytes);
        const Uint16 value = Read16(entry.bytes + 2);
        if (time == 0xfffd)
        {
            Uint32 subaddress = 0;
            if (!TablePointer(dra, SUBANIMATION_TABLE, value & 255, 29, &subaddress))
                return false;
            for (Uint32 j = 0; visits < MAX_POSES; ++j, ++visits)
            {
                SotnDataView sub = {0};
                if (j > (SDL_MAX_UINT32 - subaddress) / 4)
                    return SDL_SetError("Subanimation address arithmetic overflow");
                if (!Resolve(dra, subaddress + j * 4, 4, &sub))
                    return false;
                if (Read16(sub.bytes) == 0xffff)
                    break;
                if (flat_index++ == pose)
                {
                    *duration = Read16(sub.bytes);
                    *packed = Read16(sub.bytes + 2);
                    return true;
                }
            }
        }
        else if (flat_index++ == pose)
        {
            *duration = time;
            *packed = value;
            return true;
        }
        else if (time >= 0xfffe || time == 0)
            return SDL_SetError("Animation pose extends past its control record");
    }
    return SDL_SetError("Animation traversal exceeded the bounded pose count");
}

bool SotnAnimation_Set(SotnAnimation *animation, Uint16 id)
{
    if (animation == NULL || id >= 247)
        return SDL_SetError("Alucard animation ID must be less than 247");
    animation->id = id;
    animation->pose = 0;
    animation->remaining = 0;
    return true;
}

bool SotnAnimation_Tick(SotnDataView dra, SotnAnimation *animation)
{
    if (animation == NULL || animation->id >= 247 || animation->pose >= MAX_POSES ||
        animation->remaining < -1 || animation->remaining > 32767)
        return SDL_SetError("Invalid Alucard animation cursor");
    SotnAnimation next = *animation;
    Uint16 duration = 0, packed = 0;
    if (!Frame(dra, next.id, next.pose, &duration, &packed))
        return false;
    if (next.remaining == 0)
        next.remaining = duration;
    else if (next.remaining > 0 && --next.remaining == 0)
    {
        ++next.pose;
        if (!Frame(dra, next.id, next.pose, &duration, &packed))
            return false;
        if (duration == 0)
        {
            next.pose = packed;
            if (!Frame(dra, next.id, next.pose, &duration, &packed))
                return false;
        }
        else if (duration == 0xffff)
        {
            --next.pose;
            next.remaining = -1;
            if (!Frame(dra, next.id, next.pose, &duration, &packed))
                return false;
        }
        else if (duration == 0xfffe)
        {
            next.id = packed & 255;
            next.pose = packed >> 8;
            if (!Frame(dra, next.id, next.pose, &duration, &packed))
                return false;
        }
        if (next.remaining != -1)
            next.remaining = duration;
    }
    if (duration == 0 || duration > 32767)
        return SDL_SetError("Animation resolved to an invalid frame duration");
    const Uint16 property = (packed >> 9) & 127;
    SotnDataView box = {0};
    if (property >= 34 || !Resolve(dra, 0x800a0000 + FRAME_PROPERTIES + property * 4U, 4, &box))
        return SDL_SetError("Animation hitbox property is invalid or truncated");
    next.hitbox_x = (Sint16)(box.bytes[0] < 128 ? box.bytes[0] : (Sint32)box.bytes[0] - 256);
    next.hitbox_y = (Sint16)(box.bytes[1] < 128 ? box.bytes[1] : (Sint32)box.bytes[1] - 256);
    next.hitbox_width = box.bytes[2];
    next.hitbox_height = box.bytes[3];
    next.sprite = packed & 511;
    *animation = next;
    return true;
}
