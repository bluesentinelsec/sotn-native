// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_clouds.h and st/e_background_sky_land.h, sotn-decomp 7bc54d8157b6.
// Original UVs, plane models, grids and profiles remain private runtime inputs.
#include "actors_internal.h"
#include "boss_projection.h"

static bool Initialize(SotnActors *s, Actor *a, Uint16 count)
{
    SotnCloudProfile profile = {0};
    if (!SotnCloud_GetProfile(s->stage_id, &profile) || !ActorInitialize(s, a, profile.initialize))
        return false;
    if (!ActorPrimitives(s, a, count))
    {
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    return true;
}
static bool Land(SotnActors *s, Actor *a)
{
    if (a->state.step)
        return a->state.step == 1 || SDL_SetError("Chapel sky land entered an invalid state");
    if (!Initialize(s, a, 5))
        return false;
    if (a->remove)
        return true;
    for (Uint16 i = 0; i < 5; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.active = true,
                                        .textured = true,
                                        .screen_space = true,
                                        .quad = true,
                                        .page = 15,
                                        .palette = 0xc5,
                                        .priority_set = true,
                                        .priority = 16};
        p->state.u[0] = p->state.u[2] = 65;
        p->state.u[1] = p->state.u[3] = 127;
        p->state.v[0] = p->state.v[1] = 169;
        p->state.v[2] = p->state.v[3] = 198;
        p->state.offset_x[0] = p->state.offset_x[2] = (Sint16)(i * 62);
        p->state.offset_x[1] = p->state.offset_x[3] = (Sint16)((i + 1U) * 62);
        p->state.offset_y[0] = p->state.offset_y[1] = 192;
        p->state.offset_y[2] = p->state.offset_y[3] = 228;
    }
    return true;
}
static bool Emit(SotnActors *s, Actor *a, Uint16 *used, Uint8 kind, Uint16 priority,
                 Sint32 distance, const SotnProjectedPoint point[4])
{
    SotnDataView uv = {0};
    SotnCloudProfile profile = {0};
    ActorPrimitive *p = ActorPrimitiveGet(s, a, *used);
    if (!p || !SotnCloud_GetProfile(s->stage_id, &profile) ||
        !Slice(s->nz0, profile.uv + kind * 8U, 8, &uv))
        return false;
    p->state = (SotnActorPrimitive){.active = true,
                                    .textured = true,
                                    .screen_space = true,
                                    .quad = true,
                                    .page = 15,
                                    .palette = 0xc0,
                                    .priority_set = true,
                                    .priority = priority,
                                    .gouraud = true,
                                    .modulate = true};
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->state.u[i] = uv.bytes[i * 2U];
        p->state.v[i] = uv.bytes[i * 2U + 1];
        p->state.offset_x[i] = (Sint16)point[i].x;
        p->state.offset_y[i] = (Sint16)point[i].y;
        const bool far = i >= 2;
        p->state.vertex_red[i] =
            (Uint8)ActorShift(524288 - (Sint64)distance * 160 - (far ? 40960 : 0), 12);
        p->state.vertex_green[i] =
            (Uint8)ActorShift(524288 - (Sint64)distance * 176 - (far ? 45056 : 0), 12);
        p->state.vertex_blue[i] =
            (Uint8)ActorShift(524288 - (Sint64)distance * 128 - (far ? 32768 : 0), 12);
    }
    ++*used;
    return true;
}
static bool Band(SotnActors *s, Actor *a, SotnDataView row, Sint32 x, Sint32 height, Uint16 depth,
                 Uint16 priority, Uint16 *used)
{
    bool populated = false;
    for (Uint16 i = 0; i < 8; ++i)
    {
        if (row.bytes[i] > 6)
            return SDL_SetError("Chapel cloud grid references an invalid private UV profile");
        populated = populated || row.bytes[i] != 0;
    }
    if (!populated)
        return SDL_SetError("Chapel cloud row has no tile capable of terminating its scan");
    Sint32 position = x, direction = -1;
    Uint16 column = 0;
    for (Uint16 scan = 0; scan < 512; ++scan)
    {
        position += direction * 256;
        column = (Uint16)(((Sint32)column + direction) & 7);
        const Uint8 kind = row.bytes[column];
        if (!kind)
            continue;
        SotnProjectedPoint point[4] = {0};
        if (!SotnCloud_Project(s->nz0, s->stage_id, position, height, depth, point))
            return false;
        if (point[3].y < 0)
            return true;
        if (point[3].x < 0)
        {
            if (direction < 0)
            {
                direction = 1;
                position = x - 256;
                column = 7;
            }
            continue;
        }
        if (point[2].x > 256)
        {
            if (direction > 0)
                return true;
            continue;
        }
        if (!Emit(s, a, used, kind, priority, (Sint32)depth - 448, point))
            return false;
        if (*used == 96)
            return true;
    }
    return SDL_SetError("Chapel cloud scan did not reach a bounded screen edge");
}
static bool Clouds(SotnActors *s, Actor *a)
{
    SotnCloudProfile tables = {0};
    if (!SotnCloud_GetProfile(s->stage_id, &tables))
        return false;
    if (!a->state.step && !Initialize(s, a, 96))
        return false;
    if (a->remove)
        return true;
    if (a->state.step != 1 || a->state.primitive_count != 96)
        return SDL_SetError("Chapel clouds entered an invalid state or primitive reservation");
    a->clouds.x[0] += 65536U;
    a->clouds.x[1] += 131072U;
    a->clouds.y[0] += 327680U;
    a->clouds.y[1] += 196608U;
    a->clouds.y[2] += 327680U;
    s->backdrop_color = 6U | (5U << 5) | (8U << 10);
    for (Uint16 i = 0; i < 96; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    Uint16 used = 0;
    for (Uint16 layer = 0; layer < 3; ++layer)
    {
        SotnDataView profile = {0}, grid = {0};
        if (!Slice(s->nz0, tables.planes + layer * 8U, 8, &profile))
            return false;
        const Uint32 address = Read32(profile.bytes);
        const Sint32 height = Signed16(profile.bytes + 4);
        const Uint16 priority = Read16(profile.bytes + 6);
        const Uint32 grids = 0x80180000U + (Uint32)tables.grids;
        if (address < grids || address > grids + 128U || (address - grids) % 64U || height < -512 ||
            height > 512 || priority > 511 || !Slice(s->nz0, address - 0x80180000U, 64, &grid))
            return SDL_SetError(
                "Chapel cloud plane has an invalid private grid, height or priority");
        const Uint32 phase = a->clouds.y[layer] >> 16;
        const Sint32 scroll = Wrap16(a->clouds.x[layer] >> 16);
        const Sint32 x = (Pixel(a->state.x) - s->camera_x + scroll) % 2048;
        for (Uint16 band = 0; band < 8; ++band)
        {
            const Uint16 depth = (Uint16)(band * 256U + 448U - phase % 256U);
            const Uint32 row = (band + phase / 256U) % 8U;
            if (!Band(s, a, (SotnDataView){grid.bytes + row * 8U, 8}, x, height, depth, priority,
                      &used))
                return false;
            if (used == 96)
                return true;
        }
    }
    return true;
}
bool ActorCastleSky(SotnActors *s, Actor *a)
{
    if (!s || !a || (s->stage_id != 6 && s->stage_id != 25) ||
        (a->state.id != 0x1f && a->state.id != 0x20))
        return SDL_SetError("Castle sky requires an original Chapel/BO5 cloud or land owner");
    return a->state.id == 0x20 ? Land(s, a) : Clouds(s, a);
}
