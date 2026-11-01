// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:EntityTransparentWhiteCircle, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles and SDK tables stay private.
#include <SDL3/SDL.h>
#include <sotn/player_circle.h>

static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}
static Sint32 High(Sint32 value, Sint16 high)
{
    const Uint32 bits = (Uint32)(Uint16)high << 16 | ((Uint32)value & 65535U);
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint32 scale = 1 << bits;
    return value >= 0 ? value / scale : (Sint32)(-((-(Sint64)value + scale - 1) / scale));
}
static bool Color(SotnDataView executable, Sint16 angle, Sint16 alpha, Sint16 scale, Uint8 *out)
{
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(executable, angle, &trig))
        return false;
    *out = (Uint8)((Shift(trig.sine + 4096, 7) * alpha) / scale);
    return true;
}
static bool Geometry(SotnPlayerCircle *n, SotnPlayerCircleInput in, const Sint16 profile[6])
{
    const Sint32 x = Pixel(n->x), y = Pixel(n->y);
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnPlayerEffectQuad *outer = &n->quads[i], *inner = &n->quads[i + 16U];
        for (Uint16 v = 0; v < 2; ++v)
        {
            outer->x[v] = Word(x + ((Sint32)outer->u[v] - 32) * n->outer_x / 256);
            outer->y[v] = Word(y + ((Sint32)outer->v[v] - 224) * n->outer_y / 256);
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(in.executable, (i + v + 1U) * 256, &trig))
                return false;
            inner->x[v + 2U] = Word(x + Shift(Shift(trig.cosine, 4) * 32, 8) * n->inner_x / 256);
            inner->y[v + 2U] = Word(y - Shift(Shift(trig.sine, 4) * 32, 8) * n->inner_y / 256);
            outer->x[v + 2U] = inner->x[v] = Word(((Sint32)outer->x[v] + inner->x[v + 2U]) / 2);
            outer->y[v + 2U] = inner->y[v] = Word(((Sint32)outer->y[v] + inner->y[v + 2U]) / 2);
            const Sint16 angle = Word(n->angle + (Sint32)(i + v) * n->stride);
            if (!Color(in.executable, angle, n->alpha, profile[2], &inner->red[v]) ||
                !Color(in.executable, Word(angle + profile[0]), n->alpha, profile[3],
                       &inner->green[v]) ||
                !Color(in.executable, Word(angle + profile[1]), n->alpha, profile[4],
                       &inner->blue[v]))
                return false;
            outer->red[v + 2U] = inner->red[v];
            outer->green[v + 2U] = inner->green[v];
            outer->blue[v + 2U] = inner->blue[v];
            outer->red[v] = outer->green[v] = outer->blue[v] = 0;
            inner->red[v + 2U] = inner->green[v + 2U] = inner->blue[v + 2U] = 0;
        }
    }
    return true;
}
bool SotnPlayerCircle_Tick(SotnPlayerCircle *circle, SotnPlayerCircleInput in,
                           SotnPlayerCircleEvents *out)
{
    const Uint16 parameter = circle ? (circle->parameter >> 8) & 0x7fU : 0;
    const size_t at = 0xdde8U + (parameter & 63U) * 12U;
    if (!circle || !out || circle->removed || circle->phase > 3 || (parameter & 63U) >= 22 ||
        in.priority > 509 || !in.dra.bytes || at > in.dra.size || in.dra.size - at < 12)
        return SDL_SetError("Player circle requires its actual lifecycle and private profile");
    Sint16 profile[6];
    for (Uint16 i = 0; i < 6; ++i)
        profile[i] = Word(in.dra.bytes[at + i * 2U] | (Uint16)in.dra.bytes[at + i * 2U + 1U] << 8);
    if (!profile[2] || !profile[3] || !profile[4])
        return SDL_SetError("Player circle private color divisors must be nonzero");
    SotnPlayerCircle n = *circle;
    const SotnPlayerCircleEvents events = {.reset_afterimages = true};
    n.x = High(n.x, Word(Pixel(in.player_x)));
    n.y = High(n.y, Word(Pixel(in.player_y)));
    const bool expanding = (parameter & 0x40U) != 0;
    switch (n.phase)
    {
    case 0:
        if (!in.allocated)
        {
            n.removed = true;
            goto done;
        }
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad *outer = &n.quads[i], *inner = &n.quads[i + 16U];
            *outer = *inner = (SotnPlayerEffectQuad){.page = 0x1a,
                                                     .palette = 0x15f,
                                                     .priority = (Uint16)(in.priority + 2U),
                                                     .draw_mode = 0x235};
            for (Uint16 v = 0; v < 2; ++v)
            {
                SotnMathTrig trig = {0};
                if (!SotnMath_Trig(in.executable, (i + v) * 256, &trig))
                    return false;
                outer->u[v] = (Uint8)(Shift(Shift(trig.cosine, 4) * 32, 8) + 32);
                outer->v[v] = (Uint8)(-Shift(Shift(trig.sine, 4) * 32, 8) + 223);
                inner->u[v + 2U] = 32;
                inner->v[v + 2U] = 223;
                outer->u[v + 2U] = inner->u[v] = (Uint8)(((Uint16)outer->u[v] + 32U) / 2U);
                outer->v[v + 2U] = inner->v[v] = (Uint8)(((Uint16)outer->v[v] + 223U) / 2U);
            }
        }
        n.outer_x = n.outer_y = expanding ? 64 : 640;
        n.inner_x = n.inner_y = expanding ? 0 : 576;
        n.stride = profile[5];
        n.alpha = 192;
        n.phase = 1;
        break;
    case 1:
        n.angle = Word(n.angle + 64);
        if (!expanding)
        {
            n.inner_y = Word(n.inner_y - 10);
            if (n.inner_y < 0)
            {
                n.inner_y = 0;
                n.timer = 32;
                ++n.phase;
            }
            n.inner_x = n.inner_y;
            n.outer_y = Word(n.outer_y - 5);
            n.outer_x = Word(n.outer_x - 5);
        }
        else
        {
            n.inner_y = Word(n.inner_y + 9);
            n.inner_x = Word(n.inner_x + 6);
            if (n.inner_y > 512)
            {
                n.inner_y = 512;
                ++n.phase;
            }
            n.outer_y = Word(n.outer_y + 3);
            n.outer_x = Word(n.outer_x + 2);
        }
        break;
    case 2:
        n.angle = Word(n.angle + 64);
        if (!expanding)
        {
            n.outer_y = Word(n.outer_y - 3);
            n.outer_x = Word(n.outer_x - 6);
            n.timer = Word(n.timer - 1);
            if (!n.timer)
                ++n.phase;
        }
        else
        {
            n.alpha = Word(n.alpha - 4);
            if (n.alpha < 0)
            {
                n.removed = true;
                goto done;
            }
        }
        break;
    case 3:
        n.angle = Word(n.angle + 64);
        n.outer_y = Word(n.outer_y - 3);
        n.outer_x = Word(n.outer_x - 6);
        n.alpha = Word(n.alpha - 12);
        if (n.alpha < 0)
        {
            n.removed = true;
            goto done;
        }
        break;
    }
    if (!Geometry(&n, in, profile))
        return false;
done:
    *circle = n;
    *out = events;
    return true;
}
bool SotnPlayerCircle_Rebase(SotnPlayerCircle *n, Sint16 x, Sint16 y, bool room)
{
    if (!n || n->removed || n->phase > 3)
        return SDL_SetError("Player circle rebase requires its actual lifecycle");
    if (n->phase && room)
    {
        n->x = High(n->x, Word(Pixel(n->x) + x));
        n->y = High(n->y, Word(Pixel(n->y) + y));
    }
    // Original235 packets have neither ordinary camera2 nor room100 translation.
    return true;
}
bool SotnPlayerCircle_Displace(SotnPlayerCircle *n, Sint32 x, Sint32 y)
{
    if (!n || n->removed || n->phase > 3)
        return SDL_SetError("Player circle shove requires its actual lifecycle");
    if (n->phase)
    {
        Uint32 bits = (Uint32)n->x + (Uint32)x;
        SDL_memcpy(&n->x, &bits, sizeof(bits));
        bits = (Uint32)n->y + (Uint32)y;
        SDL_memcpy(&n->y, &bits, sizeof(bits));
    }
    return true;
}
