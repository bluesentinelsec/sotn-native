// SPDX-License-Identifier: AGPL-3.0-or-later
// servant/tt_002/faerie.c:UpdateSubEntityUseItem/UpdateSubEntityUseLifeApple,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profile bytes remain private.
#include <sotn/combat.h>
#include <sotn/fairy_effects.h>

typedef struct ItemProfile
{
    Uint16 count, batch, style, width, height, red, green, blue, draw_mode;
    Sint16 interval, priority;
    Uint32 flags;
} ItemProfile;
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return Read16(p) | (Uint32)Read16(p + 2) << 16;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-(Sint64)value + scale - 1) / scale));
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)Shift(value, 16);
}
static Sint32 Position(Sint32 value)
{
    return Fixed((Uint32)(Uint16)Word(value) << 16);
}
static Sint32 Raster(Sint16 high, Sint32 fixed)
{
    return Fixed(((Uint32)(Uint16)high << 16) | ((Uint32)fixed & 65535U));
}
static bool Profile(SotnDataView overlay, Uint16 parameter, ItemProfile *out)
{
    const size_t offset = 0x24cc + parameter * 28U;
    if (parameter >= 11 || !overlay.bytes || offset > overlay.size || overlay.size - offset < 28)
        return SDL_SetError("Fairy item effect requires its actual bounded private profile0..10");
    const Uint8 *p = overlay.bytes + offset;
    ItemProfile profile = {.count = Read16(p),
                           .batch = Read16(p + 2),
                           .interval = Word(Read16(p + 4)),
                           .style = Read16(p + 6),
                           .width = Read16(p + 8),
                           .height = Read16(p + 10),
                           .red = Read16(p + 12),
                           .green = Read16(p + 14),
                           .blue = Read16(p + 16),
                           .priority = Word(Read16(p + 18)),
                           .draw_mode = Read16(p + 20),
                           .flags = Read32(p + 24)};
    if (!profile.count || profile.count > SOTN_FAIRY_ITEM_PARTICLES || !profile.batch ||
        profile.batch > profile.count || profile.count % profile.batch || profile.style > 2 ||
        !profile.width || profile.width > 255 || !profile.height || profile.height > 255 ||
        profile.red > 255 || profile.green > 255 || profile.blue > 255)
        return SDL_SetError("Fairy item profile has invalid actual particle capacity or geometry");
    *out = profile;
    return true;
}
bool SotnFairyItemEffect_Rows(SotnDataView overlay, Uint16 parameter, Uint16 *rows)
{
    ItemProfile profile = {0};
    if (!rows || !Profile(overlay, parameter, &profile))
        return SDL_SetError("Fairy item rows require a valid original profile and output");
    *rows = (Uint16)(profile.count + 1U);
    return true;
}
static void InitTile(SotnPlayerParticle *tile, ItemProfile profile, SotnPlayerFactorySource source,
                     Uint32 *random)
{
    const Sint16 x = Pixel(source.x), y = Pixel(source.y);
    *tile = (SotnPlayerParticle){.x = Position(x),
                                 .y = Position(y),
                                 .draw_mode = (Uint16)(profile.draw_mode + 8U),
                                 .red = (Uint8)profile.red,
                                 .green = (Uint8)profile.green,
                                 .blue = (Uint8)profile.blue,
                                 .width = (Uint8)profile.width,
                                 .height = (Uint8)profile.height};
    if (profile.style == 0)
    {
        tile->x = Position(x + (source.facing_left ? -4 : 4));
        tile->y = Position(y - 26);
        tile->velocity_x = ((Sint32)(SotnCombat_Random(random) % 8192U) - 4096) * 16;
    }
    else if (profile.style == 1)
    {
        const Sint16 center = Word(x + (source.facing_left ? 24 : -24));
        tile->x = Position(center + (Sint32)(SotnCombat_Random(random) % 16U) - 8);
        tile->y = Position(y + 16);
        tile->velocity_x = ((Sint32)(SotnCombat_Random(random) % 8192U) - 4096) * 16;
        tile->velocity_y = -((Sint32)(SotnCombat_Random(random) % 4096U) + 6144) * 16;
        tile->timer = 45;
    }
    else
    {
        tile->x = Position(x + (source.facing_left ? -14 : 14));
        tile->y = Position(y - 8);
        tile->velocity_x = (Sint32)(SotnCombat_Random(random) % 2048U) * 16;
        if (source.facing_left)
            tile->velocity_x = -tile->velocity_x;
        tile->velocity_y = -((Sint32)(SotnCombat_Random(random) % 4096U) + 2048) * 16;
        tile->timer = 40;
    }
    tile->screen_x = Pixel(tile->x);
    tile->screen_y = Pixel(tile->y);
}
static Sint32 TowardZero(Sint32 value)
{
    if (value < 0)
        return SDL_min(value + 4096, 0);
    return SDL_max(value - 4096, 0);
}
bool SotnFairyItemEffect_Tick(SotnFairyItemEffect *effect, SotnFairyItemEffectInput in,
                              Uint32 *random)
{
    if (!effect || !random || effect->removed || effect->phase > 2 ||
        effect->count > SOTN_FAIRY_ITEM_PARTICLES)
        return SDL_SetError("Fairy item callback requires its actual bounded cursor and libc RNG");
    ItemProfile profile = {0};
    if (!Profile(in.overlay, effect->parameter, &profile))
        return false;
    SotnFairyItemEffect next = *effect;
    Uint32 rng = *random;
    if (!next.phase)
    {
        if (!in.allocated)
        {
            next.removed = true;
            *effect = next;
            return true;
        }
        next.x = in.source.x;
        next.y = in.source.y;
        next.flags = profile.flags;
        next.count = profile.count;
        next.priority = (Uint16)((Sint32)in.player_priority + profile.priority);
        for (Uint16 i = 0; i < next.count; ++i)
            InitTile(&next.tiles[i], profile, in.source, &rng);
        next.tiles[next.count] =
            (SotnPlayerParticle){.draw_mode = (Uint16)((Uint16)(profile.draw_mode + 8U) & 0xfff7U)};
        ++next.phase;
    }
    if (next.count != profile.count || next.active < 0 || next.active > next.count)
        return SDL_SetError("Fairy item callback lost its actual allocated row chain");
    if (next.phase == 1)
    {
        next.activation_timer = Word(next.activation_timer - 1);
        if (next.activation_timer <= 0)
        {
            if (next.active + profile.batch > next.count)
                return SDL_SetError("Fairy item activation exceeds its actual primitive chain");
            for (Uint16 i = 0; i < profile.batch; ++i)
                next.tiles[next.active + i].draw_mode &= 0xfff7U;
            next.active = Word(next.active + profile.batch);
            if (next.active >= next.count)
                ++next.phase;
            next.activation_timer = profile.interval;
        }
    }
    next.alive = 0;
    for (Uint16 i = 0; i < next.count; ++i)
    {
        SotnPlayerParticle *tile = &next.tiles[i];
        tile->x = Raster(tile->screen_x, tile->x);
        tile->y = Raster(tile->screen_y, tile->y);
        if (!(tile->draw_mode & 8U))
        {
            tile->x = Fixed((Uint32)tile->x + (Uint32)tile->velocity_x);
            tile->y = Fixed((Uint32)tile->y + (Uint32)tile->velocity_y);
            if (!profile.style)
            {
                tile->velocity_x = TowardZero(tile->velocity_x);
                tile->velocity_y = Fixed((Uint32)tile->velocity_y - 8192U);
                if (Pixel(tile->y) < 0)
                    tile->draw_mode |= 8U;
            }
            else
            {
                if (profile.style == 2)
                {
                    if (tile->red)
                        --tile->red;
                    if (tile->green)
                        --tile->green;
                    if (tile->blue)
                        --tile->blue;
                }
                tile->velocity_y =
                    Fixed((Uint32)tile->velocity_y + (profile.style == 1 ? 8192U : 6144U));
                tile->timer = (Uint16)Word(Word(tile->timer) - 1);
                if (Word(tile->timer) < 0)
                    tile->draw_mode |= 8U;
            }
        }
        tile->screen_x = Pixel(tile->x);
        tile->screen_y = Pixel(tile->y);
        next.alive |= (tile->draw_mode & 8U) ? 0 : 1;
    }
    SotnPlayerParticle *sentinel = &next.tiles[next.count];
    sentinel->draw_mode &= 0xfff7U;
    sentinel->screen_x = sentinel->screen_y = 0;
    sentinel->width = 0;
    if (!next.alive)
        next.removed = true;
    *effect = next;
    *random = rng;
    return true;
}
static bool AppleBirth(SotnFairyAppleEffect *effect, SotnFairyAppleEffectInput in)
{
    effect->x = Fixed((Uint32)in.source.x + (in.source.facing_left ? (Uint32)-4 : 4U) * 65536U);
    effect->y = in.source.y;
    effect->flags = 0x0c800000;
    SDL_memset(effect->quads, 0, sizeof(effect->quads));
    for (Uint16 i = 0; i < SOTN_FAIRY_APPLE_ROWS; ++i)
    {
        SotnPlayerEffectQuad *quad = &effect->quads[i];
        quad->priority = 448;
        quad->draw_mode = 0x435;
        if (i < 2)
            quad->red[1] = quad->red[3] = quad->green[1] = quad->green[3] = quad->blue[1] =
                quad->blue[3] = 255;
        else
        {
            SotnMathTrig left = {0}, right = {0};
            if (!SotnMath_Trig(in.executable, (i - 2) * 256, &left) ||
                !SotnMath_Trig(in.executable, (i - 1) * 256, &right))
                return false;
            quad->x[0] = quad->x[1] = Pixel(effect->x);
            quad->x[2] = Word(Pixel(effect->x) + Shift(left.cosine * 96, 12));
            quad->x[3] = Word(Pixel(effect->x) + Shift(right.cosine * 96, 12));
            quad->y[2] = Word(Shift(left.sine * 96, 12));
            quad->y[3] = Word(Shift(right.sine * 96, 12));
        }
    }
    effect->width = 16;
    effect->height = 0;
    effect->opacity = 64;
    effect->fan_opacity = 0;
    ++effect->phase;
    return true;
}
bool SotnFairyAppleEffect_Tick(SotnFairyAppleEffect *effect, SotnFairyAppleEffectInput in)
{
    if (!effect || effect->removed || effect->phase > 5)
        return SDL_SetError("Fairy Life Apple light requires its actual live ten-row cursor");
    SotnFairyAppleEffect next = *effect;
    switch (next.phase)
    {
    case 0:
        if (!in.allocated)
        {
            next.removed = true;
            *effect = next;
            return true;
        }
        if (!AppleBirth(&next, in))
            return false;
        break;
    case 1:
        next.fan_opacity = Word(next.fan_opacity + 4);
        next.height = Word(next.height + 16);
        if (next.height >= 256)
            ++next.phase;
        break;
    case 2:
        next.timer = Word(next.timer + 1);
        if (next.timer > 15)
            ++next.phase;
        break;
    case 3:
        next.opacity = Word(next.opacity + 4);
        if (next.opacity >= 256)
            next.opacity = 255;
        next.width = Word(next.width + 4);
        if (next.width >= 256)
        {
            next.timer = 0;
            ++next.phase;
        }
        break;
    case 4:
        next.timer = Word(next.timer + 1);
        if (next.timer > 60)
            ++next.phase;
        break;
    case 5:
        next.fan_opacity = Word(next.fan_opacity - 1);
        if (next.fan_opacity < 0)
            next.fan_opacity = 0;
        next.opacity = Word(next.opacity - 4);
        if (next.opacity <= 64)
            next.opacity = 64;
        next.width = Word(next.width - 8);
        if (next.width <= 0)
        {
            next.removed = true;
            *effect = next;
            return true;
        }
        break;
    }
    const Sint32 x = Pixel(next.x);
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnPlayerEffectQuad *quad = &next.quads[i];
        const Sint32 edge = i ? SDL_min(x + next.width, 256) : SDL_max(x - next.width, 0);
        quad->x[0] = quad->x[2] = Word(edge);
        quad->x[1] = quad->x[3] = Word(x);
        quad->y[0] = quad->y[1] = 0;
        quad->y[2] = quad->y[3] = next.height;
        quad->red[0] = quad->red[2] = quad->green[0] = quad->green[2] = quad->blue[0] =
            quad->blue[2] = (Uint8)next.opacity;
    }
    for (Uint16 i = 2; i < SOTN_FAIRY_APPLE_ROWS; ++i)
    {
        SotnPlayerEffectQuad *quad = &next.quads[i];
        for (Uint16 v = 0; v < 4; ++v)
            quad->red[v] = quad->green[v] = quad->blue[v] = v < 2 ? (Uint8)next.fan_opacity : 0;
    }
    *effect = next;
    return true;
}
bool SotnFairyItemEffect_Rebase(SotnFairyItemEffect *effect, Sint16 x, Sint16 y, bool room)
{
    if (!effect || effect->removed || effect->phase > 2 || (effect->phase && !effect->count) ||
        effect->count > SOTN_FAIRY_ITEM_PARTICLES)
        return SDL_SetError("Fairy item translation requires its actual live row chain");
    if (!effect->phase)
        return true;
    if (room || (effect->flags & 0x08000000U))
    {
        effect->x = Fixed((Uint32)effect->x + (Uint32)(Sint32)x * 65536U);
        effect->y = Fixed((Uint32)effect->y + (Uint32)(Sint32)y * 65536U);
    }
    for (Uint16 i = 0; i <= effect->count; ++i)
        if (effect->tiles[i].draw_mode & (room ? 0x100U : 2U))
        {
            effect->tiles[i].screen_x = Word(effect->tiles[i].screen_x + x);
            effect->tiles[i].screen_y = Word(effect->tiles[i].screen_y + y);
        }
    return true;
}
bool SotnFairyAppleEffect_Rebase(SotnFairyAppleEffect *effect, Sint16 x, Sint16 y, bool room)
{
    if (!effect || effect->removed || effect->phase > 5)
        return SDL_SetError("Fairy light translation requires its actual live ten-row cursor");
    if (!effect->phase)
        return true;
    if (room || (effect->flags & 0x08000000U))
    {
        effect->x = Fixed((Uint32)effect->x + (Uint32)(Sint32)x * 65536U);
        effect->y = Fixed((Uint32)effect->y + (Uint32)(Sint32)y * 65536U);
    }
    for (Uint16 i = 0; i < SOTN_FAIRY_APPLE_ROWS; ++i)
        if (effect->quads[i].draw_mode & (room ? 0x100U : 2U))
            for (Uint16 v = 0; v < 4; ++v)
            {
                effect->quads[i].x[v] = Word(effect->quads[i].x[v] + x);
                effect->quads[i].y[v] = Word(effect->quads[i].y[v] + y);
            }
    return true;
}
