// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c walking and shared e_misc.h gray-puff factory.
#include "library_demon_internal.h"
bool ActorDemonMove(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Lesser Demon motion exceeds native fixed-point bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
bool ActorDemonWalkFrame(SotnActors *s, Actor *a, bool *out)
{
    Uint8 mask = 0;
    bool ended = false;
    for (Uint16 i = 0; i < 3; ++i)
    {
        SotnDataView pair = {0};
        if (!Slice(s->nz0, 0x1c3c + i * 4U, 4, &pair))
            return false;
        if (Signed16(pair.bytes) == 255)
        {
            ended = true;
            break;
        }
        SotnCollider c = {0};
        const Sint32 dx = Signed16(pair.bytes);
        if (!ActorQuery(s, Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? dx : -dx)),
                        Wrap16((Sint64)Pixel(a->state.y) + Signed16(pair.bytes + 2)), &c))
            return false;
        mask = (Uint8)((mask << 1) | ((c.effects & SOTN_EFFECT_SOLID) ? 1U : 0U));
    }
    if (!ended)
        return SDL_SetError("Lesser Demon walking sensors lack a bounded sentinel");
    if (!a->remaining)
    {
        SotnDataView delta = {0};
        if (a->pose >= 8 || !Slice(s->nz0, 0x1af4 + a->pose * 2U, 2, &delta))
            return SDL_SetError("Lesser Demon walking pose exceeds its private displacement table");
        const Sint32 dx = Signed16(delta.bytes);
        a->state.x = Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? dx : -dx)) * 65536 +
                     (Sint32)((Uint32)a->state.x & 65535U);
    }
    *out = mask != 2;
    return true;
}
bool ActorDemonPuffs(SotnActors *s, const Actor *a)
{
    // The original gray-puff factory ignores its params/index arguments.
    for (Uint16 i = 0; i < 5; ++i)
    {
        Actor *puff = NULL;
        if (!ActorSpawn(s, a, 0x15, i, 96, 128, &puff))
            return false;
        if (puff)
        {
            const Sint32 x =
                (a->state.facing_left ? -4 : 4) + (Sint32)i * (a->state.facing_left ? 7 : -7);
            puff->state.x = Wrap16((Sint64)Pixel(a->state.x) + x) * 65536;
            puff->state.y = Wrap16((Sint64)Pixel(a->state.y) + 32) * 65536;
            puff->state.facing_left = false;
            puff->state.priority = (Uint16)(a->state.priority + 1);
        }
    }
    return true;
}
