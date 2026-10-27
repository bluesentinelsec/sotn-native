// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO0 func_801AFAF4, func_801AF8C0 and func_801AF604 at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime tables stay private.
#include "olrox_internal.h"

static Actor *Parent(SotnActors *s, const Actor *a, Uint16 id, bool reserved)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *parent = &s->slots[slot];
    if (!parent->state.generation || parent->state.id != id ||
        parent->state.generation != a->enemy_state.owner_generation)
        return NULL;
    if (reserved && (a->slot <= slot || a->slot - slot > 13 ||
                     parent->olrox.children[a->slot - slot] != a->state.generation))
        return NULL;
    return parent;
}
static void Whole(Sint32 *position, Sint32 pixel)
{
    *position = (Sint32)((Sint64)Wrap16(pixel) * 65536 + ((Uint32)*position & 65535U));
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Olrox projectile movement exceeds fixed-point coordinates");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Spawn(SotnActors *s, Actor *a, Uint16 id, Uint16 parameters, Uint16 begin, Uint16 end,
                  Actor **out)
{
    if (!ActorSpawn(s, a, id, parameters, begin, end, out))
        return false;
    if (*out)
    {
        (*out)->state.x = Pixel(a->state.x) * 65536;
        (*out)->state.y = Pixel(a->state.y) * 65536;
        (*out)->state.facing_left = false;
        (*out)->enemy_state.owner_slot = a->slot;
        (*out)->enemy_state.owner_generation = a->state.generation;
    }
    return true;
}
static Uint8 Turn8(Uint8 current, Uint8 target)
{
    const Uint8 bits = (Uint8)(target - current);
    const Sint32 delta = bits < 128 ? bits : (Sint32)bits - 256;
    return (Uint8)(current + SDL_clamp(delta, -2, 2));
}
static bool Angle8(SotnActors *s, Sint32 dx, Sint32 dy, Uint8 *out)
{
    Sint16 angle = 0;
    if (!SotnMath_Atan2(s->executable, Wrap16(dy), Wrap16(dx), &angle))
        return false;
    *out = (Uint8)(ActorShift(angle, 4) + 64);
    return true;
}
static bool Velocity8(SotnActors *s, Actor *a, Uint8 angle, Sint16 speed)
{
    SotnDataView x = {0}, y = {0};
    if (!Slice(s->nz0, 0x187c + angle * 2U, 2, &x) ||
        !Slice(s->nz0, 0x187c + (Uint8)(angle - 64U) * 2U, 2, &y))
        return false;
    const Sint32 sx = Signed16(x.bytes), sy = Signed16(y.bytes);
    if (sx < -4096 || sx > 4096 || sy < -4096 || sy > 4096)
        return SDL_SetError("Olrox direction exceeds the private stage sine-table amplitude");
    a->vx = sx * speed;
    a->vy = sy * speed;
    return true;
}
static bool Orb(SotnActors *s, Actor *a)
{
    Actor *parent = Parent(s, a, 0x2b, true);
    if (!parent)
    {
        a->remove = true;
        return true;
    }
    if (parent->state.flags & 0x100U)
        a->state.flags |= 0x100U;
    if (a->state.flags & 0x100U)
    {
        if (!ActorExplosionInPool(s, a, 2, 160, 192, 0))
            return false;
        a->remove = true;
        // Original destruction clears the current entity before positional attenuation.
        const Actor cleared = {.state = {.x = s->camera_x * 65536, .y = s->camera_y * 65536}};
        return ActorPositionalSound(s, &cleared, 0x64e);
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters > 8 || !ActorInitialize(s, a, 0x6d8))
            return SDL_SetError("Olrox orb requires its original birth index and definition");
        a->state.hitbox_state = 0;
        a->state.hitbox_x = a->state.hitbox_y = 1;
        a->state.palette = 0x219;
        a->state.priority = (Uint16)(parent->state.priority + 2U);
        a->pose = (Uint16)((ActorRandom(s) & 3U) * 2U);
        a->olrox.timer = (Uint16)((a->state.parameters + 1U) * 14U);
        a->state.scale_set = true;
        a->state.transform_flags_set = true;
        a->state.transform_flags = SOTN_ACTOR_SCALE_X | SOTN_ACTOR_SCALE_Y;
        a->state.parameters = (Uint16)(ActorRandom(s) & 3U);
        return true;
    case 1: {
        if (!ActorAnimate(s, a, 0xd38, &animated))
            return false;
        const Sint32 old_x = Pixel(a->state.x), center_x = Pixel(parent->state.x),
                     center_y = Wrap16((Sint64)Pixel(parent->state.y) - 32), dx = old_x - center_x,
                     dy = Pixel(a->state.y) - center_y;
        const Sint64 squared = (Sint64)dx * dx + (Sint64)dy * dy;
        Uint32 length = 0;
        Sint16 angle = 0;
        SotnMathTrig trig = {0};
        if (squared > SDL_MAX_SINT32 || !SotnMath_Sqrt12(s->executable, (Uint32)squared, &length) ||
            !SotnMath_Atan2(s->executable, -dy, dx, &angle) ||
            !SotnMath_Trig(s->executable,
                           Wrap16((Sint32)angle + (parent->state.facing_left ? 16 : -16)), &trig))
            return false;
        const Sint32 radius = (Sint32)(length >> 6) - 2;
        Whole(&a->state.x, center_x + (Sint32)((Sint64)radius * trig.cosine / 4096));
        Whole(&a->state.y, center_y - (Sint32)((Sint64)radius * trig.sine / 4096));
        const Sint32 scale = Wrap16((Sint64)radius * a->state.parameters + 256);
        a->state.scale_x = a->state.scale_y = (Uint16)SDL_min(768, scale);
        a->state.facing_left = Pixel(a->state.x) - old_x <= 0;
        if (radius < 8)
        {
            a->olrox.angle = (Uint8)ActorRandom(s);
            a->state.palette = 0x209;
            a->state.hitbox_state = 3;
            a->state.scale_set = false;
            a->state.transform_flags = 0;
            ++a->state.step;
        }
        return true;
    }
    case 2: {
        if (!ActorAnimate(s, a, 0xd38, &animated) || !Move(a))
            return false;
        a->state.facing_left = a->vx > 0;
        a->state.priority = (Uint16)((Sint32)parent->state.priority + (a->vx > 0 ? 2 : -2));
        Uint8 target = 0;
        if (!Angle8(s, Pixel(parent->state.x) - Pixel(a->state.x),
                    Pixel(parent->state.y) - Pixel(a->state.y), &target))
            return false;
        a->olrox.angle = Turn8(a->olrox.angle, target);
        if (!Velocity8(s, a, a->olrox.angle, 16))
            return false;
        if (parent->olrox.release_orbs && --a->olrox.timer == 0)
        {
            if (!Angle8(s, Pixel(s->player.x) - Pixel(a->state.x),
                        Pixel(s->player.y) - Pixel(a->state.y), &target) ||
                !Velocity8(s, a, target, 32))
                return false;
            ++a->state.step;
        }
        return true;
    }
    case 3:
        if (!ActorAnimate(s, a, 0xd38, &animated) || !Move(a))
            return false;
        a->state.facing_left = a->vx <= 0;
        if (SDL_abs(Pixel(a->state.x) - s->camera_x) > 512 ||
            SDL_abs(Pixel(a->state.y) - s->camera_y) > 512)
            a->remove = true;
        return true;
    default:
        return SDL_SetError("Olrox orb entered an invalid phase");
    }
}
static bool Portal(SotnActors *s, Actor *a)
{
    Actor *parent = Parent(s, a, 0x2b, true);
    if (!parent)
    {
        a->remove = true;
        return true;
    }
    if (parent->state.flags & 0x100U)
    {
        a->state.step = 3;
        a->state.flags |= 0x100U;
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x6cc))
            return false;
        a->state.frame = 84;
        a->state.scale_x = a->state.scale_y = 0;
        a->state.scale_set = true;
        a->state.transform_flags_set = true;
        a->state.transform_flags = SOTN_ACTOR_SCALE_X | SOTN_ACTOR_SCALE_Y;
        a->state.hitbox_state = 0;
        --a->state.priority;
        return true;
    case 1:
        if (!ActorAnimate(s, a, 0xd7c, &animated))
            return false;
        if (a->state.scale_x < 256)
            a->state.scale_x = a->state.scale_y = (Uint16)(a->state.scale_y + 8U);
        else
        {
            a->state.scale_x = a->state.scale_y = 256;
            ++a->state.step;
        }
        return true;
    case 2:
        if (!ActorAnimate(s, a, 0xd7c, &animated))
            return false;
        if (s->ticks % 12 == 0)
        {
            Actor *child = NULL;
            if (!Spawn(s, a, 0x2f, 0, 96, 104, &child) ||
                (child && !ActorPositionalSound(s, a, 0x691)))
                return false;
        }
        a->state.palette = s->ticks % 5 == 0 ? 0x8215 : 0x8213;
        if (parent->olrox.close_portal)
            ++a->state.step;
        return true;
    case 3:
        if (!ActorAnimate(s, a, 0xd7c, &animated))
            return false;
        a->state.scale_x = a->state.scale_y = (Uint16)(a->state.scale_y - 8U);
        if (a->state.scale_y & 0x8000U)
            a->remove = true;
        return true;
    default:
        return SDL_SetError("Olrox portal entered an invalid phase");
    }
}
static bool Bat(SotnActors *s, Actor *a)
{
    if (s->olrox_flags)
        a->state.flags |= 0x100U;
    if (a->state.flags & 0x100U)
    {
        Actor *effect = NULL;
        if (!ActorPositionalSound(s, a, 0x655) || !Spawn(s, a, 0x47, 2, 160, 192, &effect))
            return false;
        a->remove = true;
        return true;
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters > 1 || !ActorInitialize(s, a, 0x6e4))
            return SDL_SetError("Olrox bat requires an original primary/shadow definition");
        a->state.hitbox_state = 0;
        a->state.hitbox_y = -1;
        a->state.opacity_set = true;
        a->state.transform_flags_set = true;
        a->state.transform_flags = SOTN_ACTOR_OPACITY;
        a->olrox.angle12 = (Sint16)((ActorRandom(s) & 63U) * 16U + 512U);
        if (a->state.parameters)
        {
            a->state.step = 2;
            --a->state.priority;
            a->state.blend_mode |= 0x30;
        }
        else
        {
            Actor *shadow = NULL;
            if (!Spawn(s, a, 0x2f, 1, 160, 192, &shadow))
                return false;
        }
        return true;
    case 1: {
        if (a->state.opacity < 128)
        {
            a->state.hitbox_state = 0;
            a->state.opacity = (Uint8)(a->state.opacity + 4U);
        }
        else
            a->state.hitbox_state = 3;
        if (!ActorAnimate(s, a, 0xd74, &animated) || !Move(a))
            return false;
        a->state.facing_left = a->vx > 0;
        Sint16 target = 0;
        SotnMathTrig trig = {0};
        if (!SotnMath_Atan2(s->executable,
                            -Wrap16((Sint64)Pixel(s->player.y) - Pixel(a->state.y) - 32),
                            Wrap16((Sint64)Pixel(s->player.x) - Pixel(a->state.x)), &target))
            return false;
        const Sint32 current = a->olrox.angle12 & 4095;
        Sint32 delta = (target & 4095) - current;
        if (delta > 2048)
            delta -= 4096;
        if (delta < -2048)
            delta += 4096;
        a->olrox.angle12 = (Sint16)((current + SDL_clamp(delta, -8, 8)) & 4095);
        if (!SotnMath_Trig(s->executable, a->olrox.angle12, &trig))
            return false;
        a->vx = trig.cosine * 16;
        a->vy = -trig.sine * 16;
        return true;
    }
    case 2: {
        Actor *parent = Parent(s, a, 0x2f, false);
        if (!parent)
        {
            a->remove = true;
            return true;
        }
        if (!ActorAnimate(s, a, 0xd68, &animated))
            return false;
        Whole(&a->state.x, Pixel(parent->state.x));
        Whole(&a->state.y, Pixel(parent->state.y));
        a->state.opacity = parent->state.opacity;
        return true;
    }
    default:
        return SDL_SetError("Olrox bat entered an invalid phase");
    }
}
bool ActorOlroxOrbs(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation || s->camera_x < 0 ||
        s->camera_x > 16384 || s->camera_y < 0 || s->camera_y > 16384)
        return SDL_SetError("Olrox orb/portal/bat requires its original active BO0 owner");
    switch (a->state.id)
    {
    case 0x2d:
        return Orb(s, a);
    case 0x2e:
        return Portal(s, a);
    case 0x2f:
        return Bat(s, a);
    default:
        return SDL_SetError("Olrox orb/portal/bat identity is invalid");
    }
}
