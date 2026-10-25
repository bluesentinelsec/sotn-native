// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_crow.c at sotn-decomp 7bc54d8157b6. Original animation,
// initialization, frame hitboxes and artwork stay in the private overlay.
#include "actors_internal.h"
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel bird motion exceeds bounded fixed-point coordinates");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Velocity(Actor *a, Sint64 x, Sint64 y)
{
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel bird acceleration exceeds bounded fixed-point velocity");
    a->vx = (Sint32)x;
    a->vy = (Sint32)y;
    return true;
}
static bool Hitbox(SotnActors *s, Actor *a)
{
    SotnDataView profile = {0};
    if (a->state.frame > 16 || !Slice(s->nz0, 0x1fa8U + a->state.frame * 8U, 8, &profile))
        return SDL_SetError("Chapel bird frame exceeds its private hitbox profiles");
    const Sint32 x = Signed16(profile.bytes), y = Signed16(profile.bytes + 2),
                 w = Signed16(profile.bytes + 4), h = Signed16(profile.bytes + 6);
    if (x < -128 || x > 127 || y < -128 || y > 127 || w < 0 || w > 127 || h < 0 || h > 127)
        return SDL_SetError("Chapel bird hitbox exceeds original byte-sized extents");
    a->state.hitbox_x = (Sint16)x;
    a->state.hitbox_y = (Sint16)y;
    a->state.hitbox_width = (Uint16)w;
    a->state.hitbox_height = (Uint16)h;
    return true;
}
static bool Crow(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100U) && a->state.step != 4)
    {
        a->state.rotated = true;
        a->vy = 0;
        a->state.hitbox_state = 0;
        if (!ActorPositionalSound(s, a, 0x6d6))
            return false;
        SetStep(a, 4);
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xa70))
            return false;
        a->state.frame = 1;
        a->crow.reference_y = a->state.y;
        a->state.priority = 176;
        break;
    case 1:
    case 2:
        if (!ActorAnimate(s, a, a->state.step == 1 ? 0x1f58 : 0x1f68, &animated))
            return false;
        if (!animated)
            a->state.facing_left = Side(a, s->player) == 0;
        if (a->state.step == 1)
        {
            if (Distance(a, s->player) < 80 && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 64)
                SetStep(a, 3);
        }
        else if (a->crow.hover_timer)
            --a->crow.hover_timer;
        else
            SetStep(a, 3);
        break;
    case 3:
        switch (a->substep)
        {
        case 0:
            a->state.facing_left = Side(a, s->player) == 0;
            a->vx = a->state.facing_left ? 98304 : -98304;
            a->vy = 262144;
            if (!ActorPositionalSound(s, a, 0x6d7))
                return false;
            ++a->substep;
            break;
        case 1:
            if (!Move(a) || !ActorAnimate(s, a, a->vy < -131072 ? 0x1f74 : 0x1f68, &animated) ||
                (a->vy >= -131072 && !Velocity(a, a->vx, (Sint64)a->vy - 8192)))
                return false;
            if (a->vy < 0 && (Sint64)a->state.y - a->crow.reference_y < 0)
            {
                a->vx = 0;
                ++a->substep;
            }
            break;
        case 2:
            if (!ActorAnimate(s, a, 0x1f74, &animated) || !Move(a) ||
                !Velocity(a, a->vx, SDL_min((Sint64)a->vy + 16384, 32768)))
                return false;
            if ((Sint64)a->state.y - a->crow.reference_y > 0)
            {
                a->crow.hover_timer = 128;
                a->state.frame = 1;
                a->state.facing_left = Side(a, s->player) != 0;
                SetStep(a, 2);
            }
            break;
        default:
            return SDL_SetError("Chapel Crow entered an invalid swoop phase");
        }
        break;
    case 4:
        if (!Move(a) || !Velocity(a, a->vx, (Sint64)a->vy + 6144) ||
            !ActorAnimate(s, a, 0x1f80, &animated))
            return false;
        a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + 64);
        if (!animated)
        {
            if (!ActorExplosionInPool(s, a, 0, 160, 192, 0))
                return false;
            a->remove = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("Chapel Crow entered an invalid original state");
    }
    return Hitbox(s, a);
}
static bool Raven(SotnActors *s, Actor *a)
{
    // Original blue-raven death waits while matching height (step 3).
    if ((a->state.flags & 0x100U) && a->state.step != 3)
    {
        if (!ActorPositionalSound(s, a, 0x6d6) || !ActorExplosionInPool(s, a, 0, 160, 192, 0))
            return false;
        a->remove = true;
        return true;
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xa64))
            return false;
        a->state.frame = 11;
        a->crow.reference_y = a->state.y;
        a->state.priority = 176;
        break;
    case 1:
        if (!ActorAnimate(s, a, 0x1f88, &animated))
            return false;
        if (!animated)
            a->state.facing_left = Side(a, s->player) == 0;
        if (Distance(a, s->player) < 96 && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 48)
            SetStep(a, 2);
        break;
    case 2:
        a->vy = Pixel(s->player.y) > Pixel(a->state.y) ? 65536 : -65536;
        a->vx = 0;
        a->state.facing_left = Side(a, s->player) == 0;
        SetStep(a, 3);
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x1f90, &animated) || !Move(a))
            return false;
        if (SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 8)
        {
            a->crow.hover_timer = 32;
            SetStep(a, 4);
        }
        break;
    case 4:
        if (!a->crow.hover_timer || !ActorAnimate(s, a, 0x1f9c, &animated))
            return SDL_SetError("Chapel Raven has an invalid original hover timer");
        if (!--a->crow.hover_timer)
        {
            a->vx = a->state.facing_left ? 131072 : -131072;
            if (!ActorPositionalSound(s, a, 0x6d7))
                return false;
            ++a->state.step;
        }
        break;
    case 5:
        if (!ActorAnimate(s, a, 0x1f9c, &animated) || !Move(a) ||
            !Velocity(a, (Sint64)a->vx + (a->state.facing_left ? 4096 : -4096),
                      (Sint64)a->vy - 2048))
            return false;
        break;
    default:
        return SDL_SetError("Chapel Raven entered an invalid original state");
    }
    return Hitbox(s, a);
}
bool ActorChapelCrow(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || (a->state.id != 0x38 && a->state.id != 0x39))
        return SDL_SetError("Chapel bird requires its original stage and actor identity");
    return a->state.id == 0x38 ? Crow(s, a) : Raven(s, a);
}
