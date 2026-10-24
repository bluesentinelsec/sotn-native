// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/4B758.c sprite-sheet branch and src/pc/pl_arc.c pointer-table loading.
#include <SDL3/SDL_error.h>
#include <sotn/alucard_sprite.h>

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Sint32 Signed16(Uint16 bits)
{
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}

static bool Pointer(SotnDataView source, size_t offset, Uint32 base, size_t size, SotnDataView *out)
{
    if (source.bytes == NULL || offset > source.size || source.size - offset < 4)
        return SDL_SetError("Alucard sprite pointer table is truncated");
    const Uint8 *bytes = source.bytes + offset;
    const Uint32 address =
        bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
    if (address < base || address - base > source.size || size > source.size - (address - base))
        return SDL_SetError("Alucard sprite pointer is outside its private data file");
    *out = (SotnDataView){source.bytes + (address - base), size};
    return true;
}

bool SotnAlucard_DecodeSprite(SotnDataView dra, SotnDataView arc, Uint16 frame,
                              SotnAlucardSprite *out)
{
    if (out == NULL || frame >= 265)
        return SDL_SetError("Alucard sprite requires an output and a frame ID below 265");
    if (frame == 0)
    {
        *out = (SotnAlucardSprite){0};
        return true;
    }
    SotnDataView definition = {0}, sheet = {0};
    if (!Pointer(dra, 0x2f324 + (size_t)frame * 4, 0x800a0000, 8, &definition))
        return false;
    const Uint16 sheet_id = Read16(definition.bytes);
    if ((sheet_id & 0x8000) == 0 || (sheet_id & 0x7fff) >= 256)
        return SDL_SetError("Alucard frame does not reference a supported ARC sprite sheet");
    const size_t table = 32 + (size_t)(sheet_id & 0x7fff) * 4;
    if (!Pointer(arc, table, 0x8013c000, 4, &sheet))
        return false;
    SotnAlucardSprite sprite = {0};
    sprite.width = sheet.bytes[0];
    sprite.height = sheet.bytes[1];
    if (sprite.width == 0 || sprite.height == 0 || sprite.width % 4 != 0)
        return SDL_SetError("Alucard sprite dimensions cannot form an original VRAM transfer");
    const size_t bytes = (size_t)sprite.width * sprite.height / 2;
    if (!Pointer(arc, table, 0x8013c000, 4 + bytes, &sheet))
        return false;
    sprite.pixels = (SotnDataView){sheet.bytes + 4, bytes};
    sprite.pivot_x = Signed16(Read16(definition.bytes + 2)) + sheet.bytes[2];
    sprite.pivot_y = Signed16(Read16(definition.bytes + 4)) + sheet.bytes[3];
    *out = sprite;
    return true;
}

bool SotnAlucard_DrawSprite(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x,
                            Sint32 y, bool facing_left, Uint16 *frame, size_t count)
{
    if (sprite == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Alucard drawing requires a decoded sprite and full framebuffer");
    if (sprite->width == 0 && sprite->height == 0)
        return true;
    if (sprite->width > 255 || sprite->height > 255 || sprite->width == 0 || sprite->height == 0 ||
        sprite->width % 4 != 0 || sprite->pixels.bytes == NULL ||
        sprite->pixels.size != (size_t)sprite->width * sprite->height / 2 || common.bytes == NULL ||
        common.size != 0x42000)
        return SDL_SetError("Alucard drawing requires bounded packed pixels and a base palette");
    const Sint64 left =
        facing_left ? (Sint64)x - sprite->pivot_x - sprite->width + 1 : (Sint64)x + sprite->pivot_x;
    const Sint64 top = (Sint64)y + sprite->pivot_y;
    for (Uint32 row = 0; row < sprite->height; ++row)
    {
        const Sint64 py = top + row;
        if (py < 0 || py >= SOTN_FRAME_HEIGHT)
            continue;
        for (Uint32 column = 0; column < sprite->width; ++column)
        {
            const Sint64 px = left + column;
            if (px < 0 || px >= SOTN_FRAME_WIDTH)
                continue;
            const Uint32 source_x = facing_left ? sprite->width - column - 1 : column;
            const size_t pixel = (size_t)row * sprite->width + source_x;
            const Uint8 index = (sprite->pixels.bytes[pixel / 2] >> ((pixel & 1) * 4)) & 15;
            const Uint16 color = Read16(common.bytes + 0x40000 + (size_t)index * 2);
            if (color != 0)
                frame[(size_t)py * SOTN_FRAME_WIDTH + (size_t)px] = color;
        }
    }
    return true;
}
