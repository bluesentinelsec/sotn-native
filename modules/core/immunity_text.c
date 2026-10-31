// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntityGuardText parameter0, sotn-decomp
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
    if (!text || text->removed || text->phase > 5)
        return SDL_SetError("Immunity text requires its actual live parameter0 child");
    SotnImmunityText n = *text;
    if (n.phase < 2)
    {
        if (!n.phase || grounded)
            ++n.phase;
        *text = n;
        return true;
    }
    if (n.phase == 5 || (n.phase == 2 && !allocated))
    {
        n.removed = true;
        *text = n;
        return true;
    }
    if (!allocated || !dra.bytes || dra.size < 0xcff4)
        return SDL_SetError("Immunity text requires its private metadata and eighteen real rows");
    const Uint8 *profile = dra.bytes + 0xcfec;
    const Uint16 palette = (Uint16)(profile[4] | (Uint16)profile[5] << 8),
                 mode = (Uint16)(profile[6] | (Uint16)profile[7] << 8);
    if (mode != 1 || profile[2] <= profile[0] || profile[3] <= profile[1])
        return SDL_SetError("Immunity text lost its original selected mode1 metadata");
    switch (n.phase)
    {
    case 2:
        n.x = x;
        n.y = Add(y, 32 * 65536);
        if (n.y > 224 * 65536)
            n.y = 216 * 65536;
        n.timer = 56;
        n.half_width = (Sint16)((profile[2] - profile[0]) / 2);
        n.half_height = n.angle = 0;
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
    *text = n;
    return true;
}
bool SotnImmunityText_Rebase(SotnImmunityText *n, Sint16 x, Sint16 y)
{
    if (!n || n->removed || n->phase > 5)
        return SDL_SetError("Immunity text rebase requires its actual live cursor");
    if (n->phase >= 3)
    {
        n->x = Add(n->x, (Sint32)x * 65536);
        n->y = Add(n->y, (Sint32)y * 65536);
        for (Uint16 v = 0; v < 4; ++v)
        {
            n->quad.x[v] = Word(n->quad.x[v] + x);
            n->quad.y[v] = Word(n->quad.y[v] + y);
        }
    }
    return true;
}
