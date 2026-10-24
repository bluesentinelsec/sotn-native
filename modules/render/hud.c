// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected DrawHud/DrawHudSubweapon and palette paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/5F60C.c and 4A538.c.
// Sprite positions, pixels, palettes, and animation tables remain private runtime data.
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/hud.h>

typedef struct HudSprite
{
    Sint32 x, y;
    Uint16 u, v, width, height, palette;
    bool hidden, rotate, blend;
} HudSprite;

struct SotnHud
{
    SotnDataView dra, common, icons[11], hp_colors[3], mp_colors[5], frame_colors[2];
    HudSprite sprites[14];
    SotnHudStatus status;
    Uint16 display_hp, glint_timer, heart_cost[10];
    Uint8 glint_phase, glint_repeats;
    Uint32 ticks, random, palette_period;
};

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}

static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}

static bool Resolve(SotnDataView dra, Uint32 address, size_t size, SotnDataView *out)
{
    if (dra.bytes == NULL || address < 0x800a0000 || (address & 1) ||
        address - 0x800a0000 > dra.size || size > dra.size - (address - 0x800a0000))
        return SDL_SetError("HUD references an invalid or truncated private DRA address");
    *out = (SotnDataView){dra.bytes + (address - 0x800a0000), size};
    return true;
}

static bool StatusValid(SotnHudStatus status)
{
    return status.hp_max != 0 && status.hp_max <= 9999 && status.hp <= status.hp_max &&
           status.mp_max != 0 && status.mp_max <= 9999 && status.mp <= status.mp_max &&
           status.hearts <= 9999 && status.subweapon < 10;
}

static bool Palettes(SotnHud *hud)
{
    SotnDataView table = {0}, record = {0};
    if (!Resolve(hud->dra, 0x800a3bb8, 6 * 4, &table))
        return false;
    for (Uint32 i = 0; i < 3; ++i)
    {
        if (!Resolve(hud->dra, Read32(table.bytes + (i + 1) * 4), 16, &record) ||
            Read32(record.bytes) != 1 || Read32(record.bytes + 4) != 0x171e ||
            Read32(record.bytes + 8) != 2 ||
            !Resolve(hud->dra, Read32(record.bytes + 12), 4, &hud->hp_colors[i]))
            return SDL_SetError("HUD requires the original bounded HP palette-copy definitions");
    }
    if (!Resolve(hud->dra, Read32(table.bytes + 5 * 4), 36, &record) ||
        (Read32(record.bytes) & 65535) != 3 || Read32(record.bytes + 4) != 0x1600 ||
        Read32(record.bytes + 8) != 256 || Read32(record.bytes + 32) != SDL_MAX_UINT32)
        return SDL_SetError("HUD requires the original five-frame MP palette cycle");
    hud->palette_period = Read32(record.bytes) >> 16;
    if (hud->palette_period == 0 || hud->palette_period > 255)
        return SDL_SetError("HUD MP palette period is invalid");
    for (Uint32 i = 0; i < 5; ++i)
    {
        SotnDataView colors = {0};
        if (!Resolve(hud->dra, Read32(record.bytes + 12 + i * 4), 512, &colors))
            return false;
        hud->mp_colors[i] = (SotnDataView){colors.bytes + 2 * 16 * 2, 32};
    }
    if (!Resolve(hud->dra, Read32(table.bytes + 4 * 4), 24, &record) ||
        Read32(record.bytes) != 0x40003 || Read32(record.bytes + 4) != 0x1700 ||
        Read32(record.bytes + 8) != 16 || Read32(record.bytes + 20) != SDL_MAX_UINT32)
        return SDL_SetError("HUD requires the original two-frame subweapon-frame palette cycle");
    for (Uint32 i = 0; i < 2; ++i)
        if (!Resolve(hud->dra, Read32(record.bytes + 12 + i * 4), 32, &hud->frame_colors[i]))
            return false;
    return true;
}

bool SotnHud_Create(SotnDataView dra, SotnDataView common, SotnHudStatus status, SotnHud **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnDataView layout = {0}, icon_table = {0};
    if (!StatusValid(status) || common.bytes == NULL || common.size != 0x42000 ||
        !Resolve(dra, 0x800a2ee8, 0xd4, &layout) || !Resolve(dra, 0x800c52f8, 11 * 4, &icon_table))
        return SDL_SetError("HUD requires bounded original status, layout, and common artwork");
    SotnHud *hud = SDL_calloc(1, sizeof(*hud));
    if (hud == NULL)
        return false;
    hud->dra = dra;
    hud->common = common;
    hud->status = status;
    hud->display_hp = status.hp;
    for (Uint32 i = 0; i < 14; ++i)
    {
        const Uint8 *p = layout.bytes;
        hud->sprites[i] = (HudSprite){.x = p[i],
                                      .y = p[16 + i],
                                      .u = p[32 + i],
                                      .v = p[48 + i],
                                      .width = p[64 + i],
                                      .height = p[80 + i],
                                      .palette = Read16(p + 96 + i * 2),
                                      .hidden = (Read16(p + 124 + i * 2) & 8) != 0,
                                      .rotate = i == 5};
        const HudSprite sprite = hud->sprites[i];
        if (sprite.width == 0 || sprite.width > 128 || sprite.height == 0 || sprite.height > 64 ||
            (i >= 6 && (sprite.width != 8 || sprite.height != (i < 10 ? 16 : 8))) ||
            sprite.u + sprite.width > 256 || sprite.v + sprite.height > 128 ||
            sprite.palette < 256 || sprite.palette >= 512)
        {
            SDL_SetError("HUD sprite layout exceeds the supported texture or palette bounds");
            goto failed;
        }
    }
    for (Uint32 i = 0; i < 14; ++i)
    {
        const Uint32 palette = Read16(layout.bytes + 152 + i * 2);
        if (palette < 256 || palette >= 512 ||
            (Uint32)layout.bytes[180 + i] + hud->sprites[1].width > 256 ||
            (Uint32)layout.bytes[196 + i] + hud->sprites[1].height > 128)
        {
            SDL_SetError("HUD glint exceeds the supported texture or palette bounds");
            goto failed;
        }
    }
    SotnDataView subweapons = {0};
    if (!Resolve(dra, 0x800a4a00, 10 * 20, &subweapons))
        goto failed;
    for (Uint32 i = 0; i < 10; ++i)
    {
        const Uint16 cost = Read16(subweapons.bytes + i * 20 + 2);
        hud->heart_cost[i] = cost == 0 || cost > 32767 ? 1 : cost;
    }
    for (Uint32 i = 0; i < 11; ++i)
    {
        SotnDataView header = {0};
        const Uint32 address = Read32(icon_table.bytes + i * 4);
        if (!Resolve(dra, address, 4, &header) || header.bytes[0] != (i == 10 ? 8 : 16) ||
            header.bytes[1] != (i == 10 ? 56 : 16) ||
            !Resolve(dra, address, i == 10 ? 228 : 132, &hud->icons[i]))
        {
            SDL_SetError("HUD subweapon/MP artwork has invalid dimensions or truncated pixels");
            goto failed;
        }
    }
    if (!Palettes(hud))
        goto failed;
    hud->glint_timer = (Uint16)((SotnCombat_Random(&hud->random) & 255) + 8);
    hud->glint_repeats = (Uint8)((SotnCombat_Random(&hud->random) & 7) + 1);
    *out = hud;
    return true;
failed:
    SDL_free(hud);
    return false;
}

bool SotnHud_Tick(SotnHud *hud, SotnHudStatus status)
{
    if (hud == NULL || !StatusValid(status))
        return SDL_SetError("HUD tick requires bounded HP, MP, hearts, and subweapon status");
    const Sint32 difference = (Sint32)status.hp - hud->display_hp;
    const Sint32 step = SDL_abs(difference) > 10 ? difference / 10
                        : difference > 0         ? 1
                        : difference < 0         ? -1
                                                 : 0;
    hud->display_hp = (Uint16)((Sint32)hud->display_hp + step);
    hud->status = status;
    ++hud->ticks;
    if (status.subweapon != 0 && --hud->glint_timer == 0)
    {
        ++hud->glint_phase;
        if (hud->glint_phase == 15)
        {
            hud->glint_phase = 0;
            hud->glint_timer = (Uint16)((SotnCombat_Random(&hud->random) & 255) + 8);
            if (--hud->glint_repeats != 0)
            {
                hud->glint_phase = 1;
                hud->glint_timer = 1;
            }
            else
                hud->glint_repeats = (Uint8)((SotnCombat_Random(&hud->random) & 7) + 1);
        }
        else
            hud->glint_timer = 1;
    }
    return true;
}

static Uint16 Color(const SotnHud *hud, Uint16 palette, Uint8 index)
{
    if (palette == 0x171 && index >= 14)
    {
        const Uint32 state = hud->display_hp == hud->status.hp_max          ? 1U
                             : hud->display_hp <= (hud->status.hp_max >> 2) ? 2U
                                                                            : 0U;
        return Read16(hud->hp_colors[state].bytes + (index - 14) * 2U);
    }
    if (palette == 0x170)
        return Read16(hud->frame_colors[(hud->ticks / 4) % 2].bytes + index * 2U);
    if (palette == 0x162)
        return Read16(hud->mp_colors[(hud->ticks / hud->palette_period) % 5].bytes + index * 2U);
    return Read16(hud->common.bytes + 0x40000 + ((palette - 256U) * 16 + index) * 2);
}

static Uint8 Pixel(const SotnHud *hud, Uint32 u, Uint32 v)
{
    if (u >= 112 && u < 120 && v < 56)
    {
        if (v >= (Uint32)hud->status.mp * 50 / hud->status.mp_max)
            return 0;
        const size_t pixel = v * 8 + u - 112;
        return (hud->icons[10].bytes[4 + pixel / 2] >> ((pixel & 1) * 4)) & 15;
    }
    if (u < 16 && v >= 32 && v < 48)
    {
        const size_t pixel = (v - 32) * 16 + u;
        return (hud->icons[hud->status.subweapon].bytes[4 + pixel / 2] >> ((pixel & 1) * 4)) & 15;
    }
    const Uint32 word = u / 4;
    const size_t offset = (28U + word / 32) * 8192 + v * 64 + (word % 32) * 2;
    return (Uint8)((Read16(hud->common.bytes + offset) >> ((u & 3) * 4)) & 15);
}

static void DrawSprite(const SotnHud *hud, HudSprite sprite, Uint16 *frame)
{
    if (sprite.hidden)
        return;
    const Uint32 width = sprite.rotate ? sprite.height : sprite.width;
    const Uint32 height = sprite.rotate ? sprite.width : sprite.height;
    for (Uint32 y = 0; y < height; ++y)
        for (Uint32 x = 0; x < width; ++x)
        {
            const Sint32 px = sprite.x + (Sint32)x, py = sprite.y + (Sint32)y;
            if (px < 0 || py < 0 || px >= SOTN_FRAME_WIDTH || py >= SOTN_FRAME_HEIGHT)
                continue;
            const Uint32 u = sprite.u + (sprite.rotate ? sprite.width - y - 1 : x);
            const Uint32 v = sprite.v + (sprite.rotate ? x : y);
            Uint16 color = Color(hud, sprite.palette, Pixel(hud, u, v));
            if (color == 0)
                continue;
            const size_t pixel = (size_t)py * SOTN_FRAME_WIDTH + (size_t)px;
            if (sprite.blend && (color & 0x8000))
            {
                Uint16 blended = 0;
                for (Uint32 shift = 0; shift < 15; shift += 5)
                    blended |= (Uint16)(SDL_min(31, ((frame[pixel] >> shift) & 31) +
                                                        ((color >> shift) & 31) / 4)
                                        << shift);
                color = blended;
            }
            frame[pixel] = color;
        }
}

static void Digits(HudSprite *sprites, Uint16 value, bool hearts)
{
    const Uint32 leading = value > 999 ? 0 : value > 99 ? 1 : value > 9 ? 2 : 3;
    const Sint32 spacing = hearts ? 4 : value > 99 ? 6 : 7;
    const Sint32 left = hearts        ? (value > 999  ? 59
                                         : value > 99 ? 55
                                                      : 51)
                        : value > 999 ? 3
                        : value > 99  ? 0
                                      : -6;
    Uint32 divisor = 1000;
    for (Uint32 i = 0; i < 4; ++i, divisor /= 10)
    {
        sprites[i].hidden = i < leading;
        sprites[i].x = left + (Sint32)i * spacing;
        sprites[i].u = (Uint16)(32 + ((value / divisor) % 10) * 8);
    }
}

bool SotnHud_Draw(const SotnHud *hud, Uint16 *frame, size_t count)
{
    if (hud == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("HUD drawing requires initialized presentation and a full framebuffer");
    HudSprite sprites[14] = {0};
    SDL_memcpy(sprites, hud->sprites, sizeof(sprites));
    sprites[0].hidden = sprites[1].hidden = sprites[2].hidden = hud->status.subweapon == 0;
    sprites[0].blend = true;
    if (hud->glint_phase != 0)
    {
        const Uint32 phase = hud->glint_phase - 1U;
        sprites[1].palette = Read16(hud->dra.bytes + 0x2f80 + phase * 2);
        sprites[1].u = hud->dra.bytes[0x2f9c + phase];
        sprites[1].v = hud->dra.bytes[0x2fac + phase];
    }
    else
        sprites[1].hidden = true;
    sprites[5].palette = hud->status.mp == hud->status.mp_max ? 0x162 : 0x174;
    Digits(sprites + 6, hud->display_hp, false);
    Digits(sprites + 10, hud->status.hearts, true);
    if (hud->status.subweapon != 0 && hud->status.hearts >= hud->heart_cost[hud->status.subweapon])
        for (Uint32 i = 10; i < 14; ++i)
            sprites[i].palette = (hud->ticks & 2) ? 0x196 : 0x193;
    for (Uint32 i = 0; i < 14; ++i)
        DrawSprite(hud, sprites[i], frame);
    return true;
}

Uint16 SotnHud_DisplayedHp(const SotnHud *hud)
{
    return hud != NULL ? hud->display_hp : 0;
}

void SotnHud_Destroy(SotnHud *hud)
{
    SDL_free(hud);
}
