// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntityGuardText parameters0..3 and medicine8..19, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Text metadata/artwork remain private.
#include <SDL3/SDL.h>
#include <sotn/immunity_text.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    Sint32 result;
    SDL_memcpy(&result, &bits, sizeof(result));
    return result;
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
bool SotnImmunityText_Tick(SotnImmunityText *text, SotnDataView dra, SotnDataView exe, Sint32 x,
                           Sint32 y, bool grounded, bool allocated)
{
    return SotnImmunityText_TickParameter(text, dra, exe, x, y, grounded, allocated, 0);
}
bool SotnImmunityText_TickParameter(SotnImmunityText *text, SotnDataView dra, SotnDataView exe,
                                    Sint32 x, Sint32 y, bool grounded, bool allocated,
                                    Uint16 parameter)
{
    if (!text || text->removed ||
        (parameter > 0x300 && (parameter < 0x800 || parameter > 0x1300)) || (parameter & 255U) ||
        text->phase > (parameter ? 7 : 5))
        return SDL_SetError("Text requires its actual live immunity/ailment/medicine child");
    SotnImmunityText n = *text;
    n.sound = 0;
    if (n.phase < 2)
    {
        if (!n.phase || grounded)
            ++n.phase;
        *text = n;
        return true;
    }
    if ((!parameter && n.phase == 5) || (n.phase == 2 && !allocated))
    {
        n.removed = true;
        *text = n;
        return true;
    }
    const size_t offset = 0xcfecU + (parameter >> 8) * 8U;
    if (!allocated || !dra.bytes || dra.size < offset + 8U)
        return SDL_SetError("Immunity text requires its private metadata and eighteen real rows");
    const Uint8 *profile = dra.bytes + offset;
    const Uint16 palette = (Uint16)(profile[4] | (Uint16)profile[5] << 8),
                 mode = (Uint16)(profile[6] | (Uint16)profile[7] << 8);
    const Uint16 expected_mode = parameter < 0x800 ? 1 : parameter <= 0xc00 ? 2 : 3;
    if (mode != expected_mode || profile[2] <= profile[0] || profile[3] <= profile[1])
        return SDL_SetError("Immunity text lost its original selected label metadata");
    Sint16 hider = -1;
    switch (n.phase)
    {
    case 2:
        n.x = x;
        n.y = Add(y, 32 * 65536);
        if (n.y > 224 * 65536)
            n.y = 216 * 65536;
        n.timer = 56;
        n.half_width = (Sint16)((profile[2] - profile[0]) / 2);
        if (mode == 2)
            n.x = Add(n.x, -6 * 65536);
        else if (mode == 3)
            n.x = Add(n.x, 32 * 65536);
        n.target = (mode == 3 ? 176 : 120) * 65536 + n.half_width;
        n.suffix.draw_mode = 10;
        n.half_height = n.angle = 0;
        for (Uint16 i = 0; i < 16; ++i)
            n.tiles[i].draw_mode = 10;
        ++n.phase;
        break;
    case 3: {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(exe, n.angle, &trig))
            return false;
        const Sint32 sine = trig.sine * 2;
        n.half_height = (Sint16)((sine >= 0 ? sine / 4096 : -((-sine + 4095) / 4096)) + 4);
        n.angle = Word(SDL_min(n.angle, 4096) + 128);
        n.timer = Word(n.timer - 1);
        if (!n.timer)
        {
            n.velocity_x = 6 * 65536;
            ++n.phase;
        }
        break;
    }
    case 4:
        n.velocity_x = Add(n.velocity_x, 65536);
        n.x = Add(n.x, n.velocity_x);
        if (n.half_height > 1)
            --n.half_height;
        n.half_width = Word(n.half_width + 5);
        if (n.x > 384 * 65536)
        {
            n.x = 384 * 65536;
            n.y = 32 * 65536;
            n.velocity_x = -10 * 65536;
            n.half_width = 30;
            ++n.phase;
        }
        n.timer = Word(n.timer + 1);
        break;
    case 5:
        n.x = Add(n.x, n.velocity_x);
        n.timer = Word(n.timer + 1);
        hider = (Sint16)(n.timer & 15);
        if (n.half_width > 12)
            n.half_width = Word(n.half_width - 5);
        if (n.x < n.target)
        {
            n.half_width = Word(n.target);
            n.x = n.target;
            n.timer = 96;
            n.angle = 0;
            n.sound = 0x6af;
            ++n.phase;
        }
        break;
    case 6: {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(exe, n.angle, &trig))
            return false;
        const Sint32 sine = trig.sine * 2;
        n.half_height = (Sint16)((sine >= 0 ? sine / 4096 : -((-sine + 4095) / 4096)) + 4);
        n.angle = Word(n.angle + 128);
        if (n.angle > 4096)
            n.angle = 4096;
        n.timer = Word(n.timer - 1);
        if (!n.timer)
        {
            n.velocity_x = -6 * 65536;
            ++n.phase;
        }
        break;
    }
    case 7:
        n.velocity_x = Add(n.velocity_x, -65536);
        n.x = Add(n.x, n.velocity_x);
        n.timer = Word(n.timer + 1);
        hider = (Sint16)(n.timer & 15);
        if (n.half_height > 1)
            --n.half_height;
        n.half_width = Word(n.half_width + 5);
        if (n.x < -128 * 65536)
        {
            n.removed = true;
            *text = n;
            return true;
        }
        break;
    }
    const Sint16 px = Pixel(n.x), py = Pixel(n.y);
    n.quad = (SotnPlayerEffectQuad){.x = {Word(px - n.half_width), Word(px + n.half_width),
                                          Word(px - n.half_width), Word(px + n.half_width)},
                                    .y = {Word(py - n.half_height), Word(py - n.half_height),
                                          Word(py + n.half_height), Word(py + n.half_height)},
                                    .u = {profile[0], profile[2], profile[0], profile[2]},
                                    .v = {profile[1], profile[1], profile[3], profile[3]},
                                    .page = 0x1a,
                                    .palette = palette,
                                    .priority = 0x1b8,
                                    .draw_mode = 0x100};
    if (mode == 2 || mode == 3)
    {
        const Uint8 *fixed = dra.bytes + 0xcfecU + (mode == 2 ? 5U : 6U) * 8U;
        if (fixed[2] <= fixed[0] || fixed[3] <= fixed[1])
            return SDL_SetError("Medicine label lost its actual private suffix geometry");
        n.suffix = n.quad;
        n.suffix.u[0] = n.suffix.u[2] = fixed[0];
        n.suffix.u[1] = n.suffix.u[3] = fixed[2];
        n.suffix.v[0] = n.suffix.v[1] = fixed[1];
        n.suffix.v[2] = n.suffix.v[3] = fixed[3];
        const Sint32 extra = (n.half_width - Word(n.target)) * 2;
        if (mode == 2)
        {
            n.suffix.x[0] = n.suffix.x[2] = n.quad.x[1];
            n.suffix.x[1] = n.suffix.x[3] = Word(n.suffix.x[0] + fixed[2] - fixed[0] + extra);
        }
        else
        {
            n.suffix.x[1] = n.suffix.x[3] = Word(n.quad.x[0] - 3);
            n.suffix.x[0] = n.suffix.x[2] = Word(n.suffix.x[1] - (fixed[2] - fixed[0]) - extra);
        }
    }
    for (Uint16 i = 0; i < 16; ++i)
        if (n.tiles[i].active)
        {
            if (!--n.tiles[i].timer)
            {
                n.tiles[i].draw_mode = 8;
                n.tiles[i].active = false;
            }
            n.tiles[i].y = Word(n.tiles[i].y + 1);
        }
        else if (hider == i)
        {
            n.tiles[i].draw_mode = (Uint16)(n.tiles[i].draw_mode & 0xfff7U);
            n.tiles[i].x = px;
            n.tiles[i].y = py;
            n.tiles[i].active = true;
            n.tiles[i].timer = 12;
        }
    *text = n;
    return true;
}
bool SotnImmunityText_Rebase(SotnImmunityText *n, Sint16 x, Sint16 y)
{
    if (!n || n->removed || n->phase > 7)
        return SDL_SetError("Immunity text rebase requires its actual live cursor");
    if (n->phase >= 3)
    {
        if (n->suffix.draw_mode & 2U)
            for (Uint16 v = 0; v < 4; ++v)
            {
                n->suffix.x[v] = Word(n->suffix.x[v] + x);
                n->suffix.y[v] = Word(n->suffix.y[v] + y);
            }
        for (Uint16 i = 0; i < 16; ++i)
            if (n->tiles[i].draw_mode & 2U)
            {
                n->tiles[i].x = Word(n->tiles[i].x + x);
                n->tiles[i].y = Word(n->tiles[i].y + y);
            }
    }
    return true;
}
bool SotnImmunityText_ChangeRoom(SotnImmunityText *n, Sint16 x, Sint16 y)
{
    if (!n || n->removed || n->phase > 7)
        return SDL_SetError("Text room entry requires its actual live cursor");
    if (n->phase >= 3)
        for (Uint16 v = 0; v < 4; ++v)
        {
            n->quad.x[v] = Word(n->quad.x[v] + x);
            n->quad.y[v] = Word(n->quad.y[v] + y);
            if (n->suffix.draw_mode & 0x100U)
            {
                n->suffix.x[v] = Word(n->suffix.x[v] + x);
                n->suffix.y[v] = Word(n->suffix.y[v] + y);
            }
        }
    return true;
}
