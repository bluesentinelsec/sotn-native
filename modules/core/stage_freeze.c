// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/st_update.h:UpdateStageEntities, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Palette data remains private.
#include <SDL3/SDL_error.h>
#include <sotn/stage_freeze.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return (Uint32)Read16(p) | (Uint32)Read16(p + 2) << 16;
}
bool SotnStageFreeze_Palettes(SotnDataView stage, SotnDataView *out)
{
    if (!out || !stage.bytes || stage.size < 44)
        return SDL_SetError("Stage freeze requires its private US overlay export");
    const Uint32 address = Read32(stage.bytes + 40);
    if (address < 0x80180000U || address % 4)
        return SDL_SetError("Stage freeze export has an invalid original address");
    const size_t at = address - 0x80180000U;
    if (at > stage.size || stage.size - at < 52)
        return SDL_SetError("Stage freeze export exceeds its private overlay");
    const Uint32 high = Read32(stage.bytes + at + 44), low = Read32(stage.bytes + at + 48);
    if ((high & 0xffff0000U) != 0x3c120000U || (low & 0xffff0000U) != 0x26520000U)
        return SDL_SetError("Stage freeze export has an unsupported private US palette reference");
    const Uint16 bits = (Uint16)low;
    const Sint64 pointer =
        ((Sint64)(high & 65535U) << 16) + (bits < 32768U ? bits : (Sint32)bits - 65536);
    if (pointer < 0x80180000LL || pointer % 2)
        return SDL_SetError("Stage freeze palette reference has an invalid original address");
    const Uint64 offset = (Uint64)(pointer - 0x80180000LL);
    if (offset > stage.size || stage.size - (size_t)offset < 44)
        return SDL_SetError("Stage freeze palette table exceeds its private overlay");
    *out = (SotnDataView){stage.bytes + (size_t)offset, 44};
    return true;
}
bool SotnStageFreeze_Begin(SotnStageFreezeActor *actor, SotnDataView palettes, bool *out)
{
    if (!actor || !out)
        return SDL_SetError("Stage freeze requires actual copied actor fields and output");
    if (!actor->step || !(actor->flags & 0x10000U))
    {
        *out = !actor->step;
        return true;
    }
    SotnStageFreezeActor next = *actor;
    if (next.flags & 15U)
    {
        const Uint16 index = (Uint16)((Uint16)next.palette_effect * 2U + (next.flags & 1U));
        if (index >= 22 || !palettes.bytes || palettes.size != 44)
            return SDL_SetError("Frozen actor hit palette exceeds its private22-word table");
        next.palette = Read16(palettes.bytes + index * 2U);
        if (!(--next.flags & 15U))
        {
            next.palette = next.hit_palette;
            next.hit_palette = 0;
        }
    }
    *actor = next;
    *out = true;
    return true;
}
