// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/no2_bg.h and st/e_background_house.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All model/placement/palette data stays private.
#include "actors_internal.h"
#include "boss_projection.h"

static bool Reserve(SotnActors *s, Actor *a, Uint16 count)
{
    if (ActorPrimitives(s, a, count))
        return true;
    a->remove = true;
    return true;
}
static Sint32 Winding(SotnProjectedPoint a, SotnProjectedPoint b, SotnProjectedPoint c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
static bool Emit(SotnActors *s, Actor *a, Uint16 *cursor, const SotnProjectedPoint corners[4],
                 Uint16 palette, Uint8 color, Uint8 kind)
{
    if (*cursor >= 16)
        return SDL_SetError("NO2 house projection exceeds its sixteen owned panels");
    ActorPrimitive *owned = ActorPrimitiveGet(s, a, (*cursor)++);
    if (!owned || palette >= 768)
        return SDL_SetError("NO2 house requires an owned panel and bounded palette");
    SotnActorPrimitive *p = &owned->state;
    *p = (SotnActorPrimitive){.active = true,
                              .screen_space = true,
                              .textured = true,
                              .quad = true,
                              .modulate = true,
                              .page = 15,
                              .palette = palette,
                              .red = color,
                              .green = color,
                              .blue = color,
                              .priority_set = true,
                              .priority = (Uint16)(0x58 - a->state.parameters)};
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->offset_x[i] = (Sint16)corners[i].x;
        p->offset_y[i] = (Sint16)corners[i].y;
        if (kind == 1)
        {
            p->u[i] = i == 0 || i == 3 ? 254 : 194;
            p->v[i] = i == 1 ? 108 : 172;
        }
        else
        {
            p->u[i] = (Uint8)((i & 1U) ? (kind == 2 ? 190 : 124) : (kind == 2 ? 130 : 4));
            p->v[i] = (Uint8)((i & 2U) ? (kind == 2 ? 164 : 158) : (kind == 2 ? 108 : 3));
        }
    }
    return true;
}
static bool Face(SotnActors *s, Actor *a, Uint16 *cursor, const SotnProjectedPoint points[19],
                 SotnDataView indices, const Uint8 *profile, Uint8 kind)
{
    for (size_t i = 0; i < indices.size; ++i)
        if (indices.bytes[i] >= 19)
            return SDL_SetError("NO2 house face references a vertex outside its private model");
    const SotnProjectedPoint p0 = points[indices.bytes[0]], p1 = points[indices.bytes[1]],
                             p2 = points[indices.bytes[2]], p3 = points[indices.bytes[3]];
    if (Winding(p0, p1, p2) <= 0)
        return true;
    const Uint16 palette = Read16(profile + (kind == 2 ? 6 : 0));
    const Uint16 shade = Read16(profile + (kind == 1 ? 4 : kind == 2 ? 8 : 2));
    if (shade > 255)
        return SDL_SetError("NO2 house shade exceeds original byte color bounds");
    if (kind == 2)
    {
        for (Uint16 face = 1; face < 5; ++face)
        {
            SotnProjectedPoint corners[4] = {0};
            for (Uint16 i = 0; i < 4; ++i)
                corners[i] = points[indices.bytes[face * 4U + i]];
            if (!Emit(s, a, cursor, corners, palette, (Uint8)shade, 2))
                return false;
        }
        return true;
    }
    if (kind == 0)
    {
        const SotnProjectedPoint front[4] = {p0, p1, p2, p3};
        const SotnProjectedPoint roof[4] = {
            p0, points[indices.bytes[4]], {(p0.x + p1.x) / 2, (p0.y + p1.y) / 2}, p1};
        return Emit(s, a, cursor, front, palette, (Uint8)shade, 0) &&
               Emit(s, a, cursor, roof, palette, (Uint8)shade, 1);
    }
    const SotnProjectedPoint top = {(p0.x + p1.x) / 2, (p0.y + p1.y) / 2},
                             bottom = {(p2.x + p3.x) / 2, (p2.y + p3.y) / 2};
    const SotnProjectedPoint left[4] = {p0, top, p2, bottom}, right[4] = {top, p1, bottom, p3};
    return Emit(s, a, cursor, left, palette, (Uint8)shade, 0) &&
           Emit(s, a, cursor, right, palette, (Uint8)shade, 0);
}
static bool House(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1 || a->state.step > 1)
        return SDL_SetError("NO2 house requires an original variant and initialized state");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x850) || !Reserve(s, a, 16))
            return false;
        return true;
    }
    const Sint32 x = Pixel(a->state.x) - s->camera_x;
    if (x < -512 || x > 512)
    {
        for (Uint16 i = 0; i < 16; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state.active = false;
        }
        return true;
    }
    SotnProjectedPoint points[19] = {0};
    SotnDataView profile = {0};
    if (!SotnNo2_ProjectHouse(s->executable, s->nz0, a->state.parameters, x,
                              Pixel(a->state.y) - s->camera_y, points) ||
        !Slice(s->nz0, 0xcdcU + a->state.parameters * 14U, 10, &profile))
        return false;
    Uint16 cursor = 0;
    for (Uint8 kind = 0; kind < 3; ++kind)
        for (Uint16 side = 0; side < 2; ++side)
        {
            SotnDataView indices = {0};
            const size_t count = kind == 2 ? 20 : kind == 1 ? 4 : 5;
            const size_t offset = kind == 2 ? 0xcb0 : kind == 1 ? 0xca8 : 0xc9c;
            if (!Slice(s->nz0, offset + side * count, count, &indices) ||
                !Face(s, a, &cursor, points, indices, profile.bytes, kind))
                return false;
        }
    for (; cursor < 16; ++cursor)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, cursor);
        if (!p)
            return false;
        p->state.active = false;
    }
    return true;
}
static bool SpawnHouses(SotnActors *s, Actor *a)
{
    if (a->state.step)
        return a->state.step == 1 || SDL_SetError("NO2 house spawner state is invalid");
    SotnDataView records = {0};
    if (!Slice(s->nz0, 0xcf4, 68, &records) || Signed16(records.bytes + 66) != -1)
        return SDL_SetError("NO2 house placement list has no bounded terminator");
    for (Uint16 i = 0; i < 11; ++i)
    {
        const Uint8 *p = records.bytes + i * 6U;
        if (Signed16(p) < 0 || Signed16(p + 2) < 0 || Read16(p + 4) > 1)
            return SDL_SetError("NO2 house placement has an invalid position or model");
    }
    for (Uint16 i = 0; i < 11; ++i)
    {
        const Uint8 *p = records.bytes + i * 6U;
        Actor *child = NULL;
        if (!ActorSpawn(s, a, 0x1e, Read16(p + 4), 160, 192, &child))
            return false;
        if (!child)
            break;
        child->state.x = Signed16(p) * 65536 + (Sint32)((Uint32)child->state.x & 65535U);
        child->state.y = Signed16(p + 2) * 65536 + (Sint32)((Uint32)child->state.y & 65535U);
    }
    ++a->state.step;
    return true;
}
static bool Background(SotnActors *s, Actor *a)
{
    if (a->state.step)
        return a->state.step == 1 || SDL_SetError("NO2 repeating backdrop state is invalid");
    ++a->state.step;
    if (!Reserve(s, a, a->state.parameters ? 32 : 8))
        return false;
    if (a->remove)
        return true;
    a->state.flags = 0x04000000;
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *owned = ActorPrimitiveGet(s, a, i);
        if (!owned)
            return false;
        owned->state = (SotnActorPrimitive){.active = true,
                                            .textured = true,
                                            .quad = true,
                                            .screen_space = true,
                                            .page = 15,
                                            .palette = 54,
                                            .priority = 16,
                                            .priority_set = true,
                                            .u = {128, 200, 128, 200},
                                            .v = {0, 0, 104, 104}};
        const Sint16 x = (Sint16)((i & 7U) * 64),
                     y = (Sint16)((i >> 3) * 100 + (a->state.parameters ? -64 : 88));
        owned->state.offset_x[0] = owned->state.offset_x[2] = x;
        owned->state.offset_x[1] = owned->state.offset_x[3] = (Sint16)(x + 72);
        owned->state.offset_y[0] = owned->state.offset_y[1] = y;
        owned->state.offset_y[2] = owned->state.offset_y[3] = (Sint16)(y + 108);
    }
    return true;
}
static bool Glow(SotnActors *s, Actor *a)
{
    if (a->state.step > 2 || a->no2_scene.cycle > 6)
        return SDL_SetError("NO2 courtyard glow has invalid animation state");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x874))
            return false;
        a->state.animation_set = 0x8002;
        a->state.frame = 3;
        a->state.priority = 128;
        a->state.blend_mode = 0x70;
        a->no2_scene.cycle_timer = 2;
        a->no2_scene.heat_timer = 16;
    }
    else if (a->state.step == 1)
    {
        const Sint32 dx = Pixel(a->state.x) - Pixel(s->player.x);
        if (s->camera_y >= 0x304 && dx > -128 && dx < 128)
            ++a->state.step;
    }
    else if (--a->no2_scene.heat_timer == 0)
    {
        SotnDataView rows = {0};
        if (!Slice(s->nz0, 0xb8c, 28, &rows) || s->palette_red_shift.count)
            return SDL_SetError("NO2 courtyard palette update has invalid or conflicting rows");
        SotnPaletteRedShift shift = {.count = 7};
        for (Uint16 i = 0; i < 7; ++i)
        {
            const Uint32 row = Read32(rows.bytes + i * 4U);
            if (row >= 192)
                return SDL_SetError("NO2 courtyard palette row exceeds the common bank");
            shift.palettes[i] = (Uint16)(64 + row);
        }
        s->palette_red_shift = shift;
        a->no2_scene.heat_timer = 16;
    }
    if (--a->no2_scene.cycle_timer == 0)
    {
        a->no2_scene.cycle = (Uint16)((a->no2_scene.cycle + 1) % 7);
        a->no2_scene.cycle_timer = 2;
    }
    SotnDataView palette = {0};
    if (!Slice(s->nz0, 0xb7cU + a->no2_scene.cycle * 2U, 2, &palette))
        return false;
    a->state.palette = Read16(palette.bytes);
    return true;
}
static bool Lamp(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= SOTN_STAGE_ACTOR_SLOTS || a->state.step > 1)
        return SDL_SetError("NO2 lamp requires a bounded breakable slot and state");
    const Actor *candle = &s->slots[a->state.parameters];
    const bool missing = !candle->state.generation || candle->state.id != 1;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x874))
            return false;
        a->state.animation_set = 0x8002;
        a->state.priority = 128;
    }
    else if (!a->no2_scene.lamp_missing && missing)
    {
        a->pose = a->remaining = 0;
        for (Uint16 i = 0; i < 5; ++i)
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 6, 16, 160, 192, &effect))
                return false;
            if (effect)
            {
                const Sint32 dx = (Sint32)(ActorRandom(s) & 15U) - 8,
                             dy = (Sint32)(ActorRandom(s) & 15U) - 8;
                effect->state.x = Wrap16((Sint64)Pixel(effect->state.x) + dx) * 65536 +
                                  (Sint32)((Uint32)effect->state.x & 65535U);
                ShiftY(effect, dy);
            }
        }
    }
    Uint8 result = 0;
    if (!ActorAnimate(s, a, missing ? 0xbb4 : 0xba8, &result))
        return false;
    a->no2_scene.lamp_missing = missing;
    return true;
}
bool ActorNo2Scene(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4)
        return SDL_SetError("NO2 scenery requires its original stage owner");
    switch (a->state.id)
    {
    case 0x19:
        return Background(s, a);
    case 0x1a:
        return Glow(s, a);
    case 0x1b:
        if (!a->state.step)
        {
            if (!ActorInitialize(s, a, 0x874))
                return false;
            a->state.animation_set = 0x8002;
            a->state.frame = 1;
            a->state.priority = 160;
        }
        return a->state.step == 1 || SDL_SetError("NO2 foreground state is invalid");
    case 0x1c:
        a->state.step = 1;
        s->backdrop_color = 4 | (3U << 5) | (5U << 10);
        return true;
    case 0x1d:
        return Lamp(s, a);
    case 0x1e:
        return House(s, a);
    case 0x1f:
        return SpawnHouses(s, a);
    default:
        return SDL_SetError("NO2 scenery received an unrelated actor identity");
    }
}
