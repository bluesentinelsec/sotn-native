// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/dra/4AEA4.c:LoadPendingGfx, 4A538.c bulk palettes, and 4B758.c multipart sprites.
#include <SDL3/SDL_error.h>
#include <sotn/actor_graphics.h>
#include <sotn/decompress.h>

#define VRAM_WORDS (1024U * 512U)
#define UPLOAD_BUDGET (2U * 1024U * 1024U)

struct SotnActorGraphics
{
    SotnDataView dra, nz0;
    SotnDataView weapon[2];
    Uint16 words[VRAM_WORDS];
    Uint8 valid[VRAM_WORDS];
};

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Sint32 Signed16(const Uint8 *bytes)
{
    const Uint16 bits = Read16(bytes);
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}

static Uint32 Read32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static bool Resolve(SotnDataView image, Uint32 base, Uint32 address, size_t size, Uint32 alignment,
                    SotnDataView *out)
{
    if (image.bytes == NULL || address < base || address % alignment != 0 ||
        address - base > image.size || size > image.size - (address - base))
        return SDL_SetError("Actor graphics contain an invalid or truncated PS1 data pointer");
    *out = (SotnDataView){image.bytes + (address - base), size};
    return true;
}

static void Upload(SotnActorGraphics *graphics, Uint32 x, Uint32 y, Uint32 width, Uint32 height,
                   const Uint8 *pixels)
{
    for (Uint32 row = 0; row < height; ++row)
        for (Uint32 column = 0; column < width; ++column)
        {
            const size_t destination = (size_t)(y + row) * 1024 + x + column;
            graphics->words[destination] = Read16(pixels + ((size_t)row * width + column) * 2);
            graphics->valid[destination] = 1;
        }
}

static void RawTiles(SotnActorGraphics *graphics, SotnDataView image, Uint32 start_y)
{
    for (Uint32 block = 0; block < 32; ++block)
    {
        const Uint32 x = 512 + (block / 4) * 64 + (block & 1) * 32;
        const Uint32 y = start_y + ((block / 2) & 1) * 128;
        Upload(graphics, x, y, 32, 128, image.bytes + (size_t)block * 8192);
    }
}

static bool Palettes(SotnActorGraphics *graphics)
{
    SotnDataView table = {0}, descriptor = {0};
    if (!Resolve(graphics->nz0, 0x80180000, Read32(graphics->nz0.bytes + 24), 4, 4, &table) ||
        !Resolve(graphics->nz0, 0x80180000, Read32(table.bytes), 4, 4, &descriptor))
        return false;
    if (Read32(descriptor.bytes) != 5)
        return SDL_SetError("Actor palettes require the original Laboratory bulk-copy operation");
    const Uint32 address = Read32(table.bytes);
    for (Uint32 i = 0; i < 256; ++i)
    {
        if (!Resolve(graphics->nz0, 0x80180000, address + 4 + i * 12, 4, 4, &descriptor))
            return false;
        const Uint32 start = Read32(descriptor.bytes);
        if (start == SDL_MAX_UINT32)
            return true;
        if (!Resolve(graphics->nz0, 0x80180000, address + 4 + i * 12, 12, 4, &descriptor))
            return false;
        const Uint32 count = Read32(descriptor.bytes + 4);
        SotnDataView colors = {0};
        if (start >= 0x3000 || count == 0 || count > 0x3000 - start ||
            !Resolve(graphics->nz0, 0x80180000, Read32(descriptor.bytes + 8), count * 2, 2,
                     &colors))
            return SDL_SetError("Actor palette bulk copy exceeds the original palette memory");
        for (Uint32 j = 0; j < count; ++j)
        {
            const Uint32 index = start + j;
            const Uint32 bank = index / 0x1000;
            const Uint32 x = (bank == 0 ? 512U : (bank - 1) * 256U) + (index & 255);
            Upload(graphics, x, 240 + (index % 0x1000) / 256, 1, 1, colors.bytes + (size_t)j * 2);
        }
    }
    return SDL_SetError("Actor palette operation has no bounded terminator");
}

bool SotnActorGraphics_Create(SotnDataView dra, SotnDataView nz0, SotnTileSources sources,
                              SotnActorGraphics **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (dra.bytes == NULL || dra.size > 0x200000 || nz0.bytes == NULL || nz0.size < 40 ||
        nz0.size > 0x80000 || sources.stage.bytes == NULL || sources.stage.size != 0x40000 ||
        sources.common.bytes == NULL || sources.common.size != 0x42000)
        return SDL_SetError(
            "Actor graphics require bounded DRA/stage overlay and verified texture views");
    SotnActorGraphics *graphics = SDL_calloc(1, sizeof(*graphics));
    if (graphics == NULL)
        return false;
    graphics->dra = dra;
    graphics->nz0 = nz0;
    RawTiles(graphics, sources.stage, 0);
    RawTiles(graphics, sources.common, 256);
    Upload(graphics, 0, 240, 256, 16, sources.common.bytes + 0x40000);
    if (!Palettes(graphics))
    {
        SotnActorGraphics_Destroy(graphics);
        return false;
    }
    *out = graphics;
    return true;
}

typedef struct Transfer
{
    Uint32 x, y, width, height;
    size_t offset;
} Transfer;

bool SotnActorGraphics_LoadPortrait(SotnActorGraphics *g, Uint32 offset, Uint16 slot)
{
    if (g == NULL || slot > 1 || offset > g->nz0.size || 48 * 72 > g->nz0.size - offset)
        return SDL_SetError("Dialogue portrait slot or private pixels exceed validated bounds");
    Upload(g, 256U + slot * 32U, 256, 24, 72, g->nz0.bytes + offset);
    return true;
}

bool SotnActorGraphics_LoadBank(SotnActorGraphics *graphics, Uint32 bank)
{
    if (graphics == NULL || bank >= 25)
        return SDL_SetError("Actor graphics require a stage bank index below 25");
    SotnDataView table = {0}, record = {0};
    if (!Resolve(graphics->nz0, 0x80180000, Read32(graphics->nz0.bytes + 36), 25 * 4, 4, &table) ||
        !Resolve(graphics->nz0, 0x80180000, Read32(table.bytes + (size_t)bank * 4), 4, 4, &record))
        return false;
    const Uint32 address = Read32(table.bytes + (size_t)bank * 4);
    const Uint32 kind = Read32(record.bytes);
    if (kind == 0 || kind == SDL_MAX_UINT32)
        return true;
    if (kind > 4)
        return SDL_SetError("Actor graphics bank uses an unsupported upload kind");
    Uint8 *scratch = SDL_malloc(UPLOAD_BUDGET);
    if (scratch == NULL)
        return false;
    Transfer transfers[64] = {0};
    size_t used = 0;
    Uint32 count = 0;
    bool complete = false;
    for (; count < SDL_arraysize(transfers); ++count)
    {
        if (!Resolve(graphics->nz0, 0x80180000, address + 4 + count * 12, 4, 4, &record))
            break;
        const Uint32 xy = Read32(record.bytes);
        if (xy == SDL_MAX_UINT32)
        {
            complete = true;
            break;
        }
        if (!Resolve(graphics->nz0, 0x80180000, address + 4 + count * 12, 12, 4, &record))
            break;
        const Uint32 wh = Read32(record.bytes + 4);
        const Uint32 x = xy >> 16, y = xy & 65535;
        const Uint32 width = wh >> 16, height = wh & 65535;
        const Uint32 divisor = kind == 3 ? 1U : kind == 2 ? 2U : 4U;
        const Uint32 words = width / divisor;
        const size_t size = (size_t)words * height * 2;
        SotnDataView source = {0};
        if (width == 0 || height == 0 || width % divisor != 0 || x >= 1024 || y >= 512 ||
            words > 1024 - x || height > 512 - y || size > UPLOAD_BUDGET - used ||
            !Resolve(graphics->nz0, 0x80180000, Read32(record.bytes + 8), kind == 4 ? 8 : size, 1,
                     &source))
        {
            SDL_SetError("Actor graphics upload exceeds texture or scratch bounds");
            break;
        }
        if (kind == 4)
        {
            source.size = graphics->nz0.size - (size_t)(source.bytes - graphics->nz0.bytes);
            size_t written = 0;
            if (!SotnGraphics_Decompress(source, scratch + used, size, &written))
                break;
            if (written != size)
            {
                SDL_SetError("Actor graphics stream does not match its upload dimensions");
                break;
            }
        }
        else
            SDL_memcpy(scratch + used, source.bytes, size);
        transfers[count] = (Transfer){x, y, words, height, used};
        used += size;
    }
    if (complete)
        for (Uint32 i = 0; i < count; ++i)
            Upload(graphics, transfers[i].x, transfers[i].y, transfers[i].width,
                   transfers[i].height, scratch + transfers[i].offset);
    else if (count == SDL_arraysize(transfers))
        SDL_SetError("Actor graphics bank exceeds its transfer budget");
    SDL_free(scratch);
    return complete;
}

bool SotnActorGraphics_LoadWeapon(SotnActorGraphics *graphics, SotnDataView weapons, Uint32 hand,
                                  SotnEquipment equipment)
{
    if (graphics == NULL || hand > 1)
        return SDL_SetError("Weapon graphics require a hand index of zero or one");
    if (equipment.id == 0)
    {
        graphics->weapon[hand] = (SotnDataView){0};
        return true;
    }
    SotnDataView overlay = {0}, pointer = {0}, colors = {0};
    if (!SotnWeapon_Overlay(weapons, equipment, &overlay))
        return false;
    const Uint32 address = equipment.family == 0    ? 0x8017b0bcU
                           : equipment.family == 31 ? 0x8017a90cU
                                                    : 0x8017abd4U;
    if (!Resolve(overlay, 0x8017a000, address, 4, 4, &pointer) ||
        !Resolve(overlay, 0x8017a000, Read32(pointer.bytes), 7 * 16 * 2, 2, &colors))
        return false;
    const size_t pixels = (size_t)equipment.family * 0x7000;
    if (pixels > weapons.size || 0x4000 > weapons.size - pixels)
        return SDL_SetError("Weapon texture is truncated");
    Upload(graphics, 576, 256 + hand * 128, 64, 128, weapons.bytes + pixels);
    for (Uint32 i = 0; i < 7 * 16; ++i)
    {
        const Uint32 index = 0x1100 + hand * 0x180 + i;
        Upload(graphics, index & 255, 240 + (index - 0x1000) / 256, 1, 1, colors.bytes + i * 2);
    }
    graphics->weapon[hand] = overlay;
    return true;
}

static bool Frame(const SotnActorGraphics *graphics, SotnActorSprite sprite, SotnDataView *out)
{
    const bool stage = (sprite.animation_set & 0x8000) != 0;
    const Uint32 bank = sprite.animation_set & 0x7fff;
    const bool weapon = !stage && (bank == 16 || bank == 18);
    const SotnDataView image = weapon  ? graphics->weapon[bank == 18 ? 1 : 0]
                               : stage ? graphics->nz0
                                       : graphics->dra;
    const Uint32 base = weapon ? 0x8017a000U : stage ? 0x80180000U : 0x800a0000U;
    SotnDataView table = {0}, first = {0};
    if (bank == 0 || (!weapon && bank >= (stage ? 24U : 18U)) ||
        (!stage && (bank == 1 || bank == 13)))
        return SDL_SetError("Actor sprite bank is outside the supported multipart path");
    const Uint32 table_address = weapon  ? base + 0x40
                                 : stage ? Read32(graphics->nz0.bytes + 20)
                                         : base + 0x3b70;
    if (!weapon && !Resolve(image, base, table_address, (stage ? 24U : 18U) * 4, 4, &table))
        return false;
    const Uint32 address = weapon ? table_address : Read32(table.bytes + bank * 4);
    if (!Resolve(image, base, address, 8, 4, &table))
        return false;
    const Uint32 first_address = Read32(table.bytes + 4);
    if (first_address <= address || first_address % 4 != 0 ||
        (first_address - address) / 4 > 1024 || sprite.frame >= (first_address - address) / 4)
        return SDL_SetError("Actor sprite frame exceeds its bounded pointer table");
    if (!Resolve(image, base, address, first_address - address, 4, &table) ||
        !Resolve(image, base, Read32(table.bytes + (size_t)sprite.frame * 4), 2, 2, &first))
        return false;
    const Uint16 parts = Read16(first.bytes);
    if (parts > 128)
        return SDL_SetError("Actor sprite does not use a bounded multipart definition");
    return Resolve(image, base, Read32(table.bytes + (size_t)sprite.frame * 4),
                   2 + (size_t)parts * 22, 2, out);
}

static Uint16 Blend(Uint16 back, Uint16 front, Uint32 mode)
{
    Uint16 color = 0;
    for (unsigned int shift = 0; shift < 15; shift += 5)
    {
        const Sint32 b = (back >> shift) & 31, f = (front >> shift) & 31;
        const Sint32 channel = mode == 0   ? b / 2 + f / 2
                               : mode == 1 ? b + f
                               : mode == 2 ? b - f
                                           : b + f / 4;
        color |= (Uint16)((Uint16)SDL_clamp(channel, 0, 31) << shift);
    }
    return color;
}

static bool Word(const SotnActorGraphics *graphics, Uint32 x, Uint32 y, Uint16 *out)
{
    const size_t index = (size_t)(y & 511) * 1024 + (x & 1023);
    if (!graphics->valid[index])
        return SDL_SetError(
            "Actor sprite references texture or palette data not loaded for this room");
    *out = graphics->words[index];
    return true;
}

static bool Texel(const SotnActorGraphics *graphics, Uint32 page, Uint32 palette, Uint8 u, Uint8 v,
                  Uint16 *color)
{
    const Uint32 depth = (page >> 7) & 3;
    if (depth == 3 || palette >= 768)
        return SDL_SetError("Actor sprite texture depth or palette index is invalid");
    const Uint32 divisor = depth == 0 ? 4U : depth == 1 ? 2U : 1U;
    Uint16 packed = 0;
    if (!Word(graphics, (page & 15) * 64 + u / divisor, (page & 16) * 16 + v, &packed))
        return false;
    if (depth == 2)
    {
        *color = packed;
        return true;
    }
    const Uint32 index =
        depth == 0 ? (packed >> ((u & 3) * 4)) & 15 : (packed >> ((u & 1) * 8)) & 255;
    const Uint32 bank = palette / 256;
    const Uint32 x = (bank == 0 ? 512U : (bank - 1) * 256U) + (palette & 15) * 16 + index;
    return Word(graphics, x, 240 + (palette % 256) / 16, color);
}

static bool QuadDraw(const SotnActorGraphics *graphics, SotnTextureQuad q, Uint16 *frame,
                     size_t count, bool write);
static bool Part(const SotnActorGraphics *graphics, SotnActorSprite sprite, const Uint8 *part,
                 Sint32 x, Sint32 y, Uint16 *frame, bool write,
                 const SotnSpriteTransform *transform)
{
    const Uint32 flags = Read16(part);
    Sint32 pivot_x = Signed16(part + 2), pivot_y = Signed16(part + 4);
    Sint32 width = Signed16(part + 6), height = Signed16(part + 8);
    if ((flags & ~0x13fU) != 0 || width <= 0 || width > 512 || height <= 0 || height > 512 ||
        (sprite.blend_mode & ~0x70U) != 0)
        return SDL_SetError("Actor sprite has unsupported part flags or dimensions");
    if (flags & 4)
    {
        --width;
        if (flags & 2)
            ++pivot_x;
    }
    if (flags & 8)
    {
        --height;
        if (flags & 1)
            ++pivot_y;
    }
    if (flags & 16)
    {
        --width;
        if (!(flags & 2))
            ++pivot_x;
    }
    if (flags & 32)
    {
        --height;
        if (!(flags & 1))
            ++pivot_y;
    }
    if (width <= 0 || height <= 0)
        return SDL_SetError("Actor sprite cropping removes its whole part");
    const Uint32 texture = (Uint32)Read16(part + 12) + sprite.texture_offset;
    const Uint32 page = texture >> 2;
    Sint32 u0 = Read16(part + 14) + ((texture & 1) ? 128 : 0);
    Sint32 v0 = Read16(part + 16) + ((texture & 2) ? 128 : 0);
    Sint32 u1 = Read16(part + 18) + ((texture & 1) ? 128 : 0);
    Sint32 v1 = Read16(part + 20) + ((texture & 2) ? 128 : 0);
    if (flags & 4)
        --u1;
    if (flags & 8)
        --v1;
    if (flags & 16)
        ++u0;
    if (flags & 32)
        ++v0;
    if (((flags & 2) != 0) != sprite.facing_left)
    {
        const Sint32 old = u0;
        u0 = u1 - 1;
        u1 = old - 1;
    }
    if (flags & 1)
    {
        const Sint32 old = v0;
        v0 = v1 - 1;
        v1 = old - 1;
    }
    u0 = (Uint8)u0;
    u1 = (Uint8)u1;
    v0 = (Uint8)v0;
    v1 = (Uint8)v1;
    const Sint64 left = sprite.facing_left ? (Sint64)x - pivot_x - width + 1 : (Sint64)x + pivot_x;
    const Sint64 top = (Sint64)y + pivot_y;
    const Uint32 palette = (sprite.palette & 0x8000) ? sprite.palette & 0x7fff
                                                     : (Uint32)sprite.palette + Read16(part + 10);
    if (transform != NULL)
    {
        if (x < -65536 || x > 65536 || y < -65536 || y > 65536)
            return true;
        const double radians = transform->rotation * (SDL_PI_D / 2048.0);
        const double sine = SDL_sin(radians), cosine = SDL_cos(radians);
        const Sint32 left_local = sprite.facing_left ? -pivot_x - width + 1 : pivot_x;
        const Sint32 right_local = left_local + width;
        const Sint32 bottom_local = pivot_y + height;
        SotnTextureQuad q = {.u = {(Uint8)u0, (Uint8)u1, (Uint8)u0, (Uint8)u1},
                             .v = {(Uint8)v0, (Uint8)v0, (Uint8)v1, (Uint8)v1},
                             .page = (Uint16)(page + (sprite.blend_mode & 0x60)),
                             .modulate = true,
                             .opacity = (Uint8)transform->opacity,
                             .palette = (Uint16)palette,
                             .blend = sprite.blend_mode != 0};
        if (u1 < u0)
            for (Uint32 i = 0; i < 4; ++i)
                ++q.u[i];
        if (v1 < v0)
            for (Uint32 i = 0; i < 4; ++i)
                ++q.v[i];
        const Sint32 xs[3] = {left_local, right_local, left_local};
        const Sint32 ys[3] = {pivot_y, pivot_y, bottom_local};
        for (Uint32 i = 0; i < 3; ++i)
        {
            q.x[i] = x + (Sint32)SDL_round((xs[i] * transform->scale_x / 256.0) * cosine -
                                           (ys[i] * transform->scale_y / 256.0) * sine);
            q.y[i] = y + (Sint32)SDL_round((xs[i] * transform->scale_x / 256.0) * sine +
                                           (ys[i] * transform->scale_y / 256.0) * cosine);
        }
        q.x[3] = q.x[1] + q.x[2] - q.x[0];
        q.y[3] = q.y[1] + q.y[2] - q.y[0];
        Sint32 qleft = q.x[0], qright = q.x[0], qtop = q.y[0], qbottom = q.y[0];
        for (Uint32 i = 1; i < 4; ++i)
        {
            qleft = SDL_min(qleft, q.x[i]);
            qright = SDL_max(qright, q.x[i]);
            qtop = SDL_min(qtop, q.y[i]);
            qbottom = SDL_max(qbottom, q.y[i]);
        }
        if (qright <= 0 || qleft >= SOTN_FRAME_WIDTH || qbottom <= 0 || qtop >= SOTN_FRAME_HEIGHT)
            return true;
        return QuadDraw(graphics, q, frame, SOTN_FRAME_PIXELS, write);
    }
    for (Sint32 row = 0; row < height; ++row)
    {
        const Sint64 py = top + row;
        if (py < 0 || py >= SOTN_FRAME_HEIGHT)
            continue;
        for (Sint32 column = 0; column < width; ++column)
        {
            const Sint64 px = left + column;
            if (px < 0 || px >= SOTN_FRAME_WIDTH)
                continue;
            const Uint8 u = (Uint8)(u0 + column * (u1 - u0) / width);
            const Uint8 v = (Uint8)(v0 + row * (v1 - v0) / height);
            Uint16 color = 0;
            if (!Texel(graphics, page, palette, u, v, &color))
                return false;
            if (write && color != 0)
            {
                const size_t pixel = (size_t)py * SOTN_FRAME_WIDTH + (size_t)px;
                frame[pixel] =
                    sprite.blend_mode != 0 && (color & 0x8000)
                        ? Blend(frame[pixel], color, ((page + (sprite.blend_mode & 0x60)) >> 5) & 3)
                        : color;
            }
        }
    }
    return true;
}

static bool Draw(const SotnActorGraphics *graphics, SotnActorSprite sprite, Sint32 x, Sint32 y,
                 Uint16 *frame, size_t count, const SotnSpriteTransform *transform)
{
    if (graphics == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Actor drawing requires initialized graphics and a full framebuffer");
    if (sprite.frame == 0 || sprite.animation_set == 0)
        return true;
    SotnDataView definition = {0};
    if (!Frame(graphics, sprite, &definition))
        return false;
    const Uint32 parts = Read16(definition.bytes);
    for (unsigned int pass = 0; pass < 2; ++pass)
        for (int priority = 1; priority >= 0; --priority)
            for (Uint32 i = parts; i > 0; --i)
            {
                const Uint8 *part = definition.bytes + 2 + (size_t)(i - 1) * 22;
                if (((Read16(part) & 0x100) != 0) != (priority != 0))
                    continue;
                if (!Part(graphics, sprite, part, x, y, frame, pass != 0, transform))
                    return false;
            }
    return true;
}

void SotnActorGraphics_Destroy(SotnActorGraphics *graphics)
{
    SDL_free(graphics);
}

static bool QuadDraw(const SotnActorGraphics *graphics, SotnTextureQuad q, Uint16 *frame,
                     size_t count, bool write)
{
    if (graphics == NULL || frame == NULL || count < SOTN_FRAME_PIXELS || q.page > 0x1ff ||
        ((q.page >> 7) & 3) == 3 || q.palette >= 768)
        return SDL_SetError("Projectile quad requires bounded texture attributes and a full frame");
    Sint32 left = 2048, right = -2048, top = 2048, bottom = -2048;
    for (Uint32 i = 0; i < 4; ++i)
    {
        if (q.u[i] > 256 || q.v[i] > 256)
            return SDL_SetError("Projectile quad UV exceeds its texture page");
        if (q.x[i] < -2048 || q.x[i] > 2048 || q.y[i] < -2048 || q.y[i] > 2048)
            return SDL_SetError("Projectile quad exceeds the bounded screen margin");
        left = SDL_min(left, q.x[i]);
        right = SDL_max(right, q.x[i]);
        top = SDL_min(top, q.y[i]);
        bottom = SDL_max(bottom, q.y[i]);
    }
    if (q.x[3] != q.x[1] + q.x[2] - q.x[0] || q.y[3] != q.y[1] + q.y[2] - q.y[0])
        return SDL_SetError("Projectile quad must be affine");
    const double ax = q.x[1] - q.x[0], ay = q.y[1] - q.y[0];
    const double bx = q.x[2] - q.x[0], by = q.y[2] - q.y[0];
    const double determinant = ax * by - ay * bx;
    if (determinant == 0)
        return true;
    for (Uint32 pass = 0; pass < 2; ++pass)
        for (Sint32 y = SDL_max(0, top); y < SDL_min(SOTN_FRAME_HEIGHT, bottom); ++y)
            for (Sint32 x = SDL_max(0, left); x < SDL_min(SOTN_FRAME_WIDTH, right); ++x)
            {
                const double dx = x + 0.5 - q.x[0], dy = y + 0.5 - q.y[0];
                const double a = (dx * by - dy * bx) / determinant;
                const double b = (ax * dy - ay * dx) / determinant;
                if (a < 0 || a >= 1 || b < 0 || b >= 1)
                    continue;
                const double u = q.u[0] + a * (q.u[1] - q.u[0]) + b * (q.u[2] - q.u[0]);
                const double v = q.v[0] + a * (q.v[1] - q.v[0]) + b * (q.v[2] - q.v[0]);
                Uint16 color = 0;
                if (u < 0 || u >= 256 || v < 0 || v >= 256 ||
                    !Texel(graphics, q.page, q.palette, (Uint8)u, (Uint8)v, &color))
                    return SDL_SetError("Projectile texture reference is invalid: %s",
                                        SDL_GetError());
                if (write && pass != 0 && color != 0)
                {
                    if (q.modulate || q.tint)
                    {
                        Uint16 tinted = color & 0x8000;
                        for (Uint32 shift = 0; shift < 15; shift += 5)
                            tinted |= (Uint16)(SDL_min(31, ((color >> shift) & 31) *
                                                               (q.tint ? (shift == 0   ? q.red
                                                                          : shift == 5 ? q.green
                                                                                       : q.blue)
                                                                       : q.opacity) /
                                                               128)
                                               << shift);
                        color = tinted;
                    }
                    const size_t pixel = (size_t)y * SOTN_FRAME_WIDTH + (size_t)x;
                    frame[pixel] = q.blend && (color & 0x8000)
                                       ? Blend(frame[pixel], color, (q.page >> 5) & 3)
                                       : color;
                }
            }
    return true;
}

bool SotnActorGraphics_Draw(const SotnActorGraphics *graphics, SotnActorSprite sprite, Sint32 x,
                            Sint32 y, Uint16 *frame, size_t count)
{
    return Draw(graphics, sprite, x, y, frame, count, NULL);
}
bool SotnActorGraphics_DrawRotated(const SotnActorGraphics *graphics, SotnActorSprite sprite,
                                   Sint32 x, Sint32 y, Sint16 angle, Uint16 *frame, size_t count)
{
    const SotnSpriteTransform transform = {angle, 256, 256, 128};
    return Draw(graphics, sprite, x, y, frame, count, (angle & 4095) == 0 ? NULL : &transform);
}
bool SotnActorGraphics_Quad(const SotnActorGraphics *graphics, SotnTextureQuad q, Uint16 *frame,
                            size_t count)
{
    return QuadDraw(graphics, q, frame, count, true);
}

bool SotnActorGraphics_DrawTransform(const SotnActorGraphics *g, SotnActorSprite sprite, Sint32 x,
                                     Sint32 y, SotnSpriteTransform transform, Uint16 *frame,
                                     size_t count)
{
    if (transform.scale_x == 0 || transform.scale_y == 0 || transform.scale_x > 1024 ||
        transform.scale_y > 1024 || transform.opacity > 255)
        return SDL_SetError("Actor transform exceeds the bounded original scales or color");
    if (transform.rotation == 0 && transform.scale_x == 256 && transform.scale_y == 256 &&
        transform.opacity == 128)
        return Draw(g, sprite, x, y, frame, count, NULL);
    return Draw(g, sprite, x, y, frame, count, &transform);
}
