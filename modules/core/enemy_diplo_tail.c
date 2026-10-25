// SPDX-License-Identifier: AGPL-3.0-or-later
// st/no0/e_diplocephalus_tail.c at the pinned sotn-decomp reference.
// Segment frames, initializers and animation streams remain in private NO0 data.
#include "actors_internal.h"
static bool SpawnTail(SotnActors *s, Actor *a, const Actor *root)
{
    if ((Uint32)a->slot + 6 >= SOTN_STAGE_ACTOR_SLOTS || !ActorInitialize(s, a, 0xb48))
        return false;
    a->state.hitbox_state = 1;
    a->state.rotated = true;
    for (Uint16 i = 1; i <= 6; ++i)
    {
        Actor *part = NULL;
        if (!ActorSpawnAt(s, a, 0x24, 0, a->slot + i, &part))
            return false;
        part->enemy_state.owner_slot = root->slot;
        part->enemy_state.owner_generation = root->state.generation;
        part->diplo.index = i;
        part->state.step = i == 6 ? 5 : 3;
        part->state.animation_set = a->state.animation_set;
        part->state.frame = i == 6 ? 23 : i == 5 ? 22 : 19;
        part->state.facing_left = !root->state.facing_left;
        part->state.priority = i == 6 ? a->state.priority : (Uint16)(root->state.priority - i);
        part->state.palette = a->state.palette;
        part->state.texture_offset = a->state.texture_offset;
        part->state.flags = a->state.flags | (i == 6 ? 0x20000000U : 0);
        part->state.rotated = i != 6;
        part->state.hit_points = a->state.hit_points;
        part->state.hitbox_state = i == 6 ? 3 : 0;
        part->state.hitbox_width = i == 6 ? 8 : a->state.hitbox_width;
        part->state.hitbox_height = i == 6 ? 13 : a->state.hitbox_height;
        part->state.hitbox_y = i == 6 ? -8 : 0;
        part->enemy = a->enemy;
        part->attack = a->attack;
        part->attack_element = a->attack_element;
        if (i == 6)
            part->diplo.angle = 1280;
    }
    a->diplo.tip = (Uint16)(a->slot + 6);
    return true;
}
static bool Follow(SotnActors *s, Actor *a, const Actor *root)
{
    Actor *tip = &s->slots[a->diplo.tip];
    a->state.x = root->state.x + (a->state.facing_left ? -29 : 29) * 65536;
    a->state.y = root->state.y;
    const Sint32 dy = a->vy - a->diplo.previous_vy;
    a->diplo.angle = (Sint16)(a->diplo.angle + (dy > 0 ? 16 : dy < 0 ? -16 : 0));
    a->diplo.angle = (Sint16)(a->diplo.angle + (a->diplo.angle > 0   ? -8
                                                : a->diplo.angle < 0 ? 8
                                                                     : 0));
    a->diplo.angle = (Sint16)SDL_clamp(a->diplo.angle, -256, 256);
    Sint32 dx = a->vx - a->diplo.previous_vx;
    if (!a->state.facing_left)
        dx = -dx;
    tip->diplo.angle = (Sint16)(tip->diplo.angle + (dx > 0 ? 16 : dx < 0 ? -16 : 0));
    tip->diplo.angle = (Sint16)(tip->diplo.angle + (tip->diplo.angle > 1280   ? -8
                                                    : tip->diplo.angle < 1280 ? 8
                                                                              : 0));
    tip->diplo.angle = (Sint16)SDL_clamp(tip->diplo.angle, 768, 1792);
    const Sint32 middle = tip->diplo.angle - a->diplo.angle, first = (middle - a->diplo.angle) / 3,
                 last = (tip->diplo.angle - middle) / 3;
    for (Uint32 i = 0; i < 6; ++i)
    {
        Actor *part = i == 0 ? a : &s->slots[a->slot + i];
        Actor *next = &s->slots[a->slot + i + 1];
        if (part->state.id != 0x24 || next->state.id != 0x24 ||
            next->enemy_state.owner_generation != root->state.generation)
            return SDL_SetError("Diplocephalus tail lost an owned segment");
        part->diplo.angle =
            (Sint16)(i <= 3 ? a->diplo.angle + first * (Sint32)i : middle + last * (Sint32)(i - 4));
        next->state.x = part->state.x +
                        ActorSin(part->diplo.angle + 1024) * (a->state.facing_left ? -128 : 128);
        next->state.y = part->state.y - ActorSin(part->diplo.angle) * 128;
        part->state.rotation = (Sint16)-part->diplo.angle;
    }
    tip->state.x += 65536;
    ShiftY(tip, -2);
    a->diplo.previous_vx = root->vx;
    a->diplo.previous_vy = root->vy;
    if (++a->diplo.phase > 32)
    {
        Actor *part = &s->slots[a->slot + 1 + a->diplo.cycles];
        SetStep(part, 4);
        a->diplo.phase = 0;
        a->diplo.cycles = (Uint16)((a->diplo.cycles + 1) & 3);
    }
    return true;
}
bool ActorDiploTail(SotnActors *s, Actor *a, Actor *root)
{
    const Uint32 base = (Uint32)root->slot + 9;
    if (a->slot < base || a->slot > base + 6 || base + 6 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Diplocephalus tail has an invalid linked range");
    const Uint32 index = a->slot - base;
    if (index == 6)
        root->diplo.last_hit = a->hit_params;
    else if (a->state.hitbox_state)
    {
        a->state.hitbox_state = 0;
        s->slots[index ? a->slot - 1 : a->slot + 5].state.hitbox_state = 1;
    }
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!SpawnTail(s, a, root))
            return false;
        SDL_FALLTHROUGH;
    case 1:
        if (a->diplo.tip != base + 6)
            return SDL_SetError("Diplocephalus tail controller lost its tip");
        return Follow(s, a, root);
    case 3:
        break;
    case 4:
        if (!ActorAnimate(s, a, 0x1e30, &animation))
            return false;
        if (!animation)
            SetStep(a, 3);
        break;
    case 5:
        if (!ActorAnimate(s, a, 0x1dbc, &animation))
            return false;
        if (root->diplo.hits)
        {
            if (!ActorSound(s, 0x777))
                return false;
            SetStep(a, 6);
        }
        else if ((ActorRandom(s) & 127) == 0)
        {
            a->state.parameters = 1;
            SetStep(a, 11);
        }
        break;
    case 6:
        if (!ActorAnimate(s, a, 0x1ddc, &animation))
            return false;
        if (!animation)
            SetStep(a, 5);
        break;
    case 7:
        if (!ActorAnimate(s, a, 0x1dec, &animation))
            return false;
        if (!animation)
        {
            Actor origin = *a;
            origin.state.priority = (Uint16)(a->state.priority + 9);
            if (!ActorExplosionInPool(s, &origin, 3, 96, 128, origin.state.priority))
                return false;
            return ActorReplaceExplosion(s, a, 0);
        }
        break;
    case 8:
        if (index != 0)
            return SDL_SetError("Diplocephalus tail death controller is not its root");
        a->diplo.cycles = 1;
        a->diplo.phase = 0;
        a->diplo.tip = (Uint16)(base + 6);
        s->slots[base + 6].state.step = 7;
        s->slots[base + 6].state.flags &= ~0x20000000U;
        for (Uint32 i = 0; i <= 6; ++i)
            if (i == 0)
                a->state.hitbox_state = 0;
            else
                s->slots[base + i].state.hitbox_state = 0;
        if (!ActorSound(s, 0x778) || !ActorSound(s, 0x698))
            return false;
        ++a->state.step;
        break;
    case 9:
        if (a->diplo.tip < base || a->diplo.tip > base + 6)
            return SDL_SetError("Diplocephalus dying tail exceeded its original chain");
        if (a->diplo.tip != base + 6 || s->slots[a->diplo.tip].state.id != 0x24)
        {
            if (a->diplo.phase++ > 6)
            {
                a->diplo.phase = 0;
                ++a->diplo.cycles;
                if (a->diplo.tip == base)
                    return SDL_SetError("Diplocephalus dying tail stepped before its root");
                --a->diplo.tip;
                if (a->diplo.tip == base)
                    a->state.step = 10;
                else
                    s->slots[a->diplo.tip].state.step = 10;
            }
        }
        break;
    case 10:
        a->state.y += 16384;
        if (!ActorSound(s, 0x652))
            return false;
        return ActorReplaceExplosion(s, a, 0);
    case 11:
        if (!ActorAnimate(s, a, 0x1e0c, &animation))
            return false;
        if (!animation)
        {
            SetStep(a, 5);
            return true;
        }
        /* Byte animation cursor points one record beyond the current pose. */
        if (a->pose == 8 && a->state.parameters)
        {
            if (!ActorSound(s, 0x776))
                return false;
            Actor *fire = NULL;
            if (!ActorSpawn(s, a, 0x25, 0, 96, 128, &fire))
                return false;
            if (fire != NULL)
            {
                if (!ActorSound(s, 0x690))
                    return false;
                fire->state.x += (a->state.facing_left ? 12 : -12) * 65536;
                ShiftY(fire, -16);
                a->state.parameters = 0;
            }
        }
        break;
    default:
        return SDL_SetError("Diplocephalus tail entered invalid state %u", a->state.step);
    }
    return true;
}
