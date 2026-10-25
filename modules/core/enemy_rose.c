// SPDX-License-Identifier: AGPL-3.0-or-later
// st/no0/e_stone_rose.c at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Linked actors use checked slots/generations; all original frame/sway data stays private.
#include "actors_internal.h"
static void Animation(Actor *a, size_t stream)
{
    a->animation_stream = stream;
    a->pose = a->remaining = 0;
}
static bool Wave(SotnActors *s, Actor *a)
{
    a->rose.parameter = (Uint16)Wrap16(
        (Sint64)a->rose.base + ActorShift((Sint64)ActorSin(a->rose.phase) * a->rose.amplitude, 12));
    if ((s->ticks & 7) == 0 || a->state.step >= 7)
        a->rose.sway = (Uint16)((a->rose.sway + 1) & 15);
    Sint32 scale = 0;
    if (a->state.parameters == 0)
    {
        SotnDataView sway = {0};
        /* The original masks to 16 entries although its declared array has 15.
         * Read the adjacent original value from the bounded image, without a C overrun. */
        if (!Slice(s->nz0, 0x2324 + a->rose.sway * 2U, 2, &sway))
            return false;
        scale = Signed16(sway.bytes);
    }
    else if (a->state.parameters == 1)
        scale = 48;
    else if (a->state.parameters == 2)
        scale = 32;
    else if (a->state.parameters == 3)
        scale = 16;
    else if (a->state.parameters == 4)
        scale = 8;
    if (scale + 256 <= 0 || scale + 256 > 1024)
        return SDL_SetError("Stone Rose scale exceeds its original range");
    a->state.scale_x = a->state.scale_y = (Uint16)(scale + 256);
    if (a->state.parameters && a->state.step < 8)
    {
        if (a->slot == 0)
            return SDL_SetError("Stone Rose segment lacks its preceding actor");
        const Actor *previous = &s->slots[a->slot - 1];
        if (previous->state.id != 0x3d || !previous->state.generation)
            return SDL_SetError("Stone Rose segment chain is stale");
        a->state.x = previous->state.x;
        a->state.y = previous->state.y;
        if (a->state.parameters != 11)
        {
            a->state.x += ActorSin(previous->rose.parameter + 1024) * 112;
            a->state.y -= ActorSin(previous->rose.parameter) * 112;
        }
    }
    return true;
}
bool ActorRose(SotnActors *s, Actor *a)
{
    const Uint16 part = a->state.parameters;
    if (part > 11 || part > a->slot || a->slot - part + 11 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Stone Rose does not fit its twelve original segment slots");
    const Uint32 base = (Uint32)a->slot - part;
    Actor *root = part == 0 ? a : &s->slots[base];
    if (a->state.step < 7 &&
        (Pixel(root->state.x) - s->camera_x > 384 || Pixel(root->state.x) - s->camera_x < -128 ||
         Pixel(root->state.y) - s->camera_y > 384 || Pixel(root->state.y) - s->camera_y < 0))
        return true;
    if (part &&
        (root->state.id != 0x3d || root->state.generation != a->enemy_state.owner_generation))
    {
        a->remove = true;
        return true;
    }
    Actor *head = part == 11 ? a : &s->slots[base + 11];
    if ((head->hit_flags & 15) && a->state.step < 7)
    {
        if (head->state.flags & 0x100)
        {
            if (!ActorSound(s, 0x731))
                return false;
            for (Uint32 i = 0; i < 12; ++i)
            {
                Actor *segment = i == part ? a : &s->slots[base + i];
                if (segment->state.id != 0x3d || !segment->state.generation)
                    return SDL_SetError("Stone Rose death found a missing segment");
                segment->state.hitbox_state = 0;
                segment->state.step = 7;
                segment->rose.timer = 192;
                segment->state.flags |= 0x100;
            }
        }
        else if (head->state.step != 3 && head->state.step != 5 && head->state.step != 6)
        {
            if (head->state.hit_points >= 31 || part != 11 || a->rose.hit)
            {
                if (!ActorSound(s, 0x730))
                    return false;
                if (a->state.step != 5 && a->state.step != 6)
                    a->substep = a->state.step;
                a->state.step = 5;
                a->rose.timer = 40;
                if (head->state.hit_points > 30 && part == 11)
                    Animation(a, 0x2280);
            }
            else
            {
                a->rose.hit = true;
                Animation(a, 0x22b4);
                a->state.step = 3;
                a->state.rotated = true;
            }
        }
    }
    bool ended = false;
    switch (a->state.step)
    {
    case 0:
        if (part == 0)
        {
            for (Uint32 i = 1; i < 12; ++i)
                if (s->slots[base + i].state.generation)
                    return SDL_SetError("Stone Rose reserved segment slot is occupied");
            for (Uint16 i = 1; i < 12; ++i)
            {
                Actor *segment = NULL;
                if (!ActorSpawnAt(s, a, 0x3d, i, base + i, &segment))
                    return false;
                segment->enemy_state.owner_slot = a->slot;
                segment->enemy_state.owner_generation = a->state.generation;
            }
        }
        if (!ActorInitialize(s, a, 0xaf4))
            return false;
        if (part == 11)
            a->state.flags &= ~0x04000000U;
        a->state.frame = part == 0 ? 1 : part == 11 ? 2 : 9;
        a->state.priority = (Uint16)(0x74 + part * 2);
        a->state.facing_left = Side(a, s->player) != 0;
        if (part == 0)
        {
            a->state.hitbox_width = a->state.hitbox_height = 12;
            a->state.hitbox_x = -8;
            a->state.hitbox_y = 8;
        }
        if (part == 11)
        {
            a->state.hitbox_width = 16;
            a->state.hitbox_height = 8;
            a->state.hitbox_x = 16;
            a->state.hitbox_y = 0;
        }
        else
            a->state.hit_points = 0x7fff;
        a->rose.phase = part * 0x180;
        a->rose.base = 0x480;
        a->rose.amplitude = part * 0x40 + 0x100;
        if (part < 3)
            a->rose.amplitude = a->rose.amplitude * (part + 1) / 4;
        break;
    case 1:
        if (Pixel(a->state.x) - s->camera_x < 288 || Pixel(a->state.x) - s->camera_x >= -31 ||
            Pixel(a->state.y) - s->camera_y < 232)
            ++a->rose.seed_timer;
        a->rose.phase = Wrap16((Sint64)a->rose.phase + 48);
        if ((a->rose.seed_timer & 31) == 31 && part == 11)
        {
            Animation(a, 0x228c);
            if (!ActorSound(s, 0x6ab))
                return false;
            a->state.step = 2;
        }
        break;
    case 2:
        a->rose.phase = Wrap16((Sint64)a->rose.phase + 48);
        if (!ActorWideAnimation(s, a, a->animation_stream, &ended))
            return false;
        if (a->pose == 8 && a->remaining == 47)
        {
            if (!ActorSound(s, 0x6d1) || !ActorRoseSpawn(s, a, 0))
                return false;
        }
        if (ended)
        {
            a->state.step = 1;
            a->state.frame = 2;
        }
        break;
    case 3:
        a->rose.phase = Wrap16((Sint64)a->rose.phase + 256);
        a->rose.recoil = (Uint16)(a->rose.recoil + 384);
        a->state.rotation = (Sint16)ActorShift(ActorSin(a->rose.recoil), 3);
        if (a->rose.recoil >= 24576)
            a->state.rotated = false;
        if (!ActorWideAnimation(s, a, a->animation_stream, &ended))
            return false;
        if (ended)
        {
            Animation(a, 0x2300);
            if (!ActorSound(s, 0x6ab))
                return false;
            ++a->state.step;
        }
        break;
    case 4:
        a->state.hitbox_width = 8;
        a->state.hitbox_height = 28;
        a->state.hitbox_x = 8;
        a->state.hitbox_y = 0;
        a->rose.phase = Wrap16((Sint64)a->rose.phase + 48);
        if (!ActorWideAnimation(s, a, a->animation_stream, &ended))
            return false;
        --a->rose.seed_timer;
        if ((a->rose.seed_timer & 63) == 31)
        {
            if (!ActorSound(s, 0x6d1) || !ActorRoseSpawn(s, a, 1))
                return false;
        }
        break;
    case 5:
        if (part == 11 && !ActorWideAnimation(s, a, a->animation_stream, &ended))
            return false;
        a->rose.base -= 8;
        a->rose.phase = Wrap16((Sint64)a->rose.phase + 192);
        if (!a->rose.timer)
            return SDL_SetError("Stone Rose recoil timer underflow");
        if (--a->rose.timer == 0)
        {
            a->rose.timer = 12;
            ++a->state.step;
        }
        break;
    case 6:
        if (part == 11 && !ActorWideAnimation(s, a, a->animation_stream, &ended))
            return false;
        if (a->rose.timer)
            --a->rose.timer;
        else
        {
            a->rose.phase = Wrap16((Sint64)a->rose.phase + 48);
            a->rose.base += 16;
            if (a->rose.base >= 0x480)
            {
                a->rose.base = 0x480;
                a->state.step = a->substep;
                if (part == 11 && (a->state.step == 1 || a->state.step == 2))
                {
                    a->pose = 7;
                    a->remaining = 1;
                    a->animation_stream = 0x228c;
                    a->state.step = 2;
                }
            }
        }
        break;
    case 7:
        if ((s->ticks & 7) == 0 && !ActorSound(s, 0x655))
            return false;
        a->rose.phase = Wrap16((Sint64)a->rose.phase + 384);
        if (part == 11 && (a->rose.timer & 1) == 0)
        {
            Actor origin = *a;
            origin.state.priority = (Uint16)(a->state.priority + 2);
            origin.state.x += ((Sint32)(SotnCombat_Random(&s->combat_random) & 31) -
                               (a->state.facing_left ? 31 : 15)) *
                              65536;
            ShiftY(&origin, (Sint32)(SotnCombat_Random(&s->combat_random) & 31) - 16);
            if (!ActorExplosion(s, &origin, 1))
                return false;
        }
        if (!a->rose.timer)
            return SDL_SetError("Stone Rose destruction timer underflow");
        if (--a->rose.timer == 0)
        {
            ++a->state.step;
            a->rose.timer = (Uint16)((12 - part) * 2);
        }
        break;
    case 8:
        if (!a->rose.timer)
            return SDL_SetError("Stone Rose segment timer underflow");
        if (--a->rose.timer == 0)
        {
            if (part == 11 && !ActorSound(s, 0x698))
                return false;
            ++a->state.step;
            if (part != 0)
            {
                ShiftY(a, 8);
                return ActorReplaceExplosion(s, a, 1);
            }
            if (!ActorRoseSpawn(s, a, 2) || !ActorPrimitives(s, a, 1))
                return false;
            a->state.animation_set = 0;
            a->state.frame = 0;
            a->rose.timer = 24;
            if (!ActorSprite(s, a, 0, a->state.x, a->state.y, 0, 192, 63, 63, 0x1a, 0x15f))
                return false;
            ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
            if (p == NULL)
                return false;
            p->state.width = p->state.height = 0;
            p->state.blend = p->state.modulate = true;
            return true;
        }
        break;
    case 9: {
        if (!a->rose.timer)
            return SDL_SetError("Stone Rose blast timer underflow");
        if (--a->rose.timer == 0)
        {
            a->remove = true;
            return true;
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (p == NULL)
            return false;
        if (p->state.blue >= 9)
            p->state.blue = (Uint8)(p->state.blue - 6);
        p->state.red = p->state.green = p->state.blue;
        a->rose.radius = (Uint16)(a->rose.radius + 4);
        p->state.width = p->state.height = (Uint16)(a->rose.radius * 2);
        p->state.x = a->state.x;
        p->state.y = a->state.y;
        return true;
    }
    default:
        return SDL_SetError("Stone Rose entered invalid state %u", a->state.step);
    }
    if (a->state.step < 7 && part != 0 && part != 11)
        a->state.hitbox_state = ((part ^ s->ticks) & 1) ? 0 : 3;
    return Wave(s, a);
}
