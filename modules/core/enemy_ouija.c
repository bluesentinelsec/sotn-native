// SPDX-License-Identifier: AGPL-3.0-or-later
// st/no0/e_ouija_table.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Collision/animation data and SDK angle lookup remain private.
#include "actors_internal.h"
static Actor *Parent(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *parent = &s->slots[slot];
    return parent->state.id == 0x4a && parent->state.generation == a->enemy_state.owner_generation
               ? parent
               : NULL;
}
static bool Component(SotnActors *s, Actor *a)
{
    Actor *parent = Parent(s, a);
    if (a->state.step < 8 && parent == NULL)
        SetStep(a, 8);
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters > 3)
            return SDL_SetError("Ouija component type exceeds its original range");
        if (!ActorInitialize(s, a, 0xc20))
            return false;
        a->state.frame = (Uint16)(a->state.parameters < 3 ? 10 + a->state.parameters * 2 : 15);
        if (a->state.parameters >= 2)
        {
            a->state.hitbox_state = 0;
            a->state.step = 16;
        }
        break;
    case 1:
        if (a->table.spawned)
            ++a->state.step;
        break;
    case 2:
    case 3: {
        const bool seeking = a->state.step == 2;
        if (seeking)
        {
            a->state.x += a->vx;
            a->state.y += a->vy;
        }
        const Sint32 x =
            Pixel(parent->state.x) + ActorShift((Sint64)ActorSin(a->table.angle) * 18, 12);
        const Sint32 y = Pixel(parent->state.y) - 32 +
                         ActorShift((Sint64)ActorSin(a->table.angle + 1024) * 8, 12);
        if (seeking)
        {
            Sint16 angle = 0;
            const Sint32 dx = x - Pixel(a->state.x), dy = y - Pixel(a->state.y);
            if (!SotnMath_Atan2(s->executable, dy, dx, &angle))
                return false;
            a->vx = ActorSin(angle + 1024) * 24;
            a->vy = ActorSin(angle) * 24;
            if (SDL_abs(dx) < 2 && SDL_abs(dy) < 2)
                ++a->state.step;
        }
        else
        {
            a->state.x += (x - Pixel(a->state.x)) * 65536;
            a->state.y += (y - Pixel(a->state.y)) * 65536;
        }
        a->table.angle = (Uint16)(a->table.angle + (a->state.parameters ? 48 : -48));
        if (!seeking && a->table.thrown)
        {
            Sint16 angle = 0;
            if (!SotnMath_Atan2(s->executable, Pixel(s->player.y) - Pixel(a->state.y),
                                Pixel(s->player.x) - Pixel(a->state.x), &angle))
                return false;
            a->vx = ActorSin(angle + 1024) * 64;
            a->vy = ActorSin(angle) * 64;
            ++a->state.step;
        }
        break;
    }
    case 4:
        if (a->table.thrown)
        {
            if (a->hit_flags & 0x80)
                a->table.thrown = false;
            else if (a->hit_flags)
                a->vx = a->vy = 0;
        }
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vx -= a->vx / 32;
        a->vy -= a->vy / 32;
        if (SDL_abs(a->vx) < 32768 && SDL_abs(a->vy) < 32768)
        {
            a->table.thrown = false;
            a->state.step = 2;
        }
        break;
    case 8:
        if (a->substep == 0)
        {
            a->state.hitbox_state = 0;
            a->vx = a->vy = 0;
            a->state.rotated = true;
            ++a->substep;
        }
        if (a->substep == 1)
        {
            Uint16 collision = 0;
            a->state.rotation = (Sint16)((a->state.rotation + 64) & 4095);
            if (!ActorGround(s, a, 0x25e4, &collision))
                return false;
            if (collision & 1)
            {
                a->state.rotated = false;
                ++a->state.frame;
                a->table.timer = 32;
                if (!ActorSound(s, 0x622))
                    return false;
                ++a->substep;
            }
        }
        else if (a->substep == 2)
        {
            if (!a->table.timer)
                return SDL_SetError("Ouija component death timer underflow");
            if (--a->table.timer == 0)
            {
                if (!ActorExplosion(s, a, 0) || !ActorSound(s, 0x691))
                    return false;
                a->remove = true;
            }
        }
        else
            return SDL_SetError("Ouija component death substate is invalid");
        break;
    case 16:
        break;
    default:
        return SDL_SetError("Ouija component entered invalid state %u", a->state.step);
    }
    return true;
}
bool ActorOuija(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x4b)
        return Component(s, a);
    if (a->state.flags & 0x100)
    {
        if (!ActorExplosion(s, a, 1) || !ActorSound(s, 0x68f))
            return false;
        for (Uint32 i = 1; i < 3; ++i)
            if ((Uint32)a->slot + i < SOTN_STAGE_ACTOR_SLOTS)
                s->slots[a->slot + i].state.flags |= 0x2000;
        a->remove = true;
        return true;
    }
    Uint16 collision = 0;
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xc14))
            return false;
        a->state.frame = 5;
        a->state.hitbox_y = 2;
        break;
    case 1:
        if (!ActorGround(s, a, 0x25d4, &collision))
            return false;
        if (collision & 1)
        {
            if ((Uint32)a->slot + 3 >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Ouija table lacks its three reserved component slots");
            for (Uint32 i = 1; i <= 3; ++i)
                if (s->slots[a->slot + i].state.generation)
                    return SDL_SetError("Ouija table component slot is occupied");
            for (Uint16 i = 0; i < 3; ++i)
            {
                Actor *part = NULL;
                if (!ActorSpawnAt(s, a, 0x4b, i, (Uint32)a->slot + i + 1, &part))
                    return false;
                part->enemy_state.owner_slot = a->slot;
                part->enemy_state.owner_generation = a->state.generation;
                part->state.x += (i == 0                ? -4
                                  : i == 1              ? 2
                                  : a->state.parameters ? 24
                                                        : -24) *
                                 65536;
                ShiftY(part, i == 0 ? -16 : i == 1 ? -20 : 0);
                if (i == 2 && a->state.parameters)
                    part->state.facing_left = true;
            }
            ++a->state.step;
        }
        break;
    case 2:
        if (Distance(a, s->player) < 80 && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 64)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            s->slots[a->slot + 1].table.spawned = s->slots[a->slot + 2].table.spawned = true;
            SetStep(a, 3);
        }
        break;
    case 3:
        if (!a->substep)
        {
            a->table.timer = 128;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x25f4, &animation))
            return false;
        if (animation == 0)
            a->state.facing_left = Side(a, s->player) == 0;
        if (!ActorWalk(s, a, 0x25cc, &collision))
            return false;
        a->vx = a->state.facing_left ? 32768 : -32768;
        if (!a->table.timer)
            return SDL_SetError("Ouija attack timer underflow");
        if (--a->table.timer == 0)
        {
            a->table.timer = 192;
            s->slots[a->slot + (a->table.thrown ? 1 : 2)].table.thrown = true;
            a->table.thrown = !a->table.thrown;
        }
        break;
    case 4:
        break;
    default:
        return SDL_SetError("Ouija table entered invalid state %u", a->state.step);
    }
    return true;
}
