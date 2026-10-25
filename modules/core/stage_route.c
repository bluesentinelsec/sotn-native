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
