// SPDX-License-Identifier: AGPL-3.0-or-later
// boss/bo5/e_hippogryph.c and selected unreconstructed US BO5 attack routines,
// pinned sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Initialization, animation and pose-to-hitbox selector tables remain private.
#include "actors_internal.h"

static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Hippogryph attack exceeds bounded fixed-point position");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Velocity(Actor *a, Sint64 x, Sint64 y)
{
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Hippogryph attack exceeds bounded fixed-point velocity");
    a->vx = (Sint32)x;
    a->vy = (Sint32)y;
    return true;
}
static void Box(Actor *a, Sint16 x, Sint16 y, Uint16 width, Uint16 height)
{
    a->state.hitbox_x = x;
    a->state.hitbox_y = y;
    a->state.hitbox_width = width;
    a->state.hitbox_height = height;
}
// func_us_801A425C: the adjacent weapon follows its owned body, never another live slot.
static bool Weapon(SotnActors *s, Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Hippogryph weapon owner slot is invalid");
    const Actor *body = &s->slots[slot];
    if (!a->enemy_state.owner_generation ||
        body->state.generation != a->enemy_state.owner_generation || body->state.id != 0x22 ||
        body->remove)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x484))
            return false;
        a->state.hit_points = 0x7ffe;
        a->state.hitbox_state = 2;
        a->state.flags |= 0x400000U;
        a->state.x = Pixel(a->state.x) * 65536;
        a->state.y = Pixel(a->state.y) * 65536;
        a->attack = a->attack_element = 0;
    }
    SotnDataView selector = {0};
    const Uint8 frame = (Uint8)body->state.frame;
    if (!Slice(s->nz0, 0xb0cU + (frame < 104 ? frame : 0), 1, &selector))
        return false;
    switch (selector.bytes[0])
    {
    case 0:
        Box(a, 0, 0, 0, 0);
        break;
    case 1:
        Box(a, 8, -24, 20, 18);
        break;
    case 2:
        Box(a, 12, -8, 18, 8);
        break;
    case 3:
        Box(a, -2, 24, 8, 18);
        break;
    case 4:
        Box(a, 6, 10, 16, 10);
        break;
    case 5:
        Box(a, 16, -32, 22, 22);
        break;
    case 6:
        Box(a, 2, -32, 12, 24);
        break;
    default:
        break; // The original switch retains the previous box for a selector above six.
    }
    a->state.facing_left = body->state.facing_left;
    a->state.x = (Sint32)((Sint64)Pixel(body->state.x) * 65536 + ((Uint32)a->state.x & 65535U));
    ShiftY(a, (Sint64)Pixel(body->state.y) - Pixel(a->state.y));
    return true;
}
static bool Particle(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x46c))
            return false;
        a->state.animation_set = 2;
        a->state.frame = 1;
        a->state.rotated = a->state.opacity_set = a->state.scale_set = true;
        a->state.blend_mode |= 0x30;
        a->state.opacity = 128;
        a->vx = a->state.facing_left ? 65536 : -65536;
        a->vy = a->state.parameters ? 32768 : -65536;
        return true;
    }
    if (!Move(a) ||
        !Velocity(a, (Sint64)a->vx + (a->state.facing_left ? -8192 : 8192), (Sint64)a->vy - 4096))
        return false;
    a->state.rotation =
        (Sint16)Wrap16((Sint64)a->state.rotation + (a->state.parameters ? -64 : 64));
    a->state.scale_x = (Uint16)(a->state.scale_x - 10U);
    a->state.opacity = (Uint8)(a->state.opacity - 6U);
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, 0xa18, &animated))
        return false;
    a->remove = animated == 0;
    return true;
}
static bool FlyingObject(SotnActors *s, Actor *a)
{
    if (s->hippogryph_flags & 2U)
        a->state.flags |= 0x100U;
    if (a->state.flags & 0x100U)
    {
        if (!ActorExplosionInPool(s, a, 0, 96, 128, 0))
            return false;
        a->remove = true;
        return true;
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x4e4))
            return false;
        a->state.hitbox_state = 2;
        a->state.rotated = true;
        a->state.rotation = (Sint16)((Sint32)(ActorRandom(s) & 3U) * 512 - 768);
        a->hippogryph.rotation_direction = 1;
        return true;
    case 1: {
        Uint16 collision = 0;
        if (!ActorGround(s, a, 0xabc, &collision))
            return false;
        if (collision & 1U)
            ++a->state.step;
        SDL_FALLTHROUGH;
    }
    case 2:
        if (!a->hippogryph.timer)
        {
            if (!ActorAnimate(s, a, 0x9f0, &animated))
                return false;
            if (animated)
                return true;
            if (!ActorExplosionInPool(s, a, 0, 96, 128, 0))
                return false;
            a->state.facing_left = Side(a, s->player) != 0;
            a->state.rotated = a->state.opacity_set = a->state.scale_set = false;
            a->vx = a->state.facing_left ? -163840 : 163840;
            a->vy = -16384;
            a->pose = a->remaining = 0;
            a->state.hitbox_state = 3;
            if (!ActorPositionalSound(s, a, 0x7b7))
                return false;
            ++a->state.step;
        }
        else if (a->hippogryph.timer-- < 8)
        {
            a->state.rotation =
                (Sint16)Wrap16((Sint64)a->state.rotation + a->hippogryph.rotation_direction * 512);
            a->hippogryph.rotation_direction = (Uint8)(0U - a->hippogryph.rotation_direction);
        }
        return true;
    case 3:
        return ActorAnimate(s, a, 0x9f8, &animated) && Move(a) &&
               Velocity(a, a->vx, (Sint64)a->vy - 2560);
    default:
        return true; // The original function has no default transition.
    }
}
static bool Flame(SotnActors *s, Actor *a)
{
    if (s->hippogryph_flags & 2U)
    {
        a->remove = true;
        return true;
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x4f0))
            return false;
        a->state.animation_set = 2;
        a->state.frame = 1;
        a->state.scale_x = a->state.scale_y = 32;
        a->state.scale_set = a->state.opacity_set = a->state.rotated = true;
        a->state.opacity = a->state.parameters >= 2 ? 124 : 128;
        a->state.rotation = 0xe00;
        a->state.blend_mode |= 0x30;
        a->state.hitbox_state = a->state.parameters == 3 ? 1 : 0;
        a->vx = a->state.facing_left ? 288768 : -288768;
        a->vy = a->state.parameters == 0 ? -24576 : a->state.parameters >= 2 ? 0 : 12288;
        return true;
    case 1:
        if (!ActorAnimate(s, a, 0xa00, &animated) || !Move(a) ||
            !Velocity(a, a->vx, (Sint64)a->vy - 5120))
            return false;
        a->state.scale_x = (Uint16)(a->state.scale_x + 24U);
        a->state.scale_y = a->state.scale_x;
        a->state.opacity = (Uint8)(a->state.opacity - 5U);
        if (Wrap16(a->state.scale_x) >= 241)
        {
            a->vx /= 4;
            a->pose = a->remaining = 0;
            ++a->state.step;
        }
        if ((a->state.parameters == 3 && (ActorRandom(s) & 1U)) ||
            (a->state.parameters == 0 && (ActorRandom(s) & 7U)))
        {
            Actor *part = NULL;
            if (!ActorSpawn(s, a, 0x26, a->state.parameters, 160, 192, &part))
                return false;
            if (part)
            {
                part->state.x = Pixel(a->state.x) * 65536;
                part->state.y = Pixel(a->state.y) * 65536;
                part->state.scale_x = a->state.scale_x;
                part->state.scale_y = a->state.scale_y;
            }
        }
        return true;
    case 2:
        if (!Move(a) || !ActorAnimate(s, a, 0xa0c, &animated))
            return false;
        a->remove = animated == 0;
        return true;
    default:
        return true;
    }
}
bool ActorHippogryphAttack(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 25)
        return SDL_SetError("Hippogryph attack requires its original BO5 owner");
    switch (a->state.id)
    {
    case 0x23:
        return Weapon(s, a);
    case 0x24:
        if (!a->state.step)
        {
            if (!ActorInitialize(s, a, 0x4f0))
                return false;
            Box(a, -48, -2, 24, 4);
        }
        return true;
    case 0x25:
        return FlyingObject(s, a);
    case 0x26:
        return Particle(s, a);
    case 0x27:
        return Flame(s, a);
    default:
        return SDL_SetError("Hippogryph attack identity is not 0x23..0x27");
    }
}
