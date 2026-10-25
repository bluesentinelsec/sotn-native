// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected normal-castle st/e_stairway.h at the pinned sotn-decomp reference.
#include "actors_internal.h"
static bool Explosion(SotnActors *s, const Actor *a, bool intense, Uint16 kind, Sint16 dx,
                      Sint16 dy)
{
    Actor *effect = NULL;
    if (!ActorSpawn(s, a, intense ? 6 : 2, kind, 160, 192, &effect))
        return false;
    if (effect != NULL)
    {
        effect->state.x = (Pixel(a->state.x) + dx) * 65536;
        effect->state.y = (Pixel(a->state.y) + dy) * 65536;
        effect->state.priority = (Uint16)(a->state.priority + 1);
        effect->state.facing_left = false;
    }
    return true;
}
static bool Fragment(SotnActors *s, Actor *a)
{
    const Uint16 frame = a->state.parameters & 15;
    if (frame > 2)
        return SDL_SetError("Entrance stair debris variant exceeds its original factory");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xb18, 0xaa8)))
            return false;
        a->state.frame = (Uint16)(frame + 31);
        a->state.rotated = true;
        a->state.scale_x = a->state.scale_y = 96;
        const Sint32 speed = (Sint32)(ActorRandom(s) & 31) + 16;
        const Sint32 angle = (Sint32)ActorRandom(s) * 6 + 0x900;
        a->vx = speed * ActorSin(angle + 1024);
        a->vy = speed * ActorSin(angle);
        if (a->vx > 0)
            a->state.facing_left = true;
        return true;
    }
    if (a->state.step != 1)
        return SDL_SetError("Entrance stair debris entered an invalid state");
    a->state.x += a->vx;
    a->state.y += a->vy;
    a->vy += 8192;
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation - 32);
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + 8, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 6, frame == 0 ? 0x13 : 0x10, 160, 192, &effect))
            return false;
        if (effect != NULL)
        {
            effect->state.x = Pixel(a->state.x) * 65536;
            effect->state.y = Pixel(a->state.y) * 65536;
            effect->state.facing_left = false;
        }
        a->remove = true;
    }
    return true;
}
static bool Tile(SotnActors *s, bool broken)
{
    const Uint16 first = broken ? 0x3ee : 0x408, second = broken ? 0x3d2 : 0x40d;
    if (0x539U >= (Uint64)s->map.width * s->map.height || first >= s->map.types.size ||
        second >= s->map.types.size)
        return SDL_SetError("Entrance stair tile changes exceed the original room definitions");
    return ActorTile(s, 0x4d9, first) && ActorTile(s, 0x539, second);
}
static bool StartPiece(SotnActors *s, Actor *a)
{
    if (s->tile_pages.size <= 0x409 || s->tile_graphics.size <= 0x409 ||
        s->tile_palettes.size <= 0x409 || s->tile_pages.bytes == NULL ||
        s->tile_graphics.bytes == NULL || s->tile_palettes.bytes == NULL)
        return SDL_SetError("Entrance stair debris needs its borrowed original tile artwork");
    if (!ActorPrimitives(s, a, 16))
        return false;
    const Uint8 packed = s->tile_graphics.bytes[0x409];
    const Uint8 u = (Uint8)((packed & 15) * 16), v = packed & 0xf0;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    p->state = (SotnActorPrimitive){.x = a->state.x,
                                    .y = a->state.y,
                                    .page = (Uint16)(s->tile_pages.bytes[0x409] + 8),
                                    .palette = s->tile_palettes.bytes[0x409],
                                    .u = {u, (Uint8)(u + 15), u, (Uint8)(u + 15)},
                                    .v = {v, v, (Uint8)(v + 15), (Uint8)(v + 15)},
                                    .active = true,
                                    .textured = true,
                                    .quad = true};
    p->vx = p->vy = -65536;
    ++a->state.step;
    return true;
}
static bool MovePiece(SotnActors *s, Actor *a)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    a->stair.angle = (Sint16)Wrap16((Sint64)a->stair.angle - 32);
    p->vy += 8192;
    p->state.x += p->vx;
    p->state.y += p->vy;
    const Sint32 sine = ActorSin(a->stair.angle), cosine = ActorSin(a->stair.angle + 1024);
    for (Uint32 i = 0; i < 4; ++i)
    {
        const Sint32 x = i & 1 ? 8 : -8, y = i & 2 ? 8 : -8;
        p->state.offset_x[i] = (Sint16)ActorShift((Sint64)x * cosine - (Sint64)y * sine, 12);
        p->state.offset_y[i] = (Sint16)ActorShift((Sint64)x * sine + (Sint64)y * cosine, 12);
    }
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(p->state.x), Pixel(p->state.y) + 8, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        a->state.x = Pixel(p->state.x) * 65536;
        a->state.y = (Pixel(p->state.y) - 4) * 65536;
        ++a->state.step;
    }
    return true;
}
bool ActorEntranceStairway(SotnActors *s, Actor *a)
{
    if (s->stage_id != 65 && s->stage_id != 7)
        return SDL_SetError("Entrance stair behavior requires its own NO3/NP3 profile");
    if (a->state.id == (s->stage_id == 65 ? 0x5d : 0x4c))
        return Fragment(s, a);
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xadc, 0xa6c)))
            return false;
        a->state.hitbox_width = a->state.hitbox_height = 8;
        a->state.x = 1432 * 65536;
        a->state.y = 200 * 65536;
        a->state.hit_points = 16;
        const bool broken = s->world->flags[SOTN_ENTRANCE_STAIRS] != 0;
        if (!Tile(s, broken))
            return false;
        a->state.hitbox_state = broken ? 0 : 2;
        if (broken)
            a->state.step = 32;
        break;
    }
    case 1:
        if (a->hit_flags && !ActorSound(s, 0x64b))
            return false;
        if (a->state.flags & 0x100)
            ++a->state.step;
        break;
    case 2: {
        if (!ActorSound(s, 0x644) || !Tile(s, true))
            return false;
        s->world->flags[SOTN_ENTRANCE_STAIRS] = 1;
        Actor *food = NULL;
        if (!ActorSpawn(s, a, 0xa, 0x45, 96, 128, &food)) // Original turkey identity.
            return false;
        if (food != NULL)
        {
            food->state.x = Pixel(a->state.x) * 65536;
            food->state.y = Pixel(a->state.y) * 65536;
            food->state.facing_left = false;
        }
        if (!Explosion(s, a, true, 0x10, 8, 8))
            return false;
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        if (available < 16)
        {
            a->remove = true;
            return true;
        }
        if (!StartPiece(s, a))
            return false;
        SDL_FALLTHROUGH;
    }
    case 3:
        return MovePiece(s, a);
    case 4:
        if (!ActorSound(s, 0x644) || !Explosion(s, a, false, 0x11, 0, 0))
            return false;
        for (Uint32 i = 0; i < 6; ++i)
        {
            Actor *rock = NULL;
            if (!ActorSpawn(s, a, s->stage_id == 65 ? 0x5d : 0x4c, 0, 160, 192, &rock))
                return false;
            if (rock != NULL)
            {
                rock->state.parameters = (Uint16)(ActorRandom(s) & 3);
                if (rock->state.parameters == 3)
                    rock->state.parameters = 0;
                rock->state.x = Pixel(a->state.x) * 65536;
                rock->state.y = Pixel(a->state.y) * 65536;
                rock->state.facing_left = false;
            }
        }
        a->remove = true;
        break;
    case 32:
        break;
    default:
        return SDL_SetError("Entrance stairway entered an invalid original state");
    }
    return true;
}
