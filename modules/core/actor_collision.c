// SPDX-License-Identifier: AGPL-3.0-or-later
// Actor floor, ledge, and field sensors from st/st_common.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Sensor tables remain private.
#include "actors_internal.h"
bool ActorGround(SotnActors *actors, Actor *actor, size_t offset, Uint16 *result)
{
    SotnDataView sensors = {0};
    if (result == NULL || !Slice(actors->nz0, offset, 16, &sensors))
        return false;
    *result = 0;
    actor->state.x += actor->vx;
    actor->state.y += actor->vy;
    if (actor->vy < 6 * 65536)
        actor->vy += 16384;
    if (actor->vy >= 0)
    {
        Sint32 x = Pixel(actor->state.x), y = Pixel(actor->state.y);
        for (Uint32 i = 0; i < 4; ++i)
        {
            x = Wrap16((Sint64)x + Signed16(sensors.bytes + i * 4));
            y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
            SotnCollider c = {0}, back = {0};
            if (!ActorQuery(actors, x, y, &c))
                return false;
            if ((c.effects & SOTN_EFFECT_SLOPE_FLOOR) && i == 1)
            {
                if (c.effects & SOTN_EFFECT_SOLID)
                {
                    if (!ActorQuery(actors, x, y - 8, &back))
                        return false;
                    if (!(back.effects & SOTN_EFFECT_SOLID))
                    {
                        ShiftY(actor, 4 + c.top);
                        actor->vx = actor->vy = 0;
                        actor->state.flags &= ~0x10000000U;
                        *result = 1;
                        return true;
                    }
                }
                continue;
            }
            if ((c.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK)) && i != 1)
            {
                if (c.effects & SOTN_EFFECT_SINK)
                {
                    actor->state.flags &= ~0x10000000U;
                    *result = 4;
                    return true;
                }
                if (!ActorQuery(actors, x, y - 8, &back))
                    return false;
                if (!(back.effects & SOTN_EFFECT_SOLID))
                {
                    ShiftY(actor, c.top);
                    actor->vx = actor->vy = 0;
                    actor->state.flags &= ~0x10000000U;
                    *result = 1;
                    return true;
                }
            }
        }
    }
    actor->state.flags |= 0x10000000U;
    return true;
}
bool ActorWalk(SotnActors *actors, Actor *actor, size_t offset, Uint16 *result)
{
    SotnDataView sensors = {0};
    SotnCollider c = {0};
    if (result == NULL || !Slice(actors->nz0, offset, 8, &sensors))
        return false;
    *result = 0;
    actor->state.x += actor->vx;
    ShiftY(actor, 3);
    Sint32 x = Wrap16((Sint64)Pixel(actor->state.x) + Signed16(sensors.bytes));
    Sint32 y = Wrap16((Sint64)Pixel(actor->state.y) + Signed16(sensors.bytes + 2));
    if (!ActorQuery(actors, x, y, &c))
        return false;
    if (!(c.effects & SOTN_EFFECT_SOLID))
        return true;
    ShiftY(actor, c.top);
    if (actor->vx == 0)
    {
        *result = 1;
        return true;
    }
    const Sint32 dx = Signed16(sensors.bytes + 4);
    x = Wrap16((Sint64)x + (actor->vx < 0 ? -dx : dx));
    y = Wrap16((Sint64)y + Signed16(sensors.bytes + 6) - 7);
    if (!ActorQuery(actors, x, y, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        if ((c.effects & SOTN_EFFECT_SLOPE_FLOOR) || !(c.effects & SOTN_EFFECT_SIDE))
        {
            *result = 0x61;
            return true;
        }
        actor->state.x -= actor->vx;
        actor->vx = 0;
        *result = 0xff;
        return true;
    }
    y = Wrap16((Sint64)y + 15);
    if (!ActorQuery(actors, x, y, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        *result = (c.effects & SOTN_EFFECT_SLOPE_FLOOR) ? 0x61 : 1;
        return true;
    }
    actor->state.x -= actor->vx;
    actor->vx = 0;
    *result = 0x80;
    return true;
}
bool ActorField(SotnActors *actors, Actor *actor, size_t offset, Uint16 count, bool resolve,
                Uint16 *result)
{
    SotnDataView sensors = {0};
    if (result == NULL || count > 16 || !Slice(actors->nz0, offset, (size_t)count * 4, &sensors))
        return false;
    *result = 0;
    // The PS1 helper has no return when velocity is zero; define its no-collision result.
    if (actor->vx == 0)
        return true;
    Sint32 x = Pixel(actor->state.x), y = Pixel(actor->state.y);
    for (Uint32 i = 0; i < count; ++i)
    {
        const Sint32 dx = Signed16(sensors.bytes + i * 4);
        x = Wrap16((Sint64)x + (actor->vx < 0 ? dx : -dx));
        y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
        SotnCollider c = {0};
        if (!ActorQuery(actors, x, y, &c))
            return false;
        if ((c.effects & SOTN_EFFECT_SIDE) && (!(c.effects & SOTN_EFFECT_SLOPE_FLOOR) || i != 0))
        {
            *result = 2;
            if (resolve)
            {
                const Sint32 px =
                    Wrap16((Sint64)Pixel(actor->state.x) + (actor->vx < 0 ? c.right : c.left));
                actor->state.x = (Sint32)((Sint64)px * 65536 + ((Uint32)actor->state.x & 65535));
            }
            return true;
        }
    }
    return true;
}
bool ActorSpawn(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters, Uint32 begin,
                Uint32 end, Actor **out)
{
    if (actors == NULL || source == NULL || out == NULL || id == 0 ||
        id >= SotnStageProfile_Get(actors->stage_id)->actor_limit || begin > end ||
        end > SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Actor factory requires bounded stage slots and an original entity ID");
    *out = NULL;
    for (Uint32 i = begin; i < end; ++i)
    {
        SotnActorPlacement placement = {0};
        Uint32 generation = 0;
        if (!SotnRoomActors_Get(actors->spawns, i, &placement, &generation))
            return false;
        Actor *p = &actors->slots[i];
        if (generation != 0 || p->state.generation != 0)
            continue;
        if (actors->effect_serial >= 0x7fffffff)
            return SDL_SetError("Actor generation capacity exhausted");
        *p = (Actor){.state = {.x = source->state.x,
                               .y = source->state.y,
                               .generation = 0x80000000U | ++actors->effect_serial,
                               .id = id,
                               .parameters = parameters,
                               .facing_left = source->state.facing_left},
                     .slot = (Uint16)i,
                     .dynamic = true};
        *out = p;
        return true;
    }
    return true;
}

bool ActorCrawl(SotnActors *actors, Actor *actor, Uint8 arg0, Uint8 *out)
{
    SotnCollider collider = {0};
    Uint8 bits_45 = 0;
    Uint8 bits_23 = 0;
    Uint8 bits_67 = 0;
    Uint32 collEff = 0;
    Sint32 posX = 0;
    Sint32 posY = 0;
    Uint8 bits_01 = 0;

    actor->state.x += actor->vx;
    actor->state.y += actor->vy;
    bits_67 = 0;
    bits_23 = 0;
    bits_45 = 0;
    collEff = 0;
    bits_01 = arg0 & 3;
    switch (bits_01)
    {
    case 0:
        ShiftY(actor, 3);
        posX = Pixel(actor->state.x);
        posY = Pixel(actor->state.y);
        if (!ActorQuery(actors, posX, posY, &collider))
            return false;
        if (collider.effects != SOTN_EFFECT_NONE)
        {
            collEff = collider.effects;
            ShiftY(actor, collider.top);
            posX = Pixel(actor->state.x);
            posY = Pixel(actor->state.y) - 4;
            if (!ActorQuery(actors, posX, posY, &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SIDE)
            {
                bits_67 = 0x40;
                if (actor->vx > 0)
                {
                    bits_01 = 2;
                }
                else
                {
                    bits_01 = 3;
                    actor->vx = -actor->vx;
                }
                actor->vy = -actor->vx;
                actor->vx = 0;
            }
        }
        else
        {
            bits_67 = 0x80;
            actor->state.x -= actor->vx;
            if (actor->vx > 0)
            {
                bits_01 = 3;
            }
            else
            {
                bits_01 = 2;
                actor->vx = -actor->vx;
            }
            actor->vy = actor->vx;
            actor->vx = 0;
        }
        break;

    case 1:
        ShiftY(actor, -(3));
        posX = Pixel(actor->state.x);
        posY = Pixel(actor->state.y);
        if (!ActorQuery(actors, posX, posY, &collider))
            return false;
        if (collider.effects != SOTN_EFFECT_NONE)
        {
            collEff = collider.effects;
            ShiftY(actor, collider.bottom);
            posX = Pixel(actor->state.x);
            posY = Pixel(actor->state.y) + 4;
            if (!ActorQuery(actors, posX, posY, &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SIDE)
            {
                bits_67 = 0x40;
                if (actor->vx > 0)
                {
                    bits_01 = 2;
                }
                else
                {
                    bits_01 = 3;
                    actor->vx = -actor->vx;
                }
                actor->vy = actor->vx;
                actor->vx = 0;
            }
        }
        else
        {
            bits_67 = 0x80;
            actor->state.x -= actor->vx;
            if (actor->vx > 0)
            {
                bits_01 = 3;
            }
            else
            {
                bits_01 = 2;
                actor->vx = -actor->vx;
            }
            actor->vy = -actor->vx;
            actor->vx = 0;
        }
        break;

    case 2:
        actor->state.x = (Sint32)((Sint64)Wrap16((Sint64)Pixel(actor->state.x) + 3) * 65536 +
                                  ((Uint32)actor->state.x & 65535));
        posX = Pixel(actor->state.x);
        posY = Pixel(actor->state.y);
        if (!ActorQuery(actors, posX, posY, &collider))
            return false;
        if (collider.effects != SOTN_EFFECT_NONE)
        {
            collEff = collider.effects;
            actor->state.x =
                (Sint32)((Sint64)Wrap16((Sint64)Pixel(actor->state.x) + collider.left) * 65536 +
                         ((Uint32)actor->state.x & 65535));
            posX = Pixel(actor->state.x) - 4;
            posY = Pixel(actor->state.y);
            if (!ActorQuery(actors, posX, posY, &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SOLID)
            {
                bits_67 = 0x40;
                if (actor->vy > 0)
                {
                    bits_01 = 0;
                }
                else
                {
                    bits_01 = 1;
                    actor->vy = -actor->vy;
                }
                actor->vx = -actor->vy;
                actor->vy = 0;
            }
        }
        else
        {
            bits_67 = 0x80;
            actor->state.y -= actor->vy;
            if (actor->vy > 0)
            {
                bits_01 = 1;
            }
            else
            {
                bits_01 = 0;
                actor->vy = -actor->vy;
            }
            actor->vx = actor->vy;
            actor->vy = 0;
        }
        break;

    case 3:
        actor->state.x = (Sint32)((Sint64)Wrap16((Sint64)Pixel(actor->state.x) - (3)) * 65536 +
                                  ((Uint32)actor->state.x & 65535));
        posX = Pixel(actor->state.x);
        posY = Pixel(actor->state.y);
        if (!ActorQuery(actors, posX, posY, &collider))
            return false;
        if (collider.effects != SOTN_EFFECT_NONE)
        {
            collEff = collider.effects;
            actor->state.x =
                (Sint32)((Sint64)Wrap16((Sint64)Pixel(actor->state.x) + collider.right) * 65536 +
                         ((Uint32)actor->state.x & 65535));
            posX = Pixel(actor->state.x) + 4;
            posY = Pixel(actor->state.y);
            if (!ActorQuery(actors, posX, posY, &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SOLID)
            {
                bits_67 = 0x40;
                if (actor->vy > 0)
                {
                    bits_01 = 0;
                }
                else
                {
                    bits_01 = 1;
                    actor->vy = -actor->vy;
                }
                actor->vx = actor->vy;
                actor->vy = 0;
            }
        }
        else
        {
            bits_67 = 0x80;
            actor->state.y -= actor->vy;
            if (actor->vy > 0)
            {
                bits_01 = 1;
            }
            else
            {
                bits_01 = 0;
                actor->vy = -actor->vy;
            }
            actor->vx = -actor->vy;
            actor->vy = 0;
        }
    }

    if (collEff & SOTN_EFFECT_SLOPE_FLOOR)
    {
        bits_23 = 4;
    }
    if (collEff & SOTN_EFFECT_SLOPE_HALF)
    {
        bits_23 = 8;
    }
    if (collEff & SOTN_EFFECT_SLOPE_QUARTER)
    {
        bits_23 = 0xC;
    }
    if (collEff & SOTN_EFFECT_SLOPE_CEILING)
    {
        bits_45 = 0x20;
    }
    if (collEff & SOTN_EFFECT_SLOPE_LEFT)
    {
        bits_45 = 0x10;
    }
    bits_01 = (bits_45 + (bits_23 + (bits_01 + bits_67)));
    *out = bits_01;
    return true;
}

bool ActorSpawnAt(SotnActors *s, const Actor *source, Uint16 id, Uint16 parameters, Uint32 slot,
                  Actor **out)
{
    if (s == NULL || source == NULL || out == NULL || slot >= SOTN_STAGE_ACTOR_SLOTS || id == 0 ||
        id >= SotnStageProfile_Get(s->stage_id)->actor_limit)
        return SDL_SetError("Fixed actor factory slot or entity ID is invalid");
    Actor *p = &s->slots[slot];
    SotnActorPlacement placement = {0};
    Uint32 generation = 0;
    if (!SotnRoomActors_Get(s->spawns, slot, &placement, &generation))
        return false;
    if (p->state.generation != 0 || generation != 0)
        return SDL_SetError("Required actor child slot %u is occupied", slot);
    if (s->effect_serial >= 0x7fffffff)
        return SDL_SetError("Actor generation capacity exhausted");
    *p = (Actor){.state = {.x = source->state.x,
                           .y = source->state.y,
                           .generation = 0x80000000U | ++s->effect_serial,
                           .id = id,
                           .parameters = parameters,
                           .facing_left = source->state.facing_left},
                 .slot = (Uint16)slot,
                 .dynamic = true};
    p->enemy_state.owner_slot = source->slot;
    p->enemy_state.owner_generation = source->state.generation;
    *out = p;
    return true;
}
