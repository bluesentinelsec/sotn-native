// SPDX-License-Identifier: AGPL-3.0-or-later
// Reviewed Slogra/spear state machines, nz0/slogra.c, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All animation/hitbox tables remain private.
#include "actors_internal.h"
static void Substep(Actor *a, Uint16 substep)
{
    a->substep = substep;
    a->pose = a->remaining = 0;
}
static bool Floor(Actor *a)
{
    a->state.x += a->vx;
    a->state.y += a->vy;
    a->vy += 16384;
    if (a->vx > 0 && Pixel(a->state.x) > 896)
        a->state.x = 896 * 65536 + (Sint32)((Uint32)a->state.x & 65535);
    if (a->vx < 0 && Pixel(a->state.x) < 64)
        a->state.x = 64 * 65536 + (Sint32)((Uint32)a->state.x & 65535);
    if (Pixel(a->state.y) > 416)
    {
        ShiftY(a, 416 - Pixel(a->state.y));
        a->vx = a->vy = 0;
        return true;
    }
    return false;
}
static bool Main(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    Uint16 collision = 0;
    bool landed = false;
    a->boss.pickup = false;
    if (a->state.step > 2)
    {
        if ((a->hit_flags & 3) && a->state.step != 8)
            SetStep(a, 8);
        if (!a->boss.near_death && a->state.hit_points < a->boss.maximum / 4 && a->state.step != 12)
            SetStep(a, 12);
    }
    if ((a->state.flags & 0x100) && a->state.step != 16)
        SetStep(a, 16);
    const Actor *gaibon = &s->slots[24];
    if (gaibon->state.id == 0x43 && gaibon->boss.ascending && a->state.step != 16 &&
        a->state.step != 13)
        SetStep(a, 13);
    switch (a->state.step)
    {
    case 0: {
        Actor *spear = NULL;
        if (a->slot != 16 || !ActorInitialize(s, a, 0xd0c) ||
            !ActorSpawnAt(s, a, 0x41, 0, (Uint32)a->slot + 1, &spear))
            return false;
        a->boss.maximum = (Uint16)SDL_max(0, a->state.hit_points);
        if (spear != NULL)
        {
            spear->enemy_state.owner_slot = a->slot;
            spear->enemy_state.owner_generation = a->state.generation;
        }
        if (Floor(a))
            SetStep(a, 2);
        break;
    }
    case 1:
        if (Floor(a))
            SetStep(a, 2);
        break;
    case 2:
        if (!ActorAnimate(s, a, 0x1080, &animation))
            return false;
        if (a->hit_flags)
            s->boss_flags |= 4;
        if (s->boss_flags & 4)
            SetStep(a, 9);
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x1080, &animation))
            return false;
        if (animation == 0)
            SetStep(a, 9);
        break;
    case 9:
    case 11: {
        const bool spear = a->state.step == 9;
        if (a->substep == 0)
        {
            a->state.facing_left = (Side(a, s->player) ^ 1) != 0;
            a->boss.flag = spear ? a->boss.attack_mode : 1;
            a->boss.timer = 128;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, spear ? 0x1074 : 0x10fc, &animation))
            return false;
        a->state.facing_left = (Side(a, s->player) ^ 1) != 0;
        a->vx = (a->state.facing_left ^ (a->boss.flag != 0)) ? 49152 : -49152;
        if (!ActorWalk(s, a, 0x106c, &collision))
            return false;
        if (!a->boss.flag && Distance(a, s->player) < 72)
        {
            if (spear && !a->boss.attack_mode)
                a->boss.timer = 1;
            else
                a->boss.flag ^= 1;
        }
        if (a->boss.flag && Distance(a, s->player) > 112)
        {
            if (spear && a->boss.attack_mode)
                a->boss.timer = 1;
            else
                a->boss.flag ^= 1;
        }
        if ((ActorRandom(s) & 63) == 0)
            SetStep(a, spear ? 3 : 10);
        if (--a->boss.timer == 0)
        {
            SetStep(a, spear ? (a->boss.attack_mode ? 5 : 4) : 6);
            if (spear)
                a->boss.attack_mode ^= 1;
        }
        break;
    }
    case 4:
        if (a->substep == 0)
        {
            if (!ActorSound(s, 0x709))
                return false;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x10b4, &animation))
            return false;
        if (animation == 0)
            SetStep(a, 9);
        if (a->remaining == 0 && a->pose == 4 && !ActorSound(s, 0x6c6))
            return false;
        break;
    case 5:
        if (a->substep == 0)
        {
            a->state.facing_left = (Side(a, s->player) ^ 1) != 0;
            a->substep = 1;
        }
        if (!ActorAnimate(s, a,
                          a->substep == 1   ? 0x108c
                          : a->substep == 2 ? 0x109c
                                            : 0x10a8,
                          &animation))
            return false;
        if (animation == 0)
        {
            if (a->substep == 1)
            {
                Actor *shot = NULL;
                if (!ActorSound(s, 0x690) || !ActorSpawn(s, a, 0x42, 0, 96, 128, &shot))
                    return false;
                if (shot != NULL)
                {
                    shot->state.x += (a->state.facing_left ? 68 : -68) * 65536;
                    ShiftY(shot, -6);
                    shot->state.priority = (Uint16)(a->state.priority + 1);
                }
                Substep(a, 2);
            }
            else if (a->substep == 2)
                Substep(a, 3);
            else
                SetStep(a, 9);
        }
        break;
    case 8:
        if (a->substep == 0)
        {
            if (Floor(a))
                a->substep = 1;
            break;
        }
        if (a->substep == 1)
        {
            a->vy = -4 * 65536;
            a->vx = Side(a, s->player) ? 32768 : -32768;
            if (!ActorSound(s, 0x70c))
                return false;
            a->substep = 2;
        }
        if (a->vy < 98304)
            a->boss.pickup = true;
        landed = Floor(a);
        a->vy -= 9216;
        if (!ActorAnimate(s, a, a->boss.near_death ? 0x1114 : 0x10d4, &animation))
            return false;
        if (animation == 0 && landed)
            SetStep(a, a->boss.near_death ? 10 : 9);
        break;
    case 12:
        landed = Floor(a);
        if (!ActorAnimate(s, a, 0x10e8, &animation))
            return false;
        if (animation == 0 && landed)
            SetStep(a, 10);
        if (a->pose > 1)
            a->boss.near_death = true;
        break;
    case 10:
        if (!ActorAnimate(s, a, 0x1108, &animation))
            return false;
        if (animation == 0)
            SetStep(a, 11);
        break;
    case 6:
        if (!ActorAnimate(s, a, 0x1128, &animation))
            return false;
        if (animation == 0)
            SetStep(a, 11);
        if (a->remaining == 0 && a->pose == 7 && !ActorSound(s, 0x6c8))
            return false;
        break;
    case 13:
        switch (a->substep)
        {
        case 0:
            if (gaibon->state.id != 0x43 || !gaibon->boss.ascending)
            {
                a->vx = a->vy = 0;
                ++a->substep;
            }
            break;
        case 1:
            if (!ActorAnimate(s, a, a->boss.near_death ? 0x1140 : 0x1158, &animation))
                return false;
            a->state.x += a->vx;
            a->state.y += a->vy;
            a->vy += 16384;
            if (Pixel(a->state.y) > 416)
            {
                ShiftY(a, 416 - Pixel(a->state.y));
                if (!ActorSound(s, 0x64f))
                    return false;
                a->boss.timer = 16;
                ++a->substep;
            }
            break;
        case 2:
            if (--a->boss.timer == 0)
                SetStep(a, a->boss.near_death ? 11 : 9);
            break;
        default:
            return SDL_SetError("Slogra combo substep is invalid");
        }
        break;
    case 16:
        if (a->substep == 0)
        {
            a->state.hitbox_state = 0;
            a->boss.near_death = true;
            a->boss.timer = 64;
            if (!ActorSound(s, 0x697))
                return false;
            a->substep = 1;
        }
        if (a->substep == 1)
        {
            (void)Floor(a);
            if (!ActorAnimate(s, a, 0x1150, &animation))
                return false;
            if ((s->ticks & 3) == 0)
            {
                Actor *effect = NULL;
                if (!ActorSpawn(s, a, 2, 1, 160, 192, &effect))
                    return false;
                if (effect != NULL)
                {
                    effect->state.x += ((Sint32)(ActorRandom(s) & 31) - 16) * 65536;
                    ShiftY(effect, (Sint32)(ActorRandom(s) & 31) - 16);
                    effect->state.priority = (Uint16)(a->state.priority + 1);
                }
            }
            if (--a->boss.timer == 0)
                ++a->substep;
        }
        else if (a->substep == 2)
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 2, 3, 160, 192, &effect))
                return false;
            if (effect != NULL)
                ShiftY(effect, 16);
            s->boss_flags |= 16;
            a->remove = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("Slogra entered an unsupported step");
    }
    return ActorBossBox(s, a, 0x1178, 16, 0x11b8, 40);
}
static bool Spear(SotnActors *s, Actor *a)
{
    if (a->state.step == 0 && !ActorInitialize(s, a, 0xd18))
        return false;
    if (a->state.step == 1)
    {
        if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Slogra spear parent slot is invalid");
        const Actor *parent = &s->slots[a->enemy_state.owner_slot];
        if (parent->state.id != 0x40 || parent->state.generation != a->enemy_state.owner_generation)
        {
            a->remove = true;
            return true;
        }
        a->state.facing_left = parent->state.facing_left;
        // Original attachment copies integer halves, retaining its fractional coordinates.
        a->state.x = Pixel(parent->state.x) * 65536 + (Sint32)((Uint32)a->state.x & 65535);
        ShiftY(a, Pixel(parent->state.y) - Pixel(a->state.y));
        const Uint16 old = a->state.frame;
        a->state.frame = parent->state.frame;
        if (!ActorBossBox(s, a, 0x11e0, 14, 0x1218, 40))
            return false;
        a->state.frame = old;
        if (parent->boss.near_death)
            a->state.step = 2;
    }
    else if (a->state.step == 2)
    {
        if (a->substep == 0)
        {
            a->state.rotated = true;
            a->state.hitbox_state = 0;
            a->vx = a->state.facing_left ? -147456 : 147456;
            a->vy = -4 * 65536;
            a->state.frame = 35;
            a->state.flags |= 0x80000000U;
            ++a->substep;
        }
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += 10240;
        a->state.rotation = (Sint16)Wrap16(a->state.rotation + 128);
        if ((a->state.rotation & 4095) == 0 && !ActorSound(s, 0x625))
            return false;
    }
    return true;
}
bool ActorSlogra(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x40)
        return Main(s, a);
    if (a->state.id == 0x41)
        return Spear(s, a);
    if (a->state.id != 0x42)
        return SDL_SetError("Unknown Slogra actor");
    if (a->state.flags & 0x100)
    {
        if (!ActorExplosion(s, a, 1))
            return false;
        a->remove = true;
        return true;
    }
    Uint8 animation = 0;
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xd24))
            return false;
        a->vx = a->state.facing_left ? 4 * 65536 : -4 * 65536;
    }
    if (a->state.step == 1)
    {
        if (!ActorAnimate(s, a, 0x1160, &animation))
            return false;
        if (animation == 0)
            SetStep(a, 2);
    }
    else if (a->state.step == 2)
    {
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (!ActorAnimate(s, a, 0x1170, &animation))
            return false;
    }
    else
        return SDL_SetError("Slogra projectile step is invalid");
    return true;
}
