// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_misc.h effects at the pinned sotn-decomp reference. NO1/LIB/DAI/NO2 tables stay
// private.
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
        if (!Slice(s->nz0,
                   (s->stage_id == 6   ? 0x19c8
                    : s->stage_id == 4 ? 0x1980
                    : s->stage_id == 2 ? 0x230c
                                       : 0x239c) +
                       (index + i) * 2U,
                   2, &size) ||
            Read16(size.bytes) > 1024 || !Read16(size.bytes))
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
        return ActorIntenseExplosion(s, a);
    if (a->state.id == 0x23)
    {
        if (!a->state.step)
        {
            if (!ActorInitialize(s, a, 0x950))
                return false;
            a->state.priority = (Uint16)(a->state.priority + 16);
            a->state.animation_set = 8;
            a->state.frame = 1;
            a->state.palette = a->state.parameters ? a->state.parameters : 0x8160;
            // The original assigns scale/opacity without enabling their draw flags.
            ++a->state.step;
        }
        else
        {
            Uint8 animation = 0;
            a->state.x += a->vx;
            a->state.y += a->vy;
            if (!ActorAnimate(s, a, 0x2400, &animation))
                return false;
            a->remove = animation == 0;
        }
        return true;
    }
    if (a->state.id != 0x14 && a->state.id != 0x15)
        return SDL_SetError("Unsupported Outer Wall particle identity");
    const bool gray = a->state.id == 0x15;
    const Uint16 index = gray ? a->state.parameters : a->enemy_state.attack_index;
    if (index >= 6 || (!gray && a->state.parameters > 3))
        return SDL_SetError("Outer Wall particle type or velocity index exceeds its private table");
    if (!a->state.step)
    {
        SotnDataView velocity = {0};
        if (!Slice(s->nz0,
                   (s->stage_id == 6   ? (gray ? 0x19e8 : 0x1a00)
                    : s->stage_id == 4 ? (gray ? 0x19a0 : 0x19b8)
                    : s->stage_id == 2 ? (gray ? 0x232c : 0x2344)
                                       : (gray ? 0x23bc : 0x23d4)) +
                       index * 4U,
                   4, &velocity))
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
            if (!Slice(s->nz0,
                       (s->stage_id == 6   ? 0x19d8
                        : s->stage_id == 4 ? 0x1990
                        : s->stage_id == 2 ? 0x231c
                                           : 0x23ac) +
                           index * 2U,
                       2, &size) ||
                !Read16(size.bytes) || Read16(size.bytes) > 1024)
                return SDL_SetError("Outer Wall gray puff scale exceeds its private bounds");
            a->state.scale_x = a->state.scale_y = Read16(size.bytes);
            a->state.frame = 1;
        }
        else
        {
            SotnDataView frame = {0};
            if (!Slice(s->nz0,
                       (s->stage_id == 6   ? 0x1a18
                        : s->stage_id == 4 ? 0x19d0
                        : s->stage_id == 2 ? 0x235c
                                           : 0x23ec) +
                           a->state.parameters,
                       1, &frame))
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
        if (!Slice(s->nz0,
                   (s->stage_id == 6   ? 0x1a1c
                    : s->stage_id == 4 ? 0x19d4
                    : s->stage_id == 2 ? 0x2360
                                       : 0x23f0) +
                       a->state.parameters * 2U,
                   2, &time))
            return false;
        lifetime = Read16(time.bytes);
        if (!lifetime || lifetime > 128)
            return SDL_SetError("Outer Wall dust lifetime exceeds its private bounds");
    }
    if (a->timer > lifetime)
        a->remove = true;
    return true;
}
