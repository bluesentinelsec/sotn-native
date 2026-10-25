// SPDX-License-Identifier: AGPL-3.0-or-later
// Packed visitation/count rule from dra/5087C.c:func_800F2014, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Progress contains user state, not map art.
#include <sotn/world.h>
bool SotnWorld_Visit(SotnWorldState *s, Sint32 x, Sint32 y, bool inverted)
{
    if (s == NULL || x < 0 || x >= 64 || y < 0 || y >= 64 || s->rooms > 8192)
        return SDL_SetError(
            "Map visitation requires bounded castle coordinates and initialized progress");
    const Uint32 index = (Uint32)x / 4 + (Uint32)y * 16 + (inverted ? 1024U : 0U);
    const Uint8 mask = (Uint8)(1U << ((3 - ((Uint32)x & 3)) * 2));
    if (!(s->map[index] & mask))
    {
        if (s->rooms == 8192)
            return SDL_SetError("Castle visitation count exceeds the packed map");
        s->map[index] |= mask;
        ++s->rooms;
    }
    return true;
}
bool SotnWorld_Validate(const SotnWorldState *s)
{
    if (s == NULL || s->rooms > 8192 || s->laboratory_boss_time > 99999U * 3600)
        return SDL_SetError("Saved castle progress has invalid bounds");
    if (s->flags[SOTN_CLOCK_FLOOR] > 1)
        return SDL_SetError("Saved clock-room floor flag is invalid");
    if (s->flags[SOTN_OUTER_ELEVATOR] > 1 || s->flags[SOTN_OUTER_PRESSURE] > 1)
        return SDL_SetError("Saved Outer Wall mechanism flag is invalid");
    for (Uint32 i = 0; i < SOTN_RELICS; ++i)
        if (s->relics[i] > 3 || s->relics[i] == 2)
            return SDL_SetError("Saved relic has invalid found/active bits");
    Uint32 count = 0;
    for (Uint32 i = 0; i < SOTN_CASTLE_MAP; ++i)
        for (Uint32 bit = 0; bit < 8; bit += 2)
            count += (s->map[i] >> bit) & 1U;
    if (count != s->rooms)
        return SDL_SetError("Saved exploration count does not match its map bits");
    for (Uint32 i = SOTN_LAB_SECRET_WALL; i <= SOTN_LAB_MARIA; ++i)
        if (s->flags[i] > 1)
            return SDL_SetError("Saved Laboratory event flag is invalid");
    if (s->flags[SOTN_ENTRANCE_ROCKS] > 15 || s->flags[SOTN_ENTRANCE_JEWEL_DOOR] > 1)
        return SDL_SetError("Saved Entrance rock-puzzle flag is invalid");
    if (s->flags[SOTN_ENTRANCE_DEATH] > 1 || s->flags[SOTN_ENTRANCE_ON] > 1 ||
        s->flags[SOTN_ENTRANCE_STAIRS] > 1)
        return SDL_SetError("Saved Entrance event flag is invalid");
    return true;
}
