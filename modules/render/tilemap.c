// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/4CE2C.c:RenderTilemap and src/pc/sim_pc.c:LoadStageTileset.
#include <SDL3/SDL_error.h>
#include <sotn/tilemap.h>

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static bool TextureWord(SotnTileSources sources, Uint32 x, Uint32 y, Uint16 *out)
{
    if (x < 512 || x >= 1024 || y >= 512)
        return SDL_SetError("Tile texture references an unsupported framebuffer VRAM region");
    const SotnDataView source = y >= 256 ? sources.common : sources.stage;
    x -= 512;
    y &= 255;
    const size_t block = (x / 64) * 4 + (x % 64) / 32 + (y / 128) * 2;
    const size_t offset = block * 8192 + (y % 128) * 64 + (x % 32) * 2;
    *out = Read16(source.bytes + offset);
    return true;
}

static bool PaletteColor(SotnTileSources sources, Uint32 palette, Uint32 index, Uint16 *out)
{
    const Uint32 x = (palette & 15) * 16 + index;
    const Uint32 y = palette >> 4;
    if (palette >= 256)
    {
        if (x >= 256 || y >= 32)
            return SDL_SetError("Tile palette extends beyond loaded common palette data");
        *out = Read16(sources.common.bytes + 0x40000 + ((y - 16) * 256 + x) * 2);
        return true;
    }
    return TextureWord(sources, 512 + x, 240 + y, out);
}

static Uint16 Blend(Uint16 background, Uint16 foreground, Uint32 mode)
{
    Uint16 color = 0;
    for (unsigned int shift = 0; shift < 15; shift += 5)
    {
        const Sint32 back = (background >> shift) & 31;
        const Sint32 front = (foreground >> shift) & 31;
        Sint32 channel = 0;
        switch (mode)
        {
        case 0:
            channel = back / 2 + front / 2;
            break;
        case 1:
            channel = back + front;
            break;
        case 2:
            channel = back - front;
            break;
        default:
            channel = back + front / 4;
            break;
        }
        if (channel < 0)
            channel = 0;
        if (channel > 31)
            channel = 31;
        color |= (Uint16)((Uint16)channel << shift);
    }
    return color;
}

static bool Sample(const SotnStageLayer *layer, SotnTileSources sources, Sint64 x, Sint64 y,
                   Uint16 *color)
{
    *color = 0;
    if (layer->flags & 0x1000)
    {
        x = (Sint64)((Uint64)x & 255);
        y = (Sint64)((Uint64)y & 255);
    }
    if (x < 0 || y < 0 || x / 16 >= layer->collision.width || y / 16 >= layer->collision.height)
        return true;
    const size_t tile_index = (size_t)(y / 16) * layer->collision.width + (size_t)(x / 16);
    const Uint16 tile = Read16(layer->collision.tiles.bytes + tile_index * 2);
    if (tile == 0)
        return true;
    if (tile >= layer->pages.size || tile >= layer->graphics.size || tile >= layer->palettes.size)
        return SDL_SetError("Tile graphics reference a missing definition");
    const Uint32 page = layer->pages.bytes[tile] + 8U;
    if (page >= 32)
        return SDL_SetError("Tile graphics reference an unsupported texture page");
    const Uint32 graphic = layer->graphics.bytes[tile];
    const Uint32 u = ((graphic << 4) & 255) + (Uint32)(x & 15);
    const Uint32 v = (graphic & 0xf0) + (Uint32)(y & 15);
    const bool eight_bit = (layer->flags & 0x100) != 0;
    Uint16 packed = 0;
    if (!TextureWord(sources, (page & 15) * 64 + u / (eight_bit ? 2U : 4U), (page & 16) * 16 + v,
                     &packed))
        return false;
    const Uint32 index =
        eight_bit ? (packed >> ((u & 1) * 8)) & 255 : (packed >> ((u & 3) * 4)) & 15;
    const Uint32 palette = layer->palettes.bytes[tile] + ((layer->flags & 0x200) ? 256U : 0U);
    return PaletteColor(sources, palette, index, color);
}

bool SotnTilemap_Draw(const SotnStageLayer *layer, SotnTileSources sources, Sint32 scroll_x,
                      Sint32 scroll_y, Uint16 *frame, size_t count)
{
    if (layer == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Tile rendering requires a layer and a full original-size framebuffer");
    if (!layer->present || (layer->flags & 1) == 0)
        return true;
    if (sources.stage.bytes == NULL || sources.stage.size != 0x40000 ||
        sources.common.bytes == NULL || sources.common.size != 0x42000 ||
        layer->collision.tiles.bytes == NULL || layer->collision.width == 0 ||
        layer->collision.height == 0 ||
        layer->collision.width > (layer->collision.tiles.size / 2) / layer->collision.height ||
        layer->pages.bytes == NULL || layer->graphics.bytes == NULL ||
        layer->palettes.bytes == NULL)
        return SDL_SetError("Tile rendering requires bounded map, definition, and graphics views");
    for (unsigned int pass = 0; pass < 2; ++pass)
    {
        for (Uint32 y = 0; y < SOTN_FRAME_HEIGHT; ++y)
        {
            for (Uint32 x = 0; x < SOTN_FRAME_WIDTH; ++x)
            {
                Uint16 color = 0;
                if (!Sample(layer, sources, (Sint64)scroll_x + x, (Sint64)scroll_y + y, &color))
                    return false;
                if (pass == 1 && color != 0)
                {
                    const size_t pixel = (size_t)y * SOTN_FRAME_WIDTH + x;
                    frame[pixel] = (layer->flags & 0x80) && (color & 0x8000)
                                       ? Blend(frame[pixel], color, (layer->flags >> 5) & 3)
                                       : color;
                }
            }
        }
    }
    return true;
}

bool SotnFrame_ToRgba(const Uint16 *frame, size_t count, Uint8 *rgba, size_t capacity)
{
    if (frame == NULL || rgba == NULL || count < SOTN_FRAME_PIXELS ||
        capacity < (size_t)SOTN_FRAME_PIXELS * 4)
        return SDL_SetError("RGBA conversion requires a full original-size frame and output");
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
    {
        for (unsigned int channel = 0; channel < 3; ++channel)
            rgba[i * 4 + channel] = (Uint8)((((frame[i] >> (channel * 5)) & 31) * 255 + 15) / 31);
        rgba[i * 4 + 3] = 255;
    }
    return true;
}
