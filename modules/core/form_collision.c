// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/71830.c:func_80111D24/func_80111DE8, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private collision tables are embedded.
#include <SDL3/SDL.h>
#include <sotn/form_collision.h>

static bool Position(Sint32 x, Sint32 y)
{
    return x >= SDL_MIN_SINT16 && x <= SDL_MAX_SINT16 && y >= SDL_MIN_SINT16 && y <= SDL_MAX_SINT16;
}
bool SotnFormCollision_Barrier(const SotnCollisionMap *map, Sint32 x, Sint32 y, bool facing_left,
                               SotnFormBarrier *out)
{
    if (!map || !out || !Position(x, y))
        return SDL_SetError("Form barrier escape requires a map and bounded player position");
    SotnCollider left = {0}, right = {0};
    if (!SotnCollision_Query(map, x - 7, y, &left) || !SotnCollision_Query(map, x + 7, y, &right))
        return false;
    const bool l = (left.effects & SOTN_EFFECT_MIST_ONLY) != 0,
               r = (right.effects & SOTN_EFFECT_MIST_ONLY) != 0;
    *out = (SotnFormBarrier){.blocked = l || r,
                             .velocity_x = l && r ? (facing_left ? -49152 : 49152)
                                           : r    ? -49152
                                           : l    ? 49152
                                                  : 0};
    return true;
}
static bool Edge(Uint32 effects, bool right)
{
    const Uint32 filtered = effects & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                                       SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK);
    if (right)
    {
        switch (filtered)
        {
        case SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SOLID:
        case SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_BLOCK:
        case SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID:
        case SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK:
        case SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK:
        case SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK:
        case SOTN_EFFECT_BLOCK:
            return true;
        default:
            return false;
        }
    }
    switch (filtered)
    {
    case SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID:
    case SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK:
    case SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID:
    case SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK:
    case SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_BLOCK:
    case SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK:
    case SOTN_EFFECT_BLOCK:
        return true;
    default:
        return false;
    }
}
bool SotnFormCollision_FloorEdge(const SotnCollisionMap *map, Sint32 x, Sint32 y,
                                 Sint16 floor_sensor_y, bool ignore_mist, SotnFormEdge *out)
{
    if (!map || !out || !Position(x, y))
        return SDL_SetError("Form floor escape requires a map and bounded player position");
    const Sint32 probe_y = y + floor_sensor_y - 2;
    SotnCollider first = {0}, second = {0};
    if (!SotnCollision_Query(map, x + 9, probe_y, &first))
        return false;
    if (ignore_mist && (first.effects & SOTN_EFFECT_MIST_ONLY))
        first.effects = SOTN_EFFECT_NONE;
    if (Edge(first.effects, true))
    {
        if (!SotnCollision_Query(map, x + 8 + first.tile_left, probe_y, &second))
            return false;
        // The original clears the discarded first.effects on a second-probe Mist tile.
        // Retain the second result's solid bit; clearing it would change barrier ejection.
        if (!(second.effects & SOTN_EFFECT_SOLID))
        {
            *out = (SotnFormEdge){.displaced = true, .shift_x = first.tile_left};
            return true;
        }
    }
    if (!SotnCollision_Query(map, x - 9, probe_y, &first))
        return false;
    if (ignore_mist && (first.effects & SOTN_EFFECT_MIST_ONLY))
        first.effects = SOTN_EFFECT_NONE;
    if (Edge(first.effects, false))
    {
        if (!SotnCollision_Query(map, x - 8 + first.tile_right, probe_y, &second))
            return false;
        if (!(second.effects & SOTN_EFFECT_SOLID))
        {
            *out = (SotnFormEdge){.displaced = true, .shift_x = first.tile_right};
            return true;
        }
    }
    *out = (SotnFormEdge){0};
    return true;
}
