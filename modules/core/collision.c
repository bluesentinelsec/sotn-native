// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/collider.c:CheckCollision. See docs/game-core.md for modifications.
#include <SDL3/SDL_error.h>
#include <sotn/collision.h>

static Sint32 SlopeBottom(Sint32 numerator, unsigned int shift, Sint32 top)
{
    /* Preserve the original unsigned shift and 32-bit result, including high bits. */
    const Uint32 bits = ((Uint32)numerator >> shift) + 15U + (Uint32)top;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : -1 - (Sint32)(SDL_MAX_UINT32 - bits);
}

static bool Query(const SotnCollisionMap *map, Sint64 x, Sint64 y, SotnCollider *res, bool unk)
{
    SotnCollider col0 = {0};
    SotnCollider col1 = {0};
    Sint32 offset = 0;
    Uint8 colType = 0;
    if (x >= 0 && y >= 0 && x / 16 < map->width && y / 16 < map->height)
    {
        const size_t tile = (size_t)(y / 16) * map->width + (size_t)(x / 16);
        const size_t position = tile * 2;
        const Uint16 id =
            (Uint16)(map->tiles.bytes[position] | (Uint16)map->tiles.bytes[position + 1] << 8);
        if (id >= map->types.size)
            return SDL_SetError("Collision tile %zu references missing definition %u", tile,
                                (unsigned int)id);
        colType = map->types.bytes[id];
    }
    res->effects = SOTN_EFFECT_NONE;
    res->tile_left = res->left = -((Sint32)((Uint64)x & 0xF));
    res->tile_right = res->right = res->left + 0xF;
    res->tile_top = res->top = -((Sint32)((Uint64)y & 0xF));
    res->tile_bottom = res->bottom = res->top + 0xF;
    if ((colType & 0x80) == 0)
    {
        res->effects = colType & SOTN_EFFECT_BLOCK;
        return true;
    }

    offset = 0;
    switch (colType - 0x80)
    {
    case 0x01:
    case 0x02:
    case 0x0A:
    case 0x0B:
    case 0x18:
    case 0x19:
        if (!unk)
        {
            if (!Query(map, x, y + res->top - 1, &col0, true))
                return false;
            if (col0.effects & SOTN_EFFECT_SLOPE_FLOOR)
            {
                if (col0.effects & SOTN_EFFECT_SOLID)
                {
                    res->top += col0.top - 1;
                }
                res->effects = col0.effects | SOTN_EFFECT_BLOCK;
            }
            else
            {
                res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK;
            }
        }
        else
        {
            res->effects = SOTN_EFFECT_SOLID;
        }
        break;

    case 0x00:
        if (res->right + res->bottom < 0x10)
        {
            res->left = res->top = res->right + res->bottom - 0xF;
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_FLOOR;
        }
        break;

    case 0x08:
        offset = 0x10;
        /* Fall through. */
    case 0x09:
        if (res->right + offset + res->bottom * 2 < 0x20)
        {
            res->left = res->right + offset + res->bottom * 2 - 0x1F;
            res->top = ((res->right + offset) >> 1) + res->bottom - 0xF;
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_HALF | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_HALF;
        }
        break;

    case 0x14:
        offset = 0x10;
        /* Fall through. */
    case 0x15:
        offset += 0x10;
        /* Fall through. */
    case 0x16:
        offset += 0x10;
        /* Fall through. */
    case 0x17:
        if (res->right + offset + res->bottom * 4 < 0x40)
        {
            res->left = res->right + offset + res->bottom * 4 - 0x3F;
            res->top = ((res->right + offset) >> 2) + res->bottom - 0xF;
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_QUARTER | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_QUARTER;
        }
        break;

    case 0x03:
        if (res->right >= res->bottom)
        {
            res->top = res->bottom - res->right;
            res->right = -res->top;
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT;
        }
        break;

    case 0x0C:
        offset = 0x10;
        /* Fall through. */
    case 0x0D:
        if (res->right + offset >= res->bottom * 2)
        {
            res->top = res->bottom - ((offset + res->right) >> 1);
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                           SOTN_EFFECT_SLOPE_HALF | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects =
                SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_HALF;
        }
        break;

    case 0x1A:
        offset = 0x10;
        /* Fall through. */
    case 0x1B:
        offset += 0x10;
        /* Fall through. */
    case 0x1C:
        offset += 0x10;
        /* Fall through. */
    case 0x1D:
        if (res->right + offset >= res->bottom * 4)
        {
            res->top = res->bottom - (res->right + offset) / 4;
            res->effects = SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                           SOTN_EFFECT_SLOPE_QUARTER | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects =
                SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_QUARTER;
        }
        break;

    case 0x05:
    case 0x06:
    case 0x10:
    case 0x11:
    case 0x22:
    case 0x23:
        if (!unk)
        {
            if (!Query(map, x, y + res->bottom + 1, &col1, true))
                return false;
            if (col1.effects & SOTN_EFFECT_SLOPE_CEILING)
            {
                if (col1.effects & SOTN_EFFECT_SOLID)
                {
                    res->bottom += col1.bottom + 1;
                }
                res->effects = col1.effects | SOTN_EFFECT_BLOCK;
            }
            else
            {
                res->effects = SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK;
            }
        }
        else
        {
            res->effects = SOTN_EFFECT_SOLID;
        }
        break;

    case 0x04:
        if (res->right <= res->bottom)
        {
            res->left = res->right - res->bottom;
            res->bottom = -res->left;
            res->effects = SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_CEILING;
        }
        break;

    case 0x0E:
        offset = 0x10;
        /* Fall through. */
    case 0x0F:
        if (res->right + offset <= res->bottom * 2)
        {
            res->left = res->right + offset - res->bottom * 2;
            res->bottom = -res->left / 2;
            res->effects = SOTN_EFFECT_SLOPE_HALF | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_HALF | SOTN_EFFECT_SLOPE_CEILING;
        }
        break;

    case 0x1E:
        offset = 0x10;
        /* Fall through. */
    case 0x1F:
        offset += 0x10;
        /* Fall through. */
    case 0x20:
        offset += 0x10;
        /* Fall through. */
    case 0x21:
        if (res->right + offset <= res->bottom * 4)
        {
            res->left = res->right + offset - res->bottom * 4;
            res->bottom = -res->left / 4;
            res->effects =
                SOTN_EFFECT_SLOPE_QUARTER | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_QUARTER | SOTN_EFFECT_SLOPE_CEILING;
        }
        break;

    case 0x07:
        if (res->right + res->bottom > 0xE)
        {
            res->right = res->bottom = res->left + 0xF + res->top;
            res->effects = SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects = SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING;
        }
        break;

    case 0x12:
        offset = 0x10;
        /* Fall through. */
    case 0x13:
        if (res->right + offset + res->bottom * 2 > 0x1D)
        {
            res->bottom = SlopeBottom(res->left + offset - 0xF, 1, res->top);
            res->effects = SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_HALF |
                           SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects =
                SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_HALF | SOTN_EFFECT_SLOPE_CEILING;
        }
        break;

    case 0x24:
        offset = 0x10;
        /* Fall through. */
    case 0x25:
        offset += 0x10;
        /* Fall through. */
    case 0x26:
        offset += 0x10;
        /* Fall through. */
    case 0x27:
        if (res->right + offset + res->bottom * 4 > 0x3B)
        {
            res->bottom = SlopeBottom(res->left + offset - 0x2D, 2, res->top);
            res->effects = SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_QUARTER |
                           SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID;
        }
        else
        {
            res->effects =
                SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_QUARTER | SOTN_EFFECT_SLOPE_CEILING;
        }
        break;

    case 0x7F:
        if (res->bottom < 8)
        {
            res->effects = SOTN_EFFECT_BLOCK;
            res->top += 8;
        }
        break;

    case 0x7E:
        if (res->bottom >= 8)
        {
            res->effects = SOTN_EFFECT_BLOCK;
            res->bottom -= 8;
        }
        break;

    case 0x7D:
        if (res->bottom < 8)
        {
            res->effects = SOTN_EFFECT_SOLID;
            res->top += 8;
        }
        break;

    case 0x7C:
        if (res->bottom >= 8)
        {
            res->effects = SOTN_EFFECT_SOLID;
            res->bottom -= 8;
        }
        break;

    case 0x7B:
        res->effects = SOTN_EFFECT_SINK;
        break;

    case 0x7A:
        if (res->bottom < 8)
        {
            res->effects = SOTN_EFFECT_SINK;
            res->top += 8;
        }
        break;

    case 0x79:
        if (res->bottom < 8)
        {
            res->effects = SOTN_EFFECT_WATER;
            res->top += 8;
        }
        break;

    case 0x78:
        res->effects = SOTN_EFFECT_MIST_ONLY | SOTN_EFFECT_BLOCK;
        break;

    case 0x77:
    case 0x76:
    case 0x75:
    case 0x74:
        res->effects = SOTN_EFFECT_HURT;
        break;

    case 0x73:
    case 0x72:
    case 0x71:
    case 0x70:
    case 0x6F:
    case 0x6E:
        res->effects = SOTN_EFFECT_BLOCK;
        break;

    case 0x6D:
        res->effects = SOTN_EFFECT_WATER;
        break;

    case 0x6C:
    case 0x6B:
        break;

    case 0x6A:
        if (res->bottom >= 8)
        {
            res->effects = SOTN_EFFECT_WATER;
            res->bottom -= 8;
        }
        break;

    case 0x69:
        if (res->bottom >= 8)
        {
            res->effects = SOTN_EFFECT_SINK;
            res->bottom -= 8;
        }
        break;

    case 0x68:
        if (res->bottom < 8)
        {
            res->effects = SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID;
            res->top += 8;
        }
        break;

    case 0x67:
        if (res->bottom >= 8)
        {
            res->effects = SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID;
            res->bottom -= 8;
        }
        break;

    case 0x66:
        if (res->bottom < 8)
        {
            res->effects = SOTN_EFFECT_SOLID_FROM_BELOW | SOTN_EFFECT_SOLID;
            res->top += 8;
        }
        break;

    case 0x65:
        if (res->bottom >= 8)
        {
            res->effects = SOTN_EFFECT_SOLID_FROM_BELOW | SOTN_EFFECT_SOLID;
            res->bottom -= 8;
        }
        break;
    }
    return true;
}

bool SotnCollision_Query(const SotnCollisionMap *map, Sint32 x, Sint32 y, SotnCollider *out)
{
    if (map == NULL || out == NULL || map->tiles.bytes == NULL || map->types.bytes == NULL ||
        map->width == 0 || map->height == 0 || map->types.size == 0 ||
        map->width > (map->tiles.size / 2) / map->height)
        return SDL_SetError("Collision map requires bounded tile and definition views");
    SotnCollider result = {0};
    if (!Query(map, x, y, &result, false))
        return false;
    *out = result;
    return true;
}
