// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/dra/4AEA4.c:LoadPendingGfx, 4A538.c bulk palettes, and 4B758.c multipart sprites.
#include <SDL3/SDL_error.h>
#include <sotn/actor_graphics.h>
#include <sotn/decompress.h>
#include <sotn/stage_profile.h>

#define VRAM_WORDS (1024U * 512U)
#define UPLOAD_BUDGET (2U * 1024U * 1024U)

struct SotnActorGraphics
{
    SotnDataView dra, nz0;
    Uint16 sprite_banks, stage_id;
    SotnDataView weapon[2];
    Uint16 words[VRAM_WORDS];
    Uint8 valid[VRAM_WORDS];
    bool dopple_cached, dopple_death_cached;
    Uint16 dopple_death_words[1024], dopple_death_frame, dopple_death_bank;
    Sint16 bat_pivot[18];
    bool bat_pivot_set[18];
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
    return SotnActorGraphics_CreateStage(dra, nz0, sources, 12, out);
}

bool SotnActorGraphics_CreateStage(SotnDataView dra, SotnDataView nz0, SotnTileSources sources,
                                   Uint16 stage, SotnActorGraphics **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    const SotnStageProfile *profile = SotnStageProfile_Get(stage);
    if (profile == NULL || dra.bytes == NULL || dra.size > 0x200000 || nz0.bytes == NULL ||
        nz0.size < 40 || nz0.size > 0x80000 || sources.stage.bytes == NULL ||
        sources.stage.size != 0x40000 || sources.common.bytes == NULL ||
        sources.common.size != 0x42000)
        return SDL_SetError(
            "Actor graphics require bounded DRA/stage overlay and verified texture views");
    SotnActorGraphics *graphics = SDL_calloc(1, sizeof(*graphics));
    if (graphics == NULL)
        return false;
    graphics->dra = dra;
    graphics->nz0 = nz0;
    graphics->sprite_banks = profile->sprite_banks;
    graphics->stage_id = stage;
    RawTiles(graphics, sources.stage, 0);
    RawTiles(graphics, sources.common, 256);
    Upload(graphics, 0, 240, 256, 16, sources.common.bytes + 0x40000);
    // Play_Init loads default bank zero independently from room-specific graphics.
    // Boss arenas and Library/Chapel base artwork require that initial residency.
    if (!Palettes(graphics) ||
        ((stage == 25 || stage == 26 || stage == 2 || stage == 4 || stage == 6) &&
         !SotnActorGraphics_LoadBank(graphics, 0)))
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

bool SotnActorGraphics_PrepareOuterRope(SotnActorGraphics *g)
{
    if (g == NULL)
        return SDL_InvalidParamError("graphics");
    for (Uint32 row = 0; row < 16; ++row)
        for (Uint32 col = 0; col < 2; ++col)
            if (!g->valid[(192 + row) * 1024U + 958 + col])
                return SDL_SetError("Outer Wall rope cache needs its original uploaded texture");
    // Original clipped 16-bit draw-environment copies, expressed as a CPU texture upload.
    for (Uint32 row = 0; row < 256; ++row)
        for (Uint32 col = 0; col < 32; ++col)
        {
            const size_t dest = (256 + row) * 1024U + col;
            g->words[dest] = col < 2 ? g->words[(192 + row % 16) * 1024U + 958 + col] : 0;
            g->valid[dest] = 1;
        }
    return true;
}

bool SotnActorGraphics_LoadPortrait(SotnActorGraphics *g, Uint32 offset, Uint16 slot)
{
    if (g == NULL || slot > 1 || offset > g->nz0.size || 48 * 72 > g->nz0.size - offset)
        return SDL_SetError("Dialogue portrait slot or private pixels exceed validated bounds");
    Uint32 x = (g->stage_id == 6 ? 0U : g->stage_id == 2 ? 64U : 256U) + slot * 32U;
    if (g->stage_id == 25)
    {
        if (g->nz0.size < 0xb88)
            return SDL_SetError("BO5 portrait upload table is truncated");
        x = Read16(g->nz0.bytes + 0xb84 + slot * 2U);
        if (x > 1000)
            return SDL_SetError("BO5 portrait upload exceeds texture memory");
    }
    Upload(g, x, 256, 24, 72, g->nz0.bytes + offset);
    return true;
}

bool SotnActorGraphics_LoadDialogue(SotnActorGraphics *g, SotnDialogueState d)
{
    if (!g || g->stage_id != 25 || !d.primitive_text || d.row >= 5 || d.column > 24)
        return SDL_SetError("BO5 dialogue cache requires its stage owner and bounded text state");
    for (Uint16 row = 0; row < 5; ++row)
    {
        const size_t length = SDL_strnlen(d.lines[row], sizeof(d.lines[row]));
        if (length > 24)
            return SDL_SetError("BO5 dialogue cache row has no bounded terminator");
        for (size_t column = 0; column < length; ++column)
        {
            const Uint8 ch = (Uint8)d.lines[row][column];
            if (ch < 32 || ch > 126)
                return SDL_SetError("BO5 dialogue cache contains an unsupported US glyph");
            for (Uint16 y = 0; ch != 32 && y < 8; ++y)
                for (Uint16 x = 0; x < 2; ++x)
                    if (!g->valid[(240U + ((ch & 240U) >> 1) + y) * 1024U + 896U + (ch & 15U) * 2U +
                                  x])
                        return SDL_SetError("BO5 dialogue font word is not resident");
        }
    }
    for (Uint16 row = 0; row < 5; ++row)
    {
        for (Uint16 y = 0; y < 12; ++y)
            for (Uint16 x = 0; x < 64; ++x)
            {
                const size_t destination = (384U + row * 12U + y) * 1024U + x;
                g->words[destination] = 0;
                g->valid[destination] = 1;
            }
        for (Uint16 column = 0; column < 24 && d.lines[row][column]; ++column)
        {
            const Uint8 ch = (Uint8)d.lines[row][column];
            if (ch == 32)
                continue;
            for (Uint16 y = 0; y < 8; ++y)
                for (Uint16 x = 0; x < 2; ++x)
                    g->words[(384U + row * 12U + y) * 1024U + 2U + column * 2U + x] =
                        g->words[(240U + ((ch & 240U) >> 1) + y) * 1024U + 896U + (ch & 15U) * 2U +
                                 x];
        }
    }
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
    const Uint32 address = equipment.family == 43   ? 0x8017b044U
                           : equipment.family == 37 ? 0x8017a7b8U
                           : equipment.family == 52 ? 0x8017ac1cU
                           : equipment.family == 0  ? 0x8017b0bcU
                           : equipment.family == 31 ? 0x8017a90cU
                                                    : 0x8017abd4U;
    const Uint32 color_count = (equipment.family == 37 ? 5U : 7U) * 16;
    if (!Resolve(overlay, 0x8017a000, address, 4, 4, &pointer) ||
        !Resolve(overlay, 0x8017a000, Read32(pointer.bytes), color_count * 2, 2, &colors))
        return false;
    const size_t pixels = (size_t)equipment.family * 0x7000;
    if (pixels > weapons.size || 0x4000 > weapons.size - pixels)
        return SDL_SetError("Weapon texture is truncated");
    Upload(graphics, 576, 256 + hand * 128, 64, 128, weapons.bytes + pixels);
    for (Uint32 i = 0; i < color_count; ++i)
    {
        const Uint32 index = 0x1100 + hand * 0x180 + i;
        Upload(graphics, index & 255, 240 + (index - 0x1000) / 256, 1, 1, colors.bytes + i * 2);
    }
    graphics->weapon[hand] = overlay;
    return true;
}

static bool Frame(const SotnActorGraphics *graphics, SotnActorSprite sprite, SotnDataView *out)
{
    sprite.frame = (Uint16)(sprite.frame & 0x7fffU);
    const bool stage = (sprite.animation_set & 0x8000) != 0;
    const Uint32 bank = sprite.animation_set & 0x7fff;
    const bool weapon = !stage && (bank == 16 || bank == 18);
    const SotnDataView image = weapon  ? graphics->weapon[bank == 18 ? 1 : 0]
                               : stage ? graphics->nz0
                                       : graphics->dra;
    const Uint32 base = weapon ? 0x8017a000U : stage ? 0x80180000U : 0x800a0000U;
    SotnDataView table = {0}, first = {0};
    if (bank == 0 || (!weapon && bank >= (stage ? graphics->sprite_banks : 18U)) ||
        (!stage && (bank == 1 || bank == 13)))
        return SDL_SetError("Actor sprite bank is outside the supported multipart path");
    const Uint32 table_address = weapon  ? base + 0x40
                                 : stage ? Read32(graphics->nz0.bytes + 20)
                                         : base + 0x3b70;
    if (!weapon &&
        !Resolve(image, base, table_address, (stage ? graphics->sprite_banks : 18U) * 4, 4, &table))
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
    if (parts & 0x8000)
        return Resolve(image, base, Read32(table.bytes + (size_t)sprite.frame * 4), 6, 2, out);
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
        return SDL_SetError("Actor sprite references unloaded texture/palette word (%u,%u)",
                            x & 1023U, y & 511U);
    *out = graphics->words[index];
    return true;
}

static size_t PaletteIndex(Uint16 palette, Uint16 color)
{
    const Uint32 bank = palette / 256U;
    const Uint32 x = (bank == 0 ? 512U : (bank - 1) * 256U) + (palette & 15U) * 16U + color;
    return (size_t)(240U + (palette % 256U) / 16U) * 1024U + x;
}

// st/e_stained_glass_blend.h: native GPU memory is owned, private source files are immutable.
bool SotnActorGraphics_BlendPalette(SotnActorGraphics *g, SotnPaletteBlend blend)
{
    if (g == NULL)
        return SDL_InvalidParamError("graphics");
    if (!blend.active)
        return true;
    if (blend.source >= 768 || blend.destination >= 768 || blend.steps == 0 || blend.steps > 16 ||
        blend.steps > 768U - blend.destination ||
        (blend.source >= blend.destination && blend.source < blend.destination + blend.steps))
        return SDL_SetError("Stage palette blend requires bounded, nonoverlapping palette rows");
    Uint16 source[16] = {0}, colors[16][16] = {0};
    for (Uint16 i = 0; i < 16; ++i)
    {
        const size_t index = PaletteIndex(blend.source, i);
        if (!g->valid[index])
            return SDL_SetError("Stage palette blend source %u color %u is not loaded",
                                blend.source, i);
        source[i] = g->words[index];
    }
    const Uint32 target[3] = {blend.red / 8U, blend.green / 8U, blend.blue / 8U};
    for (Uint16 step = 0; step < blend.steps; ++step)
    {
        const Uint32 factor = step * 4096U / blend.steps;
        for (Uint16 i = 0; i < 16; ++i)
        {
            Uint16 color = source[i];
            if (color & 0x7fffU)
            {
                color &= 0x8000U;
                for (Uint16 channel = 0; channel < 3; ++channel)
                {
                    const Uint32 component = (source[i] >> (channel * 5U)) & 31U;
                    const Uint32 value =
                        (component * (4096U - factor) + target[channel] * factor) >> 12;
                    color |= (Uint16)(value << (channel * 5U));
                }
            }
            colors[step][i] = color;
        }
    }
    for (Uint16 step = 0; step < blend.steps; ++step)
        for (Uint16 i = 0; i < 16; ++i)
        {
            const size_t index = PaletteIndex((Uint16)(blend.destination + step), i);
            g->words[index] = colors[step][i];
            g->valid[index] = 1;
        }
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

static bool QuadEdgeInclusive(SotnTextureQuad q, Uint32 a, Uint32 b, Sint32 sign)
{
    const Sint32 dx = (q.x[b] - q.x[a]) * sign;
    const Sint32 dy = (q.y[b] - q.y[a]) * sign;
    return dy < 0 || (dy == 0 && dx > 0);
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
                             .tint = transform->tint,
                             .red = transform->red,
                             .green = transform->green,
                             .blue = transform->blue,
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

// dra/4B758.c sprite-sheet branch. BO4 owns separate body and bat sheet tables.
static Sint64 FloorDivide(Sint64 value, Sint64 divisor)
{
    // Callers bound numerators away from INT64_MIN and provide a nonzero scale/divisor.
    if (divisor < 0)
    {
        value = -value;
        divisor = -divisor;
    }
    return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}
static bool PackedImage(const SotnActorGraphics *g, SotnActorSprite sprite, SotnDataView definition,
                        SotnDataView *out)
{
    const Uint16 bank = sprite.animation_set & 0x7fff;
    if (g->stage_id != 26 || g->nz0.size < 52 || !(sprite.animation_set & 0x8000) ||
        (bank != 1 && bank != 2) || (sprite.blend_mode & ~0x70U))
        return SDL_SetError("Packed actor sheets require an integrated BO4 body/bat bank");
    const Uint32 base = 0x80180000U;
    SotnDataView table = {0}, sheet = {0};
    const Uint32 address = Read32(g->nz0.bytes + (bank == 1 ? 44 : 48));
    if (!Resolve(g->nz0, base, address, 4, 4, &table))
        return false;
    const Uint32 first = Read32(table.bytes);
    const Uint32 index = Read16(definition.bytes) & 0x7fff;
    if (first <= address || (first - address) % 4 || (first - address) / 4 > 1024 ||
        index >= (first - address) / 4 ||
        !Resolve(g->nz0, base, address, first - address, 4, &table) ||
        !Resolve(g->nz0, base, Read32(table.bytes + index * 4), 4, 4, &sheet))
        return SDL_SetError("Packed actor sheet index or table extent is invalid");
    const Uint32 width = sheet.bytes[0], height = sheet.bytes[1];
    if (!width || width % 4 || !height ||
        !Resolve(g->nz0, base, Read32(table.bytes + index * 4), 4 + width * height / 2, 4, &sheet))
        return SDL_SetError("Packed actor sheet dimensions or pixel extent is invalid");
    *out = sheet;
    return true;
}

static bool PackedSheet(const SotnActorGraphics *g, SotnActorSprite sprite, SotnDataView definition,
                        Sint32 x, Sint32 y, Uint16 *frame, const SotnSpriteTransform *transform)
{
    SotnDataView sheet = {0};
    if (!PackedImage(g, sprite, definition, &sheet))
        return false;
    const Uint32 width = sheet.bytes[0], height = sheet.bytes[1];
    const Uint16 pose = sprite.frame & 0x7fffU;
    const bool bat = (sprite.animation_set & 0x7fffU) == 2 && pose < 18 && g->bat_pivot_set[pose];
    const Sint32 pivot_x =
        (bat ? g->bat_pivot[pose] : Signed16(definition.bytes + 2)) + sheet.bytes[2];
    const Sint32 pivot_y = Signed16(definition.bytes + 4) + sheet.bytes[3];
    const Uint32 palette = sprite.palette & 0x7fff;
    if (palette >= 768)
        return SDL_SetError("Packed actor sheet palette exceeds native palette memory");
    Uint16 colors[16] = {0};
    const Uint32 palette_bank = palette / 256;
    const Uint32 palette_x =
        (palette_bank == 0 ? 512U : (palette_bank - 1) * 256U) + (palette & 15) * 16;
    for (Uint32 i = 0; i < 16; ++i)
        if (!Word(g, palette_x + i, 240 + (palette % 256) / 16, &colors[i]))
            return false;
    if ((sprite.frame & 0x8000U) && g->dopple_cached)
    {
        const Uint32 cache_x = Read16(g->dra.bytes + 0x21d8);
        const Uint32 cache_y = Read16(g->dra.bytes + 0x21da) & 511U;
        if (cache_x + width / 4U > 1024 || cache_y + height > 512)
            return SDL_SetError("BO4 retained pose dimensions exceed its display cache");
        for (Uint32 row = 0; row < height; ++row)
            for (Uint32 column = 0; column < width / 4U; ++column)
                if (!g->valid[(size_t)(cache_y + row) * 1024 + cache_x + column])
                    return SDL_SetError("BO4 retained pose requires valid cached pixels");
    }
    if (x < -65536 || x > 65536 || y < -65536 || y > 65536)
        return true;
    const SotnSpriteTransform t =
        transform != NULL ? *transform
                          : (SotnSpriteTransform){.scale_x = 256, .scale_y = 256, .opacity = 128};
    const double radians = t.rotation * (SDL_PI_D / 2048.0);
    const Sint32 cosine = (Sint32)SDL_lround(SDL_cos(radians) * 4096.0);
    const Sint32 sine = (Sint32)SDL_lround(SDL_sin(radians) * 4096.0);
    const Sint64 denominator_x = 8192LL * t.scale_x, denominator_y = 8192LL * t.scale_y;
    // Inverse sampling at pixel centers retains identity pivots and the original mirror +1.
    // Retained BO4 poses sample the owned display cache, including simulation-driven erasure.
    for (Sint32 py = 0; py < SOTN_FRAME_HEIGHT; ++py)
        for (Sint32 px = 0; px < SOTN_FRAME_WIDTH; ++px)
        {
            const Sint64 dx = ((Sint64)px - x) * 2 + 1, dy = ((Sint64)py - y) * 2 + 1;
            const Sint64 nx = (dx * cosine + dy * sine) * 256;
            const Sint64 ny = (-dx * sine + dy * cosine) * 256;
            const Sint64 sx = sprite.facing_left ? FloorDivide(-nx, denominator_x) + 1 - pivot_x
                                                 : FloorDivide(nx, denominator_x) - pivot_x;
            const Sint64 sy = FloorDivide(ny, denominator_y) - pivot_y;
            if (sx < 0 || sy < 0 || sx >= width || sy >= height)
                continue;
            const size_t pixel = (size_t)sy * width + (size_t)sx;
            Uint8 color_index = (sheet.bytes[4 + pixel / 2] >> ((pixel & 1U) * 4)) & 15;
            if ((sprite.frame & 0x8000U) && g->dopple_cached)
            {
                const Uint32 cache_x = Read16(g->dra.bytes + 0x21d8);
                const Uint32 cache_y = Read16(g->dra.bytes + 0x21da) & 511U;
                const size_t word = (cache_y + (size_t)sy) * 1024 + cache_x + (size_t)sx / 4;
                color_index = (Uint8)((g->words[word] >> (((size_t)sx & 3U) * 4U)) & 15U);
            }
            Uint16 color = colors[color_index];
            if (!color)
                continue;
            if (t.opacity != 128 || t.tint)
            {
                Uint16 tinted = color & 0x8000;
                for (Uint32 shift = 0; shift < 15; shift += 5)
                    tinted |= (Uint16)(SDL_min(31, ((color >> shift) & 31) *
                                                       (t.tint ? (shift == 0   ? t.red
                                                                  : shift == 5 ? t.green
                                                                               : t.blue)
                                                               : t.opacity) /
                                                       128)
                                       << shift);
                color = tinted;
            }
            const size_t destination = (size_t)py * SOTN_FRAME_WIDTH + (size_t)px;
            frame[destination] =
                sprite.blend_mode && (color & 0x8000)
                    ? Blend(frame[destination], color, (sprite.blend_mode >> 5) & 3)
                    : color;
        }
    return true;
}

static bool Draw(const SotnActorGraphics *graphics, SotnActorSprite sprite, Sint32 x, Sint32 y,
                 Uint16 *frame, size_t count, const SotnSpriteTransform *transform)
{
    if (graphics == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Actor drawing requires initialized graphics and a full framebuffer");
    if ((sprite.frame & 0x7fff) == 0 || sprite.animation_set == 0)
        return true;
    SotnDataView definition = {0};
    if (!Frame(graphics, sprite, &definition))
        return false;
    const Uint32 parts = Read16(definition.bytes);
    if (parts & 0x8000)
        return PackedSheet(graphics, sprite, definition, x, y, frame, transform);
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
    if ((!q.untextured && graphics == NULL) || frame == NULL || count < SOTN_FRAME_PIXELS ||
        q.page > 0x1ff || (!q.untextured && (((q.page >> 7) & 3) == 3 || q.palette >= 768)))
        return SDL_SetError("Projectile quad requires bounded texture attributes and a full frame");
    Sint32 left = 2048, right = -2048, top = 2048, bottom = -2048;
    for (Uint32 i = 0; i < 4; ++i)
    {
        if (!q.untextured && (q.u[i] > 256 || q.v[i] > 256))
            return SDL_SetError("Projectile quad UV exceeds its texture page");
        if (q.x[i] < -2048 || q.x[i] > 2048 || q.y[i] < -2048 || q.y[i] > 2048)
            return SDL_SetError("Projectile quad exceeds the bounded screen margin");
        left = SDL_min(left, q.x[i]);
        right = SDL_max(right, q.x[i]);
        top = SDL_min(top, q.y[i]);
        bottom = SDL_max(bottom, q.y[i]);
    }
    for (Uint32 pass = 0; pass < 2; ++pass)
        for (Sint32 y = SDL_max(0, top); y < SDL_min(SOTN_FRAME_HEIGHT, bottom); ++y)
            for (Sint32 x = SDL_max(0, left); x < SDL_min(SOTN_FRAME_WIDTH, right); ++x)
            {
                Sint64 u = 0, v = 0;
                Uint32 shade[3] = {q.red, q.green, q.blue};
                bool inside = false;
                // PS1 quad order is the triangles (0,1,2) and (1,3,2).
                for (Uint32 triangle = 0; triangle < 2 && !inside; ++triangle)
                {
                    const Uint32 ia = triangle ? 1 : 0, ib = triangle ? 3 : 1, ic = 2;
                    const Sint64 ax = q.x[ib] - q.x[ia], ay = q.y[ib] - q.y[ia];
                    const Sint64 bx = q.x[ic] - q.x[ia], by = q.y[ic] - q.y[ia];
                    const Sint64 px = (Sint64)x * 2 + 1 - (Sint64)q.x[ia] * 2;
                    const Sint64 py = (Sint64)y * 2 + 1 - (Sint64)q.y[ia] * 2;
                    Sint64 denominator = 2 * (ax * by - ay * bx);
                    Sint64 wb = px * by - py * bx, wc = ax * py - ay * px;
                    const Sint32 sign = denominator < 0 ? -1 : 1;
                    if (denominator < 0)
                    {
                        denominator = -denominator;
                        wb = -wb;
                        wc = -wc;
                    }
                    if (denominator == 0 || wb < 0 || wc < 0 || wb + wc > denominator)
                        continue;
                    const Sint64 wa = denominator - wb - wc;
                    if ((wa == 0 && !QuadEdgeInclusive(q, ib, ic, sign)) ||
                        (wb == 0 && !QuadEdgeInclusive(q, ic, ia, sign)) ||
                        (wc == 0 && !QuadEdgeInclusive(q, ia, ib, sign)))
                        continue;
                    u = (wa * q.u[ia] + wb * q.u[ib] + wc * q.u[ic]) / denominator;
                    v = (wa * q.v[ia] + wb * q.v[ib] + wc * q.v[ic]) / denominator;
                    if (q.gouraud)
                    {
                        const Uint8 *channels[3] = {q.vertex_red, q.vertex_green, q.vertex_blue};
                        for (size_t channel = 0; channel < 3; ++channel)
                            shade[channel] =
                                (Uint32)((wa * channels[channel][ia] + wb * channels[channel][ib] +
                                          wc * channels[channel][ic]) /
                                         denominator);
                    }
                    inside = true;
                }
                if (!inside)
                    continue;
                Uint16 color = 0;
                if (q.untextured)
                    color = (Uint16)((shade[0] / 8) | (shade[1] / 8) << 5 | (shade[2] / 8) << 10);
                else if (u < 0 || u >= 256 || v < 0 || v >= 256 ||
                         !Texel(graphics, q.page, q.palette, (Uint8)u, (Uint8)v, &color))
                    return SDL_SetError("Projectile texture reference is invalid: %s",
                                        SDL_GetError());
                if (write && pass != 0 && (color != 0 || q.untextured))
                {
                    if (!q.untextured && (q.modulate || q.tint || q.gouraud))
                    {
                        Uint16 tinted = color & 0x8000;
                        for (Uint32 shift = 0; shift < 15; shift += 5)
                            tinted |= (Uint16)(SDL_min(31, ((color >> shift) & 31) *
                                                               (q.gouraud ? shade[shift / 5]
                                                                : q.tint  ? (shift == 0   ? q.red
                                                                             : shift == 5 ? q.green
                                                                                          : q.blue)
                                                                          : q.opacity) /
                                                               128)
                                               << shift);
                        color = tinted;
                    }
                    const size_t pixel = (size_t)y * SOTN_FRAME_WIDTH + (size_t)x;
                    frame[pixel] = q.blend && ((color & 0x8000) || q.untextured)
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
    const SotnSpriteTransform transform = {
        .rotation = angle, .scale_x = 256, .scale_y = 256, .opacity = 128};
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
    if (transform.scale_x == 0 || transform.scale_y == 0 || transform.scale_x < -1024 ||
        transform.scale_x > 1024 || transform.scale_y < -1024 || transform.scale_y > 1024 ||
        transform.opacity > 255)
        return SDL_SetError("Actor transform exceeds the bounded original scales or color");
    if (transform.rotation == 0 && transform.scale_x == 256 && transform.scale_y == 256 &&
        transform.opacity == 128 && !transform.tint)
        return Draw(g, sprite, x, y, frame, count, NULL);
    return Draw(g, sprite, x, y, frame, count, &transform);
}

bool SotnActorGraphics_Line(SotnColorLine line, Uint16 *frame, size_t count)
{
    if (frame == NULL || count < SOTN_FRAME_PIXELS || line.blend_mode > 3)
        return SDL_SetError("Actor line requires a full frame and an original blend mode");
    for (size_t i = 0; i < 2; ++i)
        if (line.x[i] < -2048 || line.x[i] > 2048 || line.y[i] < -2048 || line.y[i] > 2048)
            return SDL_SetError("Actor line endpoint exceeds bounded native presentation");
    const Sint32 dx = line.x[1] - line.x[0], dy = line.y[1] - line.y[0];
    const Sint32 steps = SDL_max(SDL_abs(dx), SDL_abs(dy));
    const Sint32 divisor = steps ? steps : 1;
    for (Sint32 i = 0; i <= steps; ++i)
    {
        const Sint32 x = (Sint32)FloorDivide((Sint64)line.x[0] * divisor + (Sint64)dx * i, divisor);
        const Sint32 y = (Sint32)FloorDivide((Sint64)line.y[0] * divisor + (Sint64)dy * i, divisor);
        if (x < 0 || x >= SOTN_FRAME_WIDTH || y < 0 || y >= SOTN_FRAME_HEIGHT)
            continue;
        const Uint16 red =
            (Uint16)(((Sint64)line.red[0] * divisor + ((Sint32)line.red[1] - line.red[0]) * i) /
                     divisor / 8);
        const Uint16 green = (Uint16)(((Sint64)line.green[0] * divisor +
                                       ((Sint32)line.green[1] - line.green[0]) * i) /
                                      divisor / 8);
        const Uint16 blue =
            (Uint16)(((Sint64)line.blue[0] * divisor + ((Sint32)line.blue[1] - line.blue[0]) * i) /
                     divisor / 8);
        const Uint16 front = (Uint16)(red | green << 5 | blue << 10);
        const size_t pixel = (size_t)y * SOTN_FRAME_WIDTH + (size_t)x;
        frame[pixel] = line.blend ? Blend(frame[pixel], front, line.blend_mode) : front;
    }
    return true;
}

bool SotnActorGraphics_LoadDopplePose(SotnActorGraphics *g, SotnActorSprite sprite)
{
    if (g == NULL || g->stage_id != 26 || g->dra.size < 0x21dc)
        return SDL_SetError("BO4 sprite cache requires its private display-slot metadata");
    if (!(sprite.frame & 0x7fffU))
        return true;
    SotnDataView definition = {0}, sheet = {0};
    if (!Frame(g, sprite, &definition) || !(Read16(definition.bytes) & 0x8000U) ||
        !PackedImage(g, sprite, definition, &sheet))
        return SDL_SetError("BO4 display cache requires a backed packed body/bat pose");
    const Uint32 x = Read16(g->dra.bytes + 0x21d8);
    const Uint32 y = Read16(g->dra.bytes + 0x21da) & 511U;
    const Uint32 width = sheet.bytes[0] / 4U, height = sheet.bytes[1];
    if (x == 0 || y == 0 || x + width >= 1024 || y + height + 4 > 512)
        return SDL_SetError("BO4 sprite display slot and margins exceed native VRAM");
    if (sprite.frame & 0x8000U)
        return g->dopple_cached || SDL_SetError("BO4 retained pose has no prior cached pixels");
    for (Uint32 row = 0; row < height; ++row)
    {
        // This border belongs to the display slot, not an external texture asset.
        g->valid[(size_t)(y + row) * 1024 + x - 1] = 1;
        for (Uint32 column = 0; column < width; ++column)
        {
            const size_t pixel = (size_t)(y + row) * 1024 + x + column;
            g->words[pixel] = Read16(sheet.bytes + 4 + ((size_t)row * width + column) * 2);
            g->valid[pixel] = 1;
        }
    }
    for (Uint32 row = 0; row < height + 4; ++row)
    {
        const size_t pixel = (size_t)(y - 1 + row) * 1024 + x + width;
        g->words[pixel] = 0;
        g->valid[pixel] = 1;
    }
    for (Uint32 row = 0; row < 4; ++row)
        for (Uint32 column = 0; column < width + 2; ++column)
        {
            const size_t pixel = (size_t)(y + height + row) * 1024 + x - 1 + column;
            g->words[pixel] = 0;
            g->valid[pixel] = 1;
        }
    g->dopple_cached = true;
    g->dopple_death_cached = false;
    return true;
}

bool SotnActorGraphics_UpdateDoppleBat(SotnActorGraphics *g, Uint16 frame, Sint16 offset)
{
    const Uint16 pose = frame & 0x7fffU;
    if (g == NULL || g->stage_id != 26 || pose == 0 || pose >= 18 || offset < -64 || offset > 64 ||
        g->nz0.size < 0x11e8 + 36)
        return SDL_SetError("BO4 bat pivot requires a bounded pose and original offset table");
    SotnDataView definition = {0}, sheet = {0};
    const SotnActorSprite sprite = {.animation_set = 0x8002, .frame = frame};
    if (!Frame(g, sprite, &definition) || !PackedImage(g, sprite, definition, &sheet))
        return false;
    const Sint32 pivot = Signed16(g->nz0.bytes + 0x11e8 + pose * 2U) + offset;
    if (pivot < -1024 || pivot > 1024)
        return SDL_SetError("BO4 runtime bat pivot exceeds its supported geometry");
    g->bat_pivot[pose] = (Sint16)pivot;
    g->bat_pivot_set[pose] = true;
    return true;
}

bool SotnActorGraphics_DoppleDissolve(SotnActorGraphics *g, SotnActorSprite sprite,
                                      SotnDataView erased)
{
    if (g == NULL || g->stage_id != 26 || erased.bytes == NULL || erased.size != 512 ||
        g->nz0.size < 0x1fe0)
        return SDL_SetError(
            "BO4 dissolve requires its original display cache and full erasure mask");
    const Uint32 x = Read16(g->nz0.bytes + 0x1fd8), y = Read16(g->nz0.bytes + 0x1fda);
    const Uint32 width = Read16(g->nz0.bytes + 0x1fdc), height = Read16(g->nz0.bytes + 0x1fde);
    if (width != 16 || height != 64 || x + width > 1024 || y + height > 512)
        return SDL_SetError("BO4 dissolve rectangle exceeds its original 64x64 scratch image");
    if (g->dopple_death_cached && (g->dopple_death_frame != (sprite.frame & 0x7fffU) ||
                                   g->dopple_death_bank != sprite.animation_set))
        return SDL_SetError("BO4 dissolve changed its captured final pose");
    if (!g->dopple_death_cached)
    {
        // A presentation frame may have been skipped when the final pose was uploaded.
        // Resolve that immutable pose before capturing, independently of render cadence.
        SotnActorSprite final_pose = sprite;
        final_pose.frame &= 0x7fffU;
        if (!(final_pose.frame) || !SotnActorGraphics_LoadDopplePose(g, final_pose))
            return SDL_SetError("BO4 dissolve requires a backed final body pose");
        g->dopple_death_frame = final_pose.frame;
        g->dopple_death_bank = final_pose.animation_set;
        for (Uint32 row = 0; row < height; ++row)
            for (Uint32 column = 0; column < width; ++column)
                // Native VRAM starts zeroed, including unused padding around a packed pose.
                g->dopple_death_words[row * width + column] =
                    g->words[(size_t)(y + row) * 1024 + x + column];
    }
    for (Uint32 row = 0; row < height; ++row)
        for (Uint32 column = 0; column < width; ++column)
        {
            Uint16 word = g->dopple_death_words[row * width + column];
            for (Uint32 nibble = 0; nibble < 4; ++nibble)
            {
                const Uint32 bit = (row * width + column) * 4U + nibble;
                if (erased.bytes[bit / 8] & (1U << (bit & 7U)))
                    word &= (Uint16) ~(15U << (nibble * 4U));
            }
            const size_t target = (size_t)(y + row) * 1024 + x + column;
            g->words[target] = word;
            g->valid[target] = 1;
        }
    g->dopple_death_cached = true;
    return true;
}
