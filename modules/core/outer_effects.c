// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_misc.h effects at the pinned sotn-decomp reference. NO1 tables stay private.
#include "actors_internal.h"
bool ActorOuterDust(SotnActors *s, const Actor *a, Uint16 count, Uint16 kind, Sint16 x, Sint16 y,
                    Uint16 index, Sint16 gap)
{
    if (count > 6 || index + count > 6 || kind > 3 || SDL_abs(x) > 128 || SDL_abs(y) > 128 ||
        SDL_abs(gap) > 64)
        return SDL_SetError("Outer Wall dust recipe exceeds its original bounded variants");
    for (Uint16 i = 0; i < count; ++i)
    {
        Actor *dust = NULL;
        if (!ActorSpawn(s, a, 0x14, kind, 96, 128, &dust))
            return false;
        if (dust == NULL)
            continue;
        SotnDataView size = {0};
        if (!Slice(s->nz0, 0x239c + (index + i) * 2U, 2, &size) || Read16(size.bytes) > 1024 ||
            !Read16(size.bytes))
            return SDL_SetError("Outer Wall dust scale is outside its private bounds");
        dust->state.x = (Pixel(a->state.x) + x + i * gap) * 65536;
        dust->state.y = (Pixel(a->state.y) + y) * 65536;
        dust->state.scale_x = dust->state.scale_y = Read16(size.bytes);
        dust->state.priority = (Uint16)(a->state.priority + 1);
        dust->state.facing_left = false;
        dust->enemy_state.attack_index = (Uint16)(index + i);
    }
    return true;
}
bool ActorOuterEffect(SotnActors *s, Actor *a)
{
    if (a->state.id == 6)
    {
        if (!a->state.step)
        {
            if (!ActorInitialize(s, a, 0x950))
                return false;
            a->state.palette = (a->state.parameters & 0xf0) ? 0x8195 : 0x8170;
            a->state.animation_set = 5;
            a->state.frame = 1;
            a->state.blend_mode = (a->state.parameters & 0xf0) ? 0x10 : 0x30;
            if (a->state.parameters & 0xff00)
                a->state.priority = (Uint16)(a->state.parameters >> 8);
            a->state.priority = (Uint16)(a->state.priority + 8);
            return true;
        }
        a->state.y -= 16384;
        if ((++a->timer % 2) == 0)
            ++a->state.frame;
        if (a->timer > 36)
            a->remove = true;
        return true;
    }
    const bool gray = a->state.id == 0x15;
    const Uint16 index = gray ? a->state.parameters : a->enemy_state.attack_index;
    if (index >= 6 || (!gray && a->state.parameters > 3))
        return SDL_SetError("Outer Wall particle type or velocity index exceeds its private table");
    if (!a->state.step)
    {
        SotnDataView velocity = {0};
        if (!Slice(s->nz0, (gray ? 0x23bc : 0x23d4) + index * 4U, 4, &velocity))
            return false;
        a->vy = Signed32(velocity.bytes);
        if (a->vy < -2 * 65536 || a->vy > 2 * 65536)
            return SDL_SetError("Outer Wall particle velocity exceeds its private bounds");
        a->state.flags = 0x0c002000;
        a->state.palette = 0x8195;
        a->state.animation_set = gray ? 5 : 2;
        a->state.blend_mode = 0x10;
        if (gray)
        {
            SotnDataView size = {0};
            if (!Slice(s->nz0, 0x23ac + index * 2U, 2, &size) || !Read16(size.bytes) ||
                Read16(size.bytes) > 1024)
                return SDL_SetError("Outer Wall gray puff scale exceeds its private bounds");
            a->state.scale_x = a->state.scale_y = Read16(size.bytes);
            a->state.frame = 1;
        }
        else
        {
            SotnDataView frame = {0};
            if (!Slice(s->nz0, 0x23ec + a->state.parameters, 1, &frame))
                return false;
            a->state.frame = frame.bytes[0];
        }
        ++a->state.step;
        return true;
    }
    a->state.y -= a->vy;
    if ((++a->timer % 2) == 0)
        ++a->state.frame;
    Uint16 lifetime = 36;
    if (!gray)
    {
        SotnDataView time = {0};
        if (!Slice(s->nz0, 0x23f0 + a->state.parameters * 2U, 2, &time))
            return false;
        lifetime = Read16(time.bytes);
        if (!lifetime || lifetime > 128)
            return SDL_SetError("Outer Wall dust lifetime exceeds its private bounds");
    }
    if (a->timer > lifetime)
        a->remove = true;
    return true;
}
