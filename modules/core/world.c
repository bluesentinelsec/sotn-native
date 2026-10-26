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
    if (s == NULL || s->rooms > 8192 || s->game_clear > 1)
        return SDL_SetError("Saved castle progress has invalid bounds");
    for (Uint32 i = 0; i < SOTN_TIME_ATTACK_EVENTS; ++i)
    {
        const Uint32 record = s->time_attack[i];
        if (record > 999990000U || record % 100U >= 60 || record / 100U % 100U >= 60)
            return SDL_SetError("Saved time-attack event %u has an invalid HHMMSS record", i);
    }
    if (s->flags[SOTN_LIBRARY_MAP] > 1 || s->flags[SOTN_LIBRARY_MET] > 1 ||
        s->flags[SOTN_LIBRARY_DROPS] > 15 || s->flags[SOTN_LIBRARY_SHELF] > 1)
        return SDL_SetError("Saved Library encounter flag is invalid");
    if (s->flags[SOTN_CLOCK_FLOOR] > 1)
        return SDL_SetError("Saved clock-room floor flag is invalid");
    if (s->flags[SOTN_CHAPEL_SHORTCUT] > 1 || s->flags[SOTN_CHAPEL_MARIA] > 1 ||
        s->flags[SOTN_MARIA_HIPPOGRYPH] > 1)
        return SDL_SetError("Saved Chapel mechanism/encounter flag is invalid");
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

// dra/timeattack.c: original zero-only records and independent visited bits.
bool SotnWorld_RecordTime(SotnWorldState *s, Uint16 event, Uint32 seconds)
{
    if (s == NULL || event >= SOTN_TIME_ATTACK_EVENTS || seconds > 99999U * 3600)
        return SDL_SetError("Time-attack record requires a valid event and bounded elapsed time");
    if (s->time_attack[event] == 0)
        s->time_attack[event] =
            seconds % 60U + seconds / 60U % 60U * 100U + seconds / 3600U * 10000U;
    return true;
}
bool SotnWorld_VisitEvent(SotnWorldState *s, Uint16 event)
{
    if (s == NULL || event >= SOTN_TIME_ATTACK_EVENTS)
        return SDL_SetError("Time-attack visitation requires an original event index");
    s->time_attack_visited |= (Uint32)1U << event;
    return true;
}

// dra/5087C.c:func_800F2288; no private rectangle bytes are compiled into the game.
bool SotnWorld_Reveal(SotnWorldState *s, const Uint8 *bytes, size_t size, bool inverted)
{
    if (!bytes || !size || !SotnWorld_Validate(s))
        return SDL_SetError("Purchased map needs valid progress and a bounded rectangle stream");
    SotnWorldState next = *s;
    size_t cursor = 0;
    while (cursor < size)
    {
        const Uint32 x = bytes[cursor++];
        if (x == 255)
        {
            *s = next;
            return true;
        }
        if (size - cursor < 3)
            break;
        const Uint32 y = bytes[cursor++], width = bytes[cursor++], height = bytes[cursor++];
        if (!width || !height || x >= 64 || y >= 64 || width > (64U - x) / 8U || height > 64U - y ||
            width * height > size - cursor)
            return SDL_SetError("Purchased map rectangle exceeds castle or private stream bounds");
        for (Uint32 row = 0; row < height; ++row)
            for (Uint32 col = 0; col < width; ++col)
            {
                const Uint8 bits = bytes[cursor++];
                for (Uint32 bit = 0; bit < 8; ++bit)
                    if (bits & (1U << bit))
                    {
                        const Uint32 xx = x + col * 8 + bit;
                        const Uint32 index = xx / 4 + (y + row) * 16 + (inverted ? 1024U : 0U);
                        next.map[index] |= (Uint8)(2U << ((3U - (xx & 3U)) * 2));
                    }
            }
    }
    return SDL_SetError("Purchased map rectangle stream lacks a bounded terminator");
}
