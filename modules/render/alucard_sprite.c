// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/4B758.c sprite-sheet branch, 5F60C.c cape palettes,
// and src/pc/pl_arc.c pointer-table loading.
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_stdinc.h>
#include <sotn/alucard_sprite.h>

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
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

bool SotnAlucard_DrawGravity(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x,
                             Sint32 y, bool facing_left, Uint16 *frame, size_t count)
{
    if (sprite == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Gravity Boots drawing requires a decoded sprite and full frame");
    if (sprite->width == 0 && sprite->height == 0)
        return true;
    if (!sprite->width || sprite->width > 255 || !sprite->height || sprite->height > 255 ||
        sprite->width % 4 || !sprite->pixels.bytes ||
        sprite->pixels.size != (size_t)sprite->width * sprite->height / 2 || !common.bytes ||
        common.size != 0x42000)
        return SDL_SetError("Gravity Boots drawing requires bounded packed pixels and palette");
    for (Uint32 row = 0; row < sprite->height; ++row)
        for (Uint32 col = 0; col < sprite->width; ++col)
        {
            const Sint64 local_x = (Sint64)sprite->pivot_x + col;
            const Sint64 px = (Sint64)x + (facing_left ? local_x : -local_x);
            const Sint64 py = (Sint64)y + 4 - ((Sint64)sprite->pivot_y + row);
            if (px < 0 || px >= SOTN_FRAME_WIDTH || py < 0 || py >= SOTN_FRAME_HEIGHT)
                continue;
            const size_t pixel = (size_t)row * sprite->width + col;
            const Uint8 index = (sprite->pixels.bytes[pixel / 2] >> ((pixel & 1U) * 4)) & 15;
            const Uint16 color = Read16(common.bytes + 0x40000 + (size_t)index * 2);
            if (color)
                frame[(size_t)py * SOTN_FRAME_WIDTH + (size_t)px] = color;
        }
    return true;
}

bool SotnAlucard_DrawDeath(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x, Sint32 y,
                           bool facing_left, SotnPlayerDeath death, Uint16 *frame, size_t count)
{
    if (death.phase == 0)
        return SotnAlucard_DrawSprite(sprite, common, x, y, facing_left, frame, count);
    if (sprite == NULL || frame == NULL || count < SOTN_FRAME_PIXELS ||
        (death.phase != 1 && death.phase != 2 && death.phase != 128) || death.red < 128 ||
        death.red > 248 || death.blue < 8 || death.blue > 128 || death.rotation < -640 ||
        death.rotation > 0 || common.bytes == NULL || common.size != 0x42000)
        return SDL_SetError("Physical-death rendering requires bounded sprite/effect state");
    if (death.phase == 128 || (sprite->width == 0 && sprite->height == 0))
        return true;
    if (sprite->width == 0 || sprite->width > 255 || sprite->height == 0 || sprite->height > 255 ||
        sprite->pixels.bytes == NULL ||
        sprite->pixels.size != (size_t)sprite->width * sprite->height / 2 || sprite->width % 4 != 0)
        return SDL_SetError("Physical-death sprite has invalid packed pixel dimensions");
    const double angle = (double)death.rotation * 6.283185307179586 / 4096.0;
    const Sint32 cosine = (Sint32)SDL_lround(SDL_cos(angle) * 4096.0);
    const Sint32 sine = (Sint32)SDL_lround(SDL_sin(angle) * 4096.0);
    for (Sint32 py = 0; py < SOTN_FRAME_HEIGHT; ++py)
        for (Sint32 px = 0; px < SOTN_FRAME_WIDTH; ++px)
        {
            const Sint64 dx = (Sint64)px - x, dy = (Sint64)py - y;
            const Sint64 local_x = (dx * cosine + dy * sine) / 4096;
            const Sint64 local_y = (-dx * sine + dy * cosine) / 4096;
            const Sint64 sx = (facing_left ? -local_x : local_x) - sprite->pivot_x;
            const Sint64 sy = local_y - sprite->pivot_y;
            if (sx < 0 || sy < 0 || sx >= sprite->width || sy >= sprite->height)
                continue;
            const Uint32 cell = ((Uint32)sy & 15) * 16 + ((Uint32)sx & 15);
            if (death.mask[cell / 8] & (1U << (cell & 7)))
                continue;
            const size_t pixel = (size_t)sy * sprite->width + (size_t)sx;
            const Uint8 index = (sprite->pixels.bytes[pixel / 2] >> ((pixel & 1) * 4)) & 15;
            const Uint16 color = Read16(common.bytes + 0x40000 + (size_t)index * 2);
            if (color != 0)
            {
                const Uint32 red = SDL_min(31U, (color & 31U) * death.red / 128);
                const Uint32 green = ((color >> 5) & 31U) * death.blue / 128;
                const Uint32 blue = ((color >> 10) & 31U) * death.blue / 128;
                frame[(size_t)py * SOTN_FRAME_WIDTH + (size_t)px] =
                    (Uint16)((color & 0x8000) | red | (green << 5) | (blue << 10));
            }
        }
    return true;
}

bool SotnAlucard_CapePalette(SotnDataView dra, Uint16 cape, Uint16 colors[4])
{
    if (dra.bytes == NULL || dra.size < 0x3be0 || colors == NULL)
        return SDL_SetError("Cape palette requires bounded private tables and output");
    Uint32 operation = 0;
    bool found = false;
    for (Uint32 i = 0; i < 10; ++i)
    {
        const Uint32 id = Read32(dra.bytes + 0x2fbc + i * 8);
        if (id == cape || id == SDL_MAX_UINT32)
        {
            operation = Read32(dra.bytes + 0x2fc0 + i * 8) & 255;
            found = true;
            break;
        }
    }
    if (!found || operation >= 48 || 0x3bb8 + operation * 4U > dra.size - 4)
        return SDL_SetError("Cape palette table has no valid bounded operation");
    const Uint32 address = Read32(dra.bytes + 0x3bb8 + operation * 4);
    if (address < 0x800a0000 || (address & 3) || address - 0x800a0000 > dra.size - 16)
        return SDL_SetError("Cape palette operation references invalid private data");
    const Uint8 *op = dra.bytes + (address - 0x800a0000);
    const Uint32 destination = Read32(op + 4), count = Read32(op + 8);
    if (Read32(op) != 1 || destination < 0x1006 || destination > 0x1009 || count == 0 ||
        count > 0x100a - destination)
        return SDL_SetError("Cape palette requires a bounded original cape-color copy");
    const Uint32 source = Read32(op + 12);
    if (source < 0x800a0000 || (source & 1) || source - 0x800a0000 > dra.size - count * 2U)
        return SDL_SetError("Cape colors reference invalid private data");
    for (Uint32 i = 0; i < count; ++i)
        colors[destination - 0x1006 + i] = Read16(dra.bytes + source - 0x800a0000 + i * 2);
    return true;
}

// dra/4A538.c:PAL_GLOW_ANIM, with the original one-component-step convergence.
bool SotnAlucard_TwilightTick(SotnDataView dra, Uint16 colors[4], Uint16 *target, Uint8 *delay)
{
    if (dra.bytes == NULL || dra.size < 0x3c10 || colors == NULL || target == NULL ||
        delay == NULL || *target > 1 || *delay > 1)
        return SDL_SetError(
            "Twilight palette requires bounded private data and an initialized cursor");
    const Uint32 address = Read32(dra.bytes + 0x3bb8 + 21 * 4U);
    if (address < 0x800a0000 || address % 4 || address - 0x800a0000 > dra.size - 24)
        return SDL_SetError("Twilight palette operation is outside private DRA");
    const Uint8 *op = dra.bytes + address - 0x800a0000;
    if (Read32(op) != 0x20004 || Read32(op + 4) != 0x1006 || Read32(op + 8) != 2 ||
        Read32(op + 20) != SDL_MAX_UINT32)
        return SDL_SetError("Twilight palette does not match its original glow operation");
    const Uint32 source = Read32(op + 12 + *target * 4U);
    if (source < 0x800a0000 || source % 2 || source - 0x800a0000 > dra.size - 4)
        return SDL_SetError("Twilight glow target is outside private DRA");
    if (*delay)
    {
        --*delay;
        return true;
    }
    bool changed = false;
    for (Uint32 i = 0; i < 2; ++i)
    {
        const Uint16 destination = Read16(dra.bytes + source - 0x800a0000 + i * 2U);
        Uint16 color = colors[i];
        for (Uint32 shift = 0; shift <= 10; shift += 5)
        {
            const Sint32 old = (color >> shift) & 31, wanted = (destination >> shift) & 31;
            const Sint32 next = old + (wanted > old ? 1 : wanted < old ? -1 : 0);
            color = (Uint16)((color & (Uint16) ~(31U << shift)) | (Uint16)(next << shift));
        }
        changed |= color != colors[i];
        colors[i] = color;
    }
    if (!changed)
        *target ^= 1;
    *delay = 1;
    return true;
}

bool SotnAlucard_DrawPetrified(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x,
                               Sint32 y, bool facing_left, SotnPlayerStone stone, Uint16 *frame,
                               size_t count)
{
    if (sprite == NULL || frame == NULL || count < SOTN_FRAME_PIXELS || !stone.active ||
        stone.palette < 0x100 || stone.palette >= 0x200 || SDL_abs(stone.rotation) > 32 ||
        stone.pivot < 0 || stone.pivot > 24 || common.bytes == NULL || common.size != 0x42000)
        return SDL_SetError("Stone rendering requires bounded palette, transform and output");
    if (stone.hidden || (sprite->width == 0 && sprite->height == 0))
        return true;
    if (sprite->width == 0 || sprite->width > 255 || sprite->height == 0 || sprite->height > 255 ||
        sprite->width % 4 || sprite->pixels.bytes == NULL ||
        sprite->pixels.size != (size_t)sprite->width * sprite->height / 2)
        return SDL_SetError("Stone rendering requires a valid packed Alucard sprite");
    const double angle = stone.rotation * (SDL_PI_D / 2048.0);
    const Sint32 cosine = (Sint32)SDL_lround(SDL_cos(angle) * 4096.0);
    const Sint32 sine = (Sint32)SDL_lround(SDL_sin(angle) * 4096.0);
    const size_t palette = 0x40000 + (size_t)(stone.palette - 0x100) * 32;
    for (Sint32 py = 0; py < SOTN_FRAME_HEIGHT; ++py)
        for (Sint32 px = 0; px < SOTN_FRAME_WIDTH; ++px)
        {
            const Sint64 dx = (Sint64)px - x, dy = (Sint64)py - y - stone.pivot;
            const Sint64 local_x = (dx * cosine + dy * sine) / 4096;
            const Sint64 local_y = (-dx * sine + dy * cosine) / 4096 + stone.pivot;
            const Sint64 sx = (facing_left ? -local_x : local_x) - sprite->pivot_x;
            const Sint64 sy = local_y - sprite->pivot_y;
            if (sx < 0 || sy < 0 || sx >= sprite->width || sy >= sprite->height)
                continue;
            const size_t pixel = (size_t)sy * sprite->width + (size_t)sx;
            const Uint8 index = (sprite->pixels.bytes[pixel / 2] >> ((pixel & 1U) * 4)) & 15;
            const Uint16 color = Read16(common.bytes + palette + index * 2U);
            if (color)
                frame[(size_t)py * SOTN_FRAME_WIDTH + (size_t)px] = color;
        }
    return true;
}
