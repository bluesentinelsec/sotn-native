// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:EntityPlayerDissolves, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Pattern/artwork tables remain private.
#include <sotn/combat.h>
#include <sotn/stone_dissolve.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
static Sint32 High(Sint32 value, Sint16 high)
{
    const Uint32 bits = (Uint32)(Uint16)high << 16 | ((Uint32)value & 65535U);
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool Upload(SotnStoneDissolveInput in, const SotnDeathImage *image, bool secondary)
{
    if (!image->captured || !in.upload)
        return SDL_SetError("Stone dissolve requires its actual captured image/upload owner");
    return in.upload(in.context, image, secondary);
}
static void Position(SotnStoneDissolve *n, SotnStoneDissolveInput in)
{
    const Sint16 x = Word(Pixel(n->x) + (in.facing_left ? -in.image.pivot_x : in.image.pivot_x)),
                 y = Word(Pixel(n->y) + in.image.pivot_y);
    const Sint16 w = (Sint16)(((Sint16)(Uint8)(4U + in.image.width) - 4) / 6),
                 h = (Sint16)(((Sint16)(Uint8)(1U + in.image.height) - 1) / 6);
    for (Uint16 i = 0; i < SOTN_STONE_DISSOLVE_ROWS; ++i)
    {
        SotnPlayerEffectQuad *q = &n->quads[i];
        const Sint16 left = Word(x + (in.facing_left ? 1 - w * (i % 6) : w * (i % 6))),
                     right = Word(left + (in.facing_left ? -w : w)), top = Word(y + h * (i / 6));
        for (Uint16 v = 0; v < 4; ++v)
        {
            q->x[v] = v & 1U ? right : left;
            q->y[v] = Word(top + (v & 2U ? h : 0));
        }
    }
}
static void Nibbles(SotnDeathImage *image, Uint8 cursor, bool stone, Uint32 *random)
{
    const Uint16 base = (Uint16)(((cursor >> 1) & 7U) + (cursor >> 4) * 64U);
    for (Uint16 i = 0; i < 40; ++i)
    {
        Uint8 *p = &image->pixels[base + (i & 7U) * 8U + (i >> 3) * 1024U];
        if (stone)
        {
            if (!(SotnCombat_Random(random) & 3U) || !*p)
                continue;
            *p = (Uint8)((*p & (cursor & 1U ? 0xf0U : 0x0fU)) | (cursor & 1U ? 1U : 0x10U));
        }
        else
            *p = (Uint8)(*p & (cursor & 1U ? 0xf0U : 0x0fU));
    }
}
bool SotnStoneDissolve_Tick(SotnStoneDissolve *effect, SotnDeathImage *image,
                            SotnStoneDissolveInput in, Uint32 *random, SotnStoneDissolveEvents *out)
{
    if (!effect || !image || !random || !out || effect->removed || effect->phase > 4 ||
        effect->pattern > 7 || effect->index > 7 ||
        (effect->phase == 4 && (!effect->timer || effect->timer > 48)) ||
        (effect->phase && !in.allocated) || (effect->phase >= 2 && !image->captured) ||
        in.priority > 509)
        return SDL_SetError("Stone dissolve requires its actual live cursor/rows/body owner");
    SotnStoneDissolve n = *effect;
    SotnDeathImage pixels = *image;
    SotnStoneDissolveEvents events = {0};
    Uint32 rng = *random;
    if (in.player_animation != 0x38 && in.player_animation != 0x39 && in.player_animation != 0x3a &&
        (n.phase == 2 || n.phase == 3))
    {
        if (!Upload(in, &pixels, true))
            return false;
        n.phase = 4;
        n.cursor = (Uint8)SotnCombat_Random(&rng);
        n.timer = 48;
        for (Uint16 i = 0; i < SOTN_STONE_DISSOLVE_ROWS; ++i)
            n.quads[i].draw_mode &= 0xfff7U;
    }
    n.x = High(n.x, Pixel(in.player_x));
    n.y = High(n.y, Pixel(in.player_y));
    n.facing_left = in.facing_left;
    if (!(in.player_sprite & 0x7fffU))
    {
        n.removed = true;
        goto done;
    }
    if (!in.image.width || in.image.width > 255 || !in.image.height || in.image.height > 255)
        return SDL_SetError("Stone dissolve requires actual decoded private sprite geometry");
    switch (n.phase)
    {
    case 0:
        if (!in.allocated)
            goto done;
        n.flags = 0x08800000U;
        n.index = 0;
        n.pattern = (Uint8)(SotnCombat_Random(&rng) & 7U);
        if (!in.dra.bytes || in.dra.size < 0xe180U)
            return SDL_SetError("Stone dissolve requires its original private pattern rows");
        n.cursor = in.dra.bytes[0xe140U + n.pattern * 8U];
        for (Uint16 i = 0; i < SOTN_STONE_DISSOLVE_ROWS; ++i)
        {
            SotnPlayerEffectQuad *q = &n.quads[i];
            const Sint16 w = (Sint16)(((Sint16)(Uint8)(4U + in.image.width) - 4) / 6),
                         h = (Sint16)(((Sint16)(Uint8)(1U + in.image.height) - 1) / 6);
            *q = (SotnPlayerEffectQuad){.page = 0x18,
                                        .palette = (Uint16)(in.palette & 0x7fffU),
                                        .priority = (Uint16)(in.priority + 2U),
                                        .draw_mode = 0x308};
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->u[v] = (Uint8)(132U + w * (i % 6U + (v & 1U)));
                q->v[v] = (Uint8)(1U + h * (i / 6U + (v & 2U ? 1U : 0U)));
            }
            q->green[0] = (Uint8)(i / 6U * 2U + (SotnCombat_Random(&rng) & 3U));
        }
        Position(&n, in);
        n.phase = 1;
        break;
    case 1:
        if (!in.capture)
            return SDL_SetError("Stone dissolve requires its actual StoreImage owner");
        if (!in.capture(in.context, &pixels))
            return false;
        if (!pixels.captured)
            return SDL_SetError("Stone dissolve requires a completed resident StoreImage copy");
        Position(&n, in);
        n.phase = 2;
        SDL_FALLTHROUGH;
    case 2:
        events.frame_load = true;
        if (in.tapped & 0xf000U)
            n.phase = 3;
        Position(&n, in);
        break;
    case 3:
        if (!in.dra.bytes || in.dra.size < 0xe180U || !pixels.captured)
            return SDL_SetError("Stone dissolve lost its actual image/private pattern row");
        events.frame_load = true;
        n.cursor = in.dra.bytes[0xe140U + n.pattern * 8U + n.index];
        Nibbles(&pixels, n.cursor, true, &rng);
        if (!Upload(in, &pixels, false))
            return false;
        if (++n.index == 8)
        {
            n.index = 0;
            n.pattern = (Uint8)((n.pattern + 1U) & 7U);
            n.phase = 2;
        }
        Position(&n, in);
        break;
    case 4:
        if (in.player_step == 16 ? !(in.engine_timer & 1U) : !(in.engine_timer % 3U))
            break;
        for (Uint16 i = 0; i < 6; ++i)
        {
            Nibbles(&pixels, n.cursor, false, &rng);
            n.cursor = (Uint8)(n.cursor + 35U);
        }
        if (!Upload(in, &pixels, true))
            return false;
        if (!--n.timer)
        {
            n.removed = true;
            goto done;
        }
        for (Uint16 i = 0; i < SOTN_STONE_DISSOLVE_ROWS; ++i)
        {
            SotnPlayerEffectQuad *q = &n.quads[i];
            q->draw_mode = (Uint16)((q->draw_mode & 0x200U) | 2U);
            if (!q->red[0])
            {
                q->green[0] = (Uint8)(q->green[0] - 1U);
                if (!q->green[0])
                {
                    ++q->red[0];
                    q->blue[0] = (Uint8)((SotnCombat_Random(&rng) & 3U) + 248U);
                    q->red[1] = (Uint8)(32U - i / 6U * 2U);
                }
            }
            else
            {
                if (q->red[1])
                    --q->red[1];
                else
                    q->draw_mode = 8;
                if (q->blue[0] < 48 || q->blue[0] > 208)
                    q->blue[0] = (Uint8)(q->blue[0] + 2U);
                const Sint32 signed_byte = q->blue[0] < 128 ? q->blue[0] : (Sint32)q->blue[0] - 256;
                const Sint32 dy = signed_byte >= 0 ? signed_byte / 16 : -((-signed_byte + 15) / 16);
                for (Uint16 v = 0; v < 4; ++v)
                    q->y[v] = Word(q->y[v] + dy);
            }
        }
        break;
    }
    events.reset_afterimages = true;
done:
    *effect = n;
    *image = pixels;
    *random = rng;
    *out = events;
    return true;
}
bool SotnStoneDissolve_Rebase(SotnStoneDissolve *n, Sint16 x, Sint16 y, bool room)
{
    if (!n || n->removed || n->phase > 4)
        return SDL_SetError("Stone dissolve rebase requires its actual bounded entity");
    if (n->phase)
    {
        if (room || (n->flags & 0x08000000U))
        {
            n->x = High(n->x, Word(Pixel(n->x) + x));
            n->y = High(n->y, Word(Pixel(n->y) + y));
        }
        for (Uint16 i = 0; i < SOTN_STONE_DISSOLVE_ROWS; ++i)
            if (n->quads[i].draw_mode & (room ? 0x100U : 2U))
                for (Uint16 v = 0; v < 4; ++v)
                {
                    n->quads[i].x[v] = Word(n->quads[i].x[v] + x);
                    n->quads[i].y[v] = Word(n->quads[i].y[v] + y);
                }
    }
    return true;
}
