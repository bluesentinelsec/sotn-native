// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/84B88.c:EntityExpandingCircle/func_80127CC8 and
// 7879C.c:EntityUnkId24, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original animation/artwork/palette data remains private.
#include <SDL3/SDL.h>
#include <sotn/hellfire_effects.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits <= 32767U ? bits : (Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 High(Sint32 value, Sint16 high)
{
    return Fixed((Uint32)(Uint16)high * 65536U | ((Uint32)value & 65535U));
}
bool SotnHellfireEffect_Tick(SotnHellfireEffect *effect, SotnHellfireEffectInput in,
                             SotnHellfireEffectEvents *out)
{
    if (!effect || !out || effect->removed || effect->phase > 1 ||
        (effect->kind != 28 && effect->kind != 29) ||
        (effect->kind == 28 && !effect->phase && in.player_priority > 510))
        return SDL_SetError("Hellfire effect requires its actual selected kind/phase/player");
    SotnHellfireEffect e = *effect;
    SotnHellfireEffectEvents events = {0};
    const bool ring = e.kind == 28;
    if (!ring && !in.casting)
    {
        e.removed = true;
        goto finish;
    }
    const Sint16 x = ring ? Word((Sint32)in.player_x + (in.facing_left ? 10 : -10)) : in.player_x;
    e.x = High(e.x, x);
    if (ring)
        e.y = High(e.y, Word((Sint32)in.player_y + 2));
    if (!e.phase)
    {
        if (!in.allocated)
        {
            e.removed = true;
            if (!ring)
            {
                events.signal = 0xffff;
                events.signal_changed = true;
            }
            goto finish;
        }
        e.flags = ring ? 0x04840000U : 0x04860000U;
        if (ring)
        {
            e.width = 22;
            e.height = 26;
            e.quad = (SotnPlayerEffectQuad){.u = {64, 127, 64, 127},
                                            .v = {192, 192, 255, 255},
                                            .red = {128, 128, 128, 128},
                                            .green = {128, 128, 128, 128},
                                            .blue = {64, 64, 64, 64},
                                            .page = 0x1a,
                                            .palette = 0x15f,
                                            .priority = (Uint16)(in.player_priority + 1U),
                                            .draw_mode = 0x35};
        }
        else
            e.quad = (SotnPlayerEffectQuad){.red = {192, 192, 192, 192},
                                            .green = {64, 64, 64, 64},
                                            .blue = {64, 64, 64, 64},
                                            .priority = 448,
                                            .draw_mode = 0x315};
        e.phase = 1;
    }
    else if (ring)
    {
        e.width = Word((Sint32)e.width + 2);
        e.height = Word((Sint32)e.height + 2);
        if (e.width > 56)
        {
            e.removed = true;
            goto finish;
        }
    }
    else
    {
        const Sint16 previous = e.timer;
        e.timer = Word((Sint32)e.timer + 1);
        if (previous >= 14)
        {
            e.removed = true;
            goto finish;
        }
    }
    for (Uint16 v = 0; v < 4; ++v)
    {
        e.quad.x[v] =
            Word((Sint32)x + ((v & 1U) ? (ring ? e.width : 3) : -(ring ? (Sint32)e.width : 3)));
        e.quad.y[v] =
            ring       ? Word((Sint32)in.player_y + 2 + ((v & 2U) ? e.height : -(Sint32)e.height))
            : (v & 2U) ? 240
                       : 0;
    }
    if (ring && e.width > 40)
    {
        const Uint8 r = (Uint8)(e.quad.red[3] - 12U), g = (Uint8)(e.quad.green[3] - 12U),
                    b = (Uint8)(e.quad.blue[3] - 6U);
        for (Uint16 v = 0; v < 4; ++v)
        {
            e.quad.red[v] = r;
            e.quad.green[v] = g;
            e.quad.blue[v] = b;
        }
    }
    if (!ring)
    {
        if (in.game_timer & 1U)
            e.quad.draw_mode |= 8U;
        else
            e.quad.draw_mode &= 0xfff7U;
    }
finish:
    *effect = e;
    *out = events;
    return true;
}
bool SotnHellfireTrail_Tick(SotnHellfireTrail *trail, SotnHellfireTrailInput in,
                            SotnHellfireTrailEvents *out)
{
    if (!trail || !out || trail->removed || trail->phase > 1 || (trail->parameter >> 8) != 1 ||
        (!trail->phase && (in.player_priority > 509 || !in.random_ready || in.random > 32767)))
        return SDL_SetError("Hellfire trail requires its actual selected parameter/phase/RNG");
    SotnHellfireTrail t = *trail;
    SotnHellfireTrailEvents e = {0};
    if (!t.phase)
    {
        t.bank = 2;
        t.flags = 0x08100000U;
        t.priority = (Uint16)(in.player_priority + 2U);
        t.velocity_y = -65536;
        t.y = Fixed((Uint32)t.y + (Uint32)((Sint32)(in.random % 24U) - 12) * 65536U);
        t.draw_flags = 3;
        t.scale_x = t.scale_y = 128;
        t.palette = 0x8170;
        t.phase = 1;
        e.initialize = e.random_used = true;
        e.animation_offset = 0xd5fc;
    }
    else if (in.remaining < 0)
        t.removed = true;
    else
    {
        t.x = Fixed((Uint32)t.x + (Uint32)t.velocity_x);
        t.y = Fixed((Uint32)t.y + (Uint32)t.velocity_y);
    }
    *trail = t;
    *out = e;
    return true;
}
