// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected EntityPlayerSleepZ/UpdateAnim, dra/7E4BC.c/alu_anim.c, sotn-decomp 7bc54d8157b6.
// Animation records stay in private DRA. One slot suffices for the 512-tick factory interval.
#include "player_internal.h"
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnPlayer_SleepTick(SotnPlayer *p)
{
    SotnPlayerSleep *z = &p->sleep;
    if (p->step != SOTN_PLAYER_STAND || p->substep != 4)
    {
        *z = (SotnPlayerSleep){0};
        return true;
    }
    if (p->sleep_event)
    {
        if (z->active)
            return SDL_SetError("Player sleep factory overlaps its bounded lifetime");
        const Sint32 vx = p->facing_left ? -16384 : 16384;
        const Sint64 x = (Sint64)p->x.value + vx * 32, y = (Sint64)p->y.value - 16 * 65536;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
            return SDL_SetError("Player sleep factory exceeds fixed-point coordinates");
        *z = (SotnPlayerSleep){.x = (Sint32)x, .y = (Sint32)y, .velocity_x = vx, .active = true};
    }
    else if (z->active)
    {
        if (z->remaining < 0)
        {
            *z = (SotnPlayerSleep){0};
            return true;
        }
        const Sint64 x = (Sint64)z->x + z->velocity_x, y = (Sint64)z->y - 24576;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
            return SDL_SetError("Player sleep letter exceeds fixed-point motion bounds");
        z->x = (Sint32)x;
        z->y = (Sint32)y;
    }
    else
        return true;
    if (z->remaining > 0 && --z->remaining == 0)
        ++z->pose;
    const size_t offset = 0xe294U + z->pose * 4U;
    if (z->pose > 4 || !p->dra.bytes || offset > p->dra.size || 4 > p->dra.size - offset)
        return SDL_SetError("Player sleep animation exceeds its private frame stream");
    const Uint8 *record = p->dra.bytes + offset;
    const Uint16 duration = Read16(record), frame = Read16(record + 2);
    if (duration == 0xffff && z->pose)
    {
        --z->pose;
        z->remaining = -1;
        return true;
    }
    if (!duration || duration > 32767 || !frame || frame > 511)
        return SDL_SetError("Player sleep animation has an invalid duration or sprite");
    if (!z->remaining)
        z->remaining = (Sint16)duration;
    z->frame = frame;
    return true;
}
