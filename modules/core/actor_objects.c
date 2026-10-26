// SPDX-License-Identifier: AGPL-3.0-or-later
// Stage object/player collision from st/st_common.h:GetPlayerCollisionWith,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "actors_internal.h"

bool ActorDisplace(SotnActors *s, Sint32 dx, Sint32 dy, Uint32 contacts)
{
    const Sint64 x = (Sint64)s->motion.x + dx, y = (Sint64)s->motion.y + dy;
    const Sint64 px = (Sint64)s->player.x + dx, py = (Sint64)s->player.y + dy;
    if (x < -64LL * 65536 || x > 64LL * 65536 || y < -64LL * 65536 || y > 64LL * 65536 ||
        px < SDL_MIN_SINT32 || px > SDL_MAX_SINT32 || py < SDL_MIN_SINT32 || py > SDL_MAX_SINT32)
        return SDL_SetError("Stage-object shove exceeds the bounded player displacement");
    s->motion.x = (Sint32)x;
    s->motion.y = (Sint32)y;
    s->motion.contacts |= contacts;
    s->player.x = (Sint32)px;
    s->player.y = (Sint32)py;
    s->player.contacts |= contacts;
    return true;
}
bool ActorObjectCollision(SotnActors *s, const Actor *a, Uint16 width, Uint16 height, Uint16 flags,
                          Uint16 *result)
{
    if (s == NULL || a == NULL || result == NULL || width > 256 || height > 256 ||
        s->player.solid_width > 64 || s->player.solid_height > 64)
        return SDL_SetError("Stage object collision has invalid extents");
    *result = 0;
    const Sint32 sx = Pixel(a->state.x) - s->camera_x, sy = Pixel(a->state.y) - s->camera_y;
    if (sx > 288 || sx < -32 || sy < -384 || sy > 384 || s->player.step == SOTN_PLAYER_KILL)
        return true;
    Sint32 x = Pixel(s->player.x) - Pixel(a->state.x) +
               (a->state.facing_left ? a->state.hitbox_x : -a->state.hitbox_x);
    Sint32 y = Pixel(s->player.y) - Pixel(a->state.y) - a->state.hitbox_y;
    const Sint32 w = (width + s->player.solid_width) * 2, h = (height + s->player.solid_height) * 2;
    const Uint16 checks = (Uint16)((x > 0 ? 1 : 0) | (y > 0 ? 2 : 0));
    x += w / 2;
    y += h / 2;
    if (x < 0 || x > w || y < 0 || y > h)
        return true;
    if (x != 0 && x != w)
    {
        if ((flags & 4) && (checks ^ 2) && s->player.velocity_y >= 0 && y < 8)
        {
            *result = 4;
            return ActorDisplace(s, 0, -y * 65536, 0x41);
        }
        if ((flags & 2) && (checks & 2) && (s->player.velocity_y <= 0 || (flags & 16)) &&
            h - y < 16)
        {
            *result = 2;
            return ActorDisplace(s, 0, (h - y) * 65536, 0x42);
        }
    }
    if (y != 0 && y != h && (flags & 1))
    {
        const Sint32 push =
            (flags & 8) ? SDL_min((checks & 1) ? w - x : x, 2) : ((checks & 1) ? w - x : x);
        *result = 1;
        return ActorDisplace(s, ((checks & 1) ? push : -push) * 65536, 0,
                             (checks & 1) ? 0x48 : 0x44);
    }
    return true;
}
bool ActorTile(SotnActors *s, Uint32 index, Uint16 tile)
{
    if (s == NULL || s->mutable_tiles == NULL || index >= (Uint64)s->map.width * s->map.height ||
        tile >= s->map.types.size)
        return SDL_SetError("Stage tile mutation exceeds the owned room or its definitions");
    s->mutable_tiles[(size_t)index * 2] = (Uint8)tile;
    s->mutable_tiles[(size_t)index * 2 + 1] = (Uint8)(tile >> 8);
    return true;
}
bool ActorTilePatch(SotnActors *s, Uint32 index, Uint32 rows, Uint32 columns, Uint32 stride,
                    size_t offset)
{
    SotnDataView data = {0};
    if (rows == 0 || rows > 64 || columns == 0 || columns > 64 || stride < columns ||
        !Slice(s->nz0, offset, (size_t)rows * columns * 2, &data))
        return SDL_SetError("Stage tile patch has invalid bounds");
    // Validate the entire patch before changing a tile.
    for (Uint32 r = 0; r < rows; ++r)
        for (Uint32 c = 0; c < columns; ++c)
            if ((Uint64)index + (Uint64)r * stride + c >= (Uint64)s->map.width * s->map.height ||
                Read16(data.bytes + ((size_t)r * columns + c) * 2) >= s->map.types.size)
                return SDL_SetError("Stage tile patch exceeds the room or tile definitions");
    for (Uint32 r = 0; r < rows; ++r)
        for (Uint32 c = 0; c < columns; ++c)
            if (!ActorTile(s, index + r * stride + c,
                           Read16(data.bytes + ((size_t)r * columns + c) * 2)))
                return false;
    return true;
}
bool ActorSprite(SotnActors *s, Actor *a, Uint16 index, Sint32 x, Sint32 y, Uint8 u, Uint8 v,
                 Uint16 width, Uint16 height, Uint16 page, Uint16 palette)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
    if (p == NULL || width == 0 || height == 0 || u + width > 255 || v + height > 255)
        return SDL_SetError("Stage sprite rectangle exceeds its texture page");
    p->state = (SotnActorPrimitive){.x = x + (Sint32)width * 32768,
                                    .y = y + (Sint32)height * 32768,
                                    .width = width,
                                    .height = height,
                                    .page = page,
                                    .palette = palette,
                                    .u = {u, (Uint8)(u + width), u, (Uint8)(u + width)},
                                    .v = {v, v, (Uint8)(v + height), (Uint8)(v + height)},
                                    .red = 128,
                                    .green = 128,
                                    .blue = 128,
                                    .active = true,
                                    .textured = true};
    return true;
}

bool SotnActors_SetBackground(SotnActors *s, SotnCollisionMap map, SotnCollisionMap *out)
{
    if (s == NULL || !s->entered || out == NULL)
        return SDL_SetError("Background tiles require an entered actor service and output");
    const bool empty = map.width == 0 && map.height == 0 && map.tiles.bytes == NULL &&
                       map.types.bytes == NULL && map.tiles.size == 0 && map.types.size == 0;
    Uint8 *tiles = NULL;
    size_t bytes = 0;
    if (!empty)
    {
        if (!map.width || !map.height || map.width > 1024 || map.height > 1024)
            return SDL_SetError("Background tiles require bounded room dimensions");
        SotnCollider probe = {0};
        if (!SotnCollision_Query(&map, 0, 0, &probe))
            return false;
        bytes = (size_t)map.width * map.height * 2;
        if (map.tiles.bytes == NULL || map.tiles.size < bytes)
            return SDL_SetError("Background tile data is truncated");
        tiles = SDL_malloc(bytes);
        if (tiles == NULL)
            return false;
        SDL_memcpy(tiles, map.tiles.bytes, bytes);
    }
    SDL_free(s->background_tiles);
    s->background_tiles = tiles;
    s->background = map;
    s->background.tiles = (SotnDataView){tiles, bytes};
    *out = s->background;
    return true;
}

bool SotnActors_SetTileArt(SotnActors *s, SotnDataView pages, SotnDataView graphics,
                           SotnDataView palettes)
{
    if (s == NULL || !s->entered || pages.bytes == NULL || graphics.bytes == NULL ||
        palettes.bytes == NULL || pages.size < s->map.types.size ||
        graphics.size < s->map.types.size || palettes.size < s->map.types.size)
        return SDL_SetError(
            "Tile effects require private artwork metadata for every room definition");
    s->tile_pages = pages;
    s->tile_graphics = graphics;
    s->tile_palettes = palettes;
    return true;
}
