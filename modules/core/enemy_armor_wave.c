// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_armor_lord.h fire-wave helpers at the pinned reference. Runtime artwork is private.
#include "actors_internal.h"

static ActorPrimitive *Pair(SotnActors *s, Actor *a, Uint16 *index)
{
    for (Uint16 i = 0; i < a->state.primitive_count; i = (Uint16)(i + 2))
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return NULL;
        if (!p->armor_wave.used)
        {
            const Uint32 generation = p->armor_wave.generation + 1U;
            p->state = (SotnActorPrimitive){0};
            p->phase = 0;
            SDL_memset(&p->armor_wave, 0, sizeof(p->armor_wave));
            p->armor_wave.generation = generation ? generation : 1;
            p->armor_wave.used = true;
            *index = i;
            return p;
        }
    }
    return NULL;
}
static bool DamageBox(SotnActors *s, Actor *a)
{
    const Uint16 owner = a->enemy_state.owner_slot;
    if (owner >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Armor Lord wave hitbox owner exceeds the actor slots");
    const Actor *parent = &s->slots[owner];
    if (parent->state.id != 0x4c || parent->state.generation != a->enemy_state.owner_generation ||
        parent->remove || a->armor.pair >= parent->state.primitive_count)
    {
        a->remove = true;
        return true;
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, parent, a->armor.pair);
    if (p == NULL)
        return false;
    if (!p->armor_wave.used || p->armor_wave.generation != a->armor.pair_generation)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        const Uint16 height = a->state.hitbox_height;
        const Sint16 offset = a->state.hitbox_y;
        if (!ActorInitialize(s, a, 0xaf4))
            return false;
        a->state.hitbox_width = 8;
        a->state.hitbox_x = 8;
        a->state.hitbox_height = height;
        a->state.hitbox_y = offset;
    }
    if (a->state.step++ > 5)
    {
        p->armor_wave.damage = false;
        a->remove = true;
    }
    return true;
}
static bool Smoke(SotnActors *s, ActorPrimitive *p)
{
    SotnActorPrimitive *q = &p->state;
    if (!p->phase)
    {
        const Uint8 v = p->armor_wave.branches ? 0 : 40;
        const Sint16 x = (Sint16)(q->offset_x[0] - 8), y = (Sint16)(q->offset_y[0] - 8);
        *q = (SotnActorPrimitive){.width = 16,
                                  .height = 16,
                                  .page = 0x1a,
                                  .palette = 0x160,
                                  .u = {240, 255, 240, 255},
                                  .v = {v, v, (Uint8)(v + 15), (Uint8)(v + 15)},
                                  .offset_x = {x, (Sint16)(x + 16), x, (Sint16)(x + 16)},
                                  .offset_y = {y, y, (Sint16)(y + 16), (Sint16)(y + 16)},
                                  .red = 160,
                                  .green = 160,
                                  .blue = 160,
                                  .active = true,
                                  .textured = true,
                                  .blend = true,
                                  .modulate = true,
                                  .quad = true};
        ++p->phase;
    }
    else
    {
        if (s->ticks % 4 == 0)
            for (Uint32 i = 0; i < 4; ++i)
                ++q->offset_y[i];
        q->red = (Uint8)(q->red - 2);
        q->green = q->blue = q->red;
        if (q->red < 16)
        {
            p->armor_wave.used = false;
            q->active = false;
            p->phase = 0;
        }
    }
    return true;
}
static bool Wave(SotnActors *s, Actor *a, ActorPrimitive *p, Uint16 index)
{
    SotnActorPrimitive *q = &p->state;
    if (s->ticks % 3 == 0)
    {
        const Uint8 u = q->u[0];
        q->u[0] = q->u[2] = q->u[1];
        q->u[1] = q->u[3] = u;
    }
    if (!p->phase)
    {
        q->offset_x[0] = q->offset_x[2];
        q->offset_x[1] = q->offset_x[3];
        q->offset_y[0] = q->offset_y[1] = q->offset_y[2];
        q->width = q->height = 1;
        q->page = 0x14;
        q->palette = 0x220;
        q->u[0] = q->u[2] = 224;
        q->u[1] = q->u[3] = 239;
        for (Uint32 i = 0; i < 4; ++i)
            q->v[i] = 208;
        q->red = q->green = q->blue = 112;
        q->active = q->textured = q->blend = q->modulate = q->quad = true;
        ++p->phase;
    }
    else if (p->phase == 1)
    {
        q->v[2] = q->v[3] = (Uint8)(q->v[2] + 2);
        q->offset_y[0] = (Sint16)(q->offset_y[0] - 2);
        q->offset_y[1] = q->offset_y[0];
        q->offset_x[3] = (Sint16)(q->offset_x[3] + (a->state.facing_left ? 1 : -1));
        q->offset_x[1] = q->offset_x[3];
        SotnCollider collider = {0};
        if (!ActorQuery(s, q->offset_x[3], q->offset_y[3] + 1, &collider))
            return false;
        if (!(collider.effects & SOTN_EFFECT_SOLID))
        {
            p->armor_wave.branches = 0;
            ++p->phase;
        }
        else
        {
            Uint16 next = 0;
            if (q->v[2] == 224 && p->armor_wave.branches)
            {
                ActorPrimitive *child = Pair(s, a, &next);
                if (child != NULL)
                {
                    child->armor_wave.branches = (Uint8)(p->armor_wave.branches - 1);
                    const Sint16 x = (Sint16)(q->offset_x[3] + (a->state.facing_left ? -8 : 8));
                    child->state.offset_x[2] = x;
                    child->state.offset_x[3] = (Sint16)(x + (a->state.facing_left ? 16 : -16));
                    child->state.offset_y[2] = child->state.offset_y[3] = q->offset_y[3];
                }
            }
            if (q->v[2] > 253)
            {
                ActorPrimitive *child = Pair(s, a, &next);
                if (child != NULL)
                {
                    child->armor_wave.smoke = true;
                    child->armor_wave.branches = (Uint8)(ActorRandom(s) & 1U);
                    child->state.offset_x[0] =
                        (Sint16)(q->offset_x[1] + (a->state.facing_left ? -16 : 16));
                    child->state.offset_y[0] = (Sint16)(q->offset_y[1] + 16);
                }
                ++p->phase;
            }
        }
    }
    else if (p->phase == 2)
    {
        q->v[2] = q->v[3] = (Uint8)(q->v[2] - 1);
        ++q->offset_y[0];
        q->offset_y[1] = q->offset_y[0];
        q->green = (Uint8)(q->green - 4);
        q->blue = (Uint8)(q->blue - 2);
        if (q->v[2] < 210)
        {
            q->active = false;
            p->armor_wave.used = false;
            p->phase = 0;
        }
    }
    else
        return SDL_SetError("Armor Lord fire wave entered an invalid phase");
    const Sint32 dx = a->state.facing_left ? q->offset_x[3] - Pixel(s->player.x)
                                           : Pixel(s->player.x) - q->offset_x[3];
    if (dx > 0 && dx < 16 && !p->armor_wave.damage)
    {
        Actor *hit = NULL;
        if (!ActorSpawn(s, a, 0x4d, 0, 96, 128, &hit))
            return false;
        if (hit != NULL)
        {
            const Sint32 height = (q->offset_y[2] - q->offset_y[0]) / 2;
            if (height < 0 || height > 64)
                return SDL_SetError("Armor Lord fire-wave height exceeds its original range");
            hit->state.x = q->offset_x[3] * 65536;
            hit->state.y = q->offset_y[0] * 65536;
            hit->state.hitbox_height = (Uint16)height;
            hit->state.hitbox_y = (Sint16)(height + 8);
            hit->enemy_state.owner_slot = a->slot;
            hit->enemy_state.owner_generation = a->state.generation;
            hit->armor.pair = index;
            hit->armor.pair_generation = p->armor_wave.generation;
            p->armor_wave.damage = true;
        }
    }
    return true;
}

bool ActorArmorWave(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x4d)
        return DamageBox(s, a);
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x968))
            return false;
        if (!ActorPrimitives(s, a, 26))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        a->timer = 0;
        Uint16 index = 0;
        ActorPrimitive *p = Pair(s, a, &index);
        if (p == NULL)
            return SDL_SetError("Armor Lord wave failed to reserve its first primitive pair");
        const Sint16 x = (Sint16)Pixel(a->state.x), y = (Sint16)(Pixel(a->state.y) + 40);
        p->state.offset_x[2] = x;
        p->state.offset_x[3] = (Sint16)(x + (a->state.facing_left ? 32 : -32));
        p->state.offset_y[2] = p->state.offset_y[3] = y;
        p->armor_wave.branches = 5;
    }
    for (Uint16 i = 0; i < a->state.primitive_count; i = (Uint16)(i + 2))
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        if (p->armor_wave.used && !(p->armor_wave.smoke ? Smoke(s, p) : Wave(s, a, p, i)))
            return false;
    }
    if (a->timer++ > 256)
        a->remove = true;
    return true;
}
