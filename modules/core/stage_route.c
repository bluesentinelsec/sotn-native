// SPDX-License-Identifier: AGPL-3.0-or-later
// Serialized RoomTeleport interpretation from dra/5087C.c:func_800F14CC/func_800F16D0,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Records remain private.
#include <SDL3/SDL.h>
#include <sotn/stage_route.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnStageRoute_Read(SotnDataView dra, Uint16 source, Uint16 index, SotnStageRoute *out)
{
    const size_t offset = 0x245c + (size_t)index * 10;
    if (out == NULL || dra.bytes == NULL || index >= 131 || offset > dra.size ||
        10 > dra.size - offset)
        return SDL_SetError(
            "Stage transition requires an intact US DRA record and index below 131");
    const Uint8 *p = dra.bytes + offset;
    const Uint16 room_offset = Read16(p + 4);
    const SotnStageRoute route = {.source = Read16(p + 6),
                                  .stage = Read16(p + 8),
                                  .room = (Uint16)(room_offset / 8),
                                  .x = Read16(p),
                                  .y = Read16(p + 2)};
    if (route.source != source || source > 65 || room_offset % 8 != 0 || route.room >= 256 ||
        route.stage > 65 || route.x > 16384 || route.y > 16384)
        return SDL_SetError(
            "Stage transition has invalid source, room, destination, or coordinates");
    *out = route;
    return true;
}

// dra/5087C.c:func_800F0CD8. Vertical rooms overlap at the transition; a world-grid
// rebase alone loses the original upward 48-pixel and downward 24/48-pixel adjustments.
static bool CrossingBounds(Sint32 x, Sint32 y, Uint32 width, Uint32 height)
{
    return width >= 256 && width <= 16384 && height >= 256 && height <= 16384 && width % 256 == 0 &&
           height % 256 == 0 && x >= -64 && y >= -64 && x <= (Sint32)width + 64 &&
           y <= (Sint32)height + 64;
}
static SotnRoomCrossing VerticalCrossing(Sint32 x, Sint32 y, Uint32 height, bool grounded)
{
    SotnRoomCrossing crossing = {.x = x, .y = y};
    if (y < 4)
    {
        crossing.active = true;
        crossing.y = -1;
        crossing.arrival_y = -48;
    }
    else
    {
        const Sint32 overlap = grounded ? 48 : 24;
        if (y >= (Sint32)height - overlap + 20)
        {
            crossing.active = true;
            crossing.y = (Sint32)height;
            crossing.arrival_y = overlap;
        }
    }
    return crossing;
}
bool SotnStageRoute_Crossing(Sint32 x, Sint32 y, Uint32 width, Uint32 height, bool grounded,
                             SotnRoomCrossing *out)
{
    if (out == NULL || !CrossingBounds(x, y, width, height))
        return SDL_SetError("Room crossing requires bounded tile-grid dimensions and position");
    *out = x < 0 || x >= (Sint32)width ? (SotnRoomCrossing){.x = x, .y = y, .active = true}
                                       : VerticalCrossing(x, y, height, grounded);
    return true;
}

bool SotnStageRoute_WindowCrossing(SotnRoomWindow window, Sint32 x, Sint32 y, Uint32 width,
                                   Uint32 height, bool grounded, SotnRoomCrossing *out)
{
    if (out == NULL || !CrossingBounds(x, y, width, height))
        return SDL_SetError("Room window requires bounded tile-grid dimensions and position");
    if (window.start_x < 0 || window.end_x < 256 || window.start_x >= window.end_x ||
        window.end_x > (Sint32)width || window.origin_cell < 0 || window.origin_cell > 64 ||
        window.last_cell < -1 || window.last_cell >= (Sint32)(width / 256))
        return SDL_SetError("Room window has invalid horizontal limits or castle-cell endpoints");
    const Sint32 reference_x = x - window.origin_cell * 256;
    SotnRoomCrossing crossing = VerticalCrossing(x, y, height, grounded);
    if (reference_x < window.start_x)
        crossing = (SotnRoomCrossing){.x = (window.origin_cell - 1) * 256, .y = y, .active = true};
    else if (reference_x >= window.end_x)
        crossing = (SotnRoomCrossing){.x = (window.last_cell + 1) * 256, .y = y, .active = true};
    *out = crossing;
    return true;
}

// dra/5087C.c:FindBossTeleport. Serialized indices omit the caller's +2 return tag.
bool SotnStageRoute_FindBoss(SotnDataView dra, Uint16 source, Sint32 x, Sint32 y,
                             const SotnWorldState *world, bool *found, SotnStageRoute *out)
{
    if (dra.bytes == NULL || world == NULL || found == NULL || out == NULL || source > 65 ||
        x < -1 || x > 64 || y < -1 || y > 64)
        return SDL_SetError(
            "Boss transfer requires private DRA, bounded chunks and event progress");
    if (!SotnWorld_Validate(world))
        return false;
    for (Uint32 i = 0; i < 64; ++i)
    {
        const size_t offset = 0x297c + (size_t)i * 20;
        if (offset > dra.size || 20 > dra.size - offset)
            return SDL_SetError("Private boss transfer table is truncated before its sentinel");
        const Uint8 *p = dra.bytes + offset;
        const Uint32 chunk_x = p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
        if (chunk_x == 128)
        {
            *found = false;
            return true;
        }
        Uint32 words[5] = {0};
        for (Uint32 word = 0; word < 5; ++word)
        {
            const Uint8 *v = p + word * 4U;
            words[word] = v[0] | (Uint32)v[1] << 8 | (Uint32)v[2] << 16 | (Uint32)v[3] << 24;
        }
        if (words[0] >= 64 || words[1] >= 64 || words[2] > 65 ||
            (words[3] >= SOTN_TIME_ATTACK_EVENTS && words[3] != 255) || words[4] >= 131)
            return SDL_SetError("Private boss transfer %u has invalid coordinates, event or index",
                                i);
        if (words[0] != (Uint32)x || words[1] != (Uint32)y || words[2] != source ||
            (words[3] != 255 && world->time_attack[words[3]] != 0))
            continue;
        SotnStageRoute route = {0};
        if (!SotnStageRoute_Read(dra, source, (Uint16)words[4], &route))
            return false;
        *out = route;
        *found = true;
        return true;
    }
    return SDL_SetError("Private boss transfer table has no bounded sentinel");
}
