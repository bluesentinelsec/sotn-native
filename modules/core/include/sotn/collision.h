#ifndef SOTN_COLLISION_H
#define SOTN_COLLISION_H

#include <sotn/game_data.h>

/** @brief Original collision effect bits consumed by player and entity sensors. */
typedef enum SotnCollisionEffect
{
    SOTN_EFFECT_NONE = 0,
    SOTN_EFFECT_SOLID = 0x0001,
    SOTN_EFFECT_SIDE = 0x0002,
    SOTN_EFFECT_SINK = 0x0004,
    SOTN_EFFECT_WATER = 0x0008,
    SOTN_EFFECT_MIST_ONLY = 0x0010,
    SOTN_EFFECT_HURT = 0x0020,
    SOTN_EFFECT_SOLID_FROM_ABOVE = 0x0040,
    SOTN_EFFECT_SOLID_FROM_BELOW = 0x0080,
    SOTN_EFFECT_SLOPE_CEILING = 0x0800,
    SOTN_EFFECT_SLOPE_HALF = 0x1000,
    SOTN_EFFECT_SLOPE_QUARTER = 0x2000,
    SOTN_EFFECT_SLOPE_LEFT = 0x4000,
    SOTN_EFFECT_SLOPE_FLOOR = 0x8000,
    SOTN_EFFECT_BLOCK = SOTN_EFFECT_SOLID | SOTN_EFFECT_SIDE
} SotnCollisionEffect;

/** @brief Borrowed little-endian tile IDs and byte collision codes for 16x16 tiles. */
typedef struct SotnCollisionMap
{
    SotnDataView tiles; /**< Row-major Uint16 IDs, decoded without native pointer casts. */
    SotnDataView types; /**< One collision code per tile definition. */
    Uint32 width;       /**< Width in tiles, greater than zero. */
    Uint32 height;      /**< Height in tiles, greater than zero. */
} SotnCollisionMap;

/** @brief Original tile bounds and resolved surface distances relative to a query point. */
typedef struct SotnCollider
{
    Uint32 effects;
    Sint32 tile_left, tile_top, tile_right, tile_bottom;
    Sint32 left, top, right, bottom;
} SotnCollider;

/**
 * @brief Evaluate the original US tile collision rules at a room-local pixel position.
 * @param map Borrowed immutable tile map; its views must remain alive for the call.
 * @param x Horizontal pixel position; positions outside the map query an empty tile.
 * @param y Vertical pixel position; no camera offset is added by this API.
 * @param out Receives effects and distances on success; unchanged on failure.
 * @return True on success, false with SDL_GetError() for invalid maps or tile references.
 * @note Neighbor probes are limited to one level as in the original CheckCollision.
 * No allocation, graphics initialization, or global game state is required.
 */
bool SotnCollision_Query(const SotnCollisionMap *map, Sint32 x, Sint32 y, SotnCollider *out);

#endif
