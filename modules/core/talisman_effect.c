// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:func_80123B40, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include <SDL3/SDL_error.h>
#include <sotn/talisman_effect.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
bool SotnTalismanEffect_Tick(SotnTalismanEffect *effect, SotnPlayerBlinkShared *shared,
                             SotnPlayerBlinkInput in, Uint16 parameter, Uint32 clock,
                             SotnPlayerBlinkEvents *out)
{
    if (!effect || !shared || !out || parameter > 1 || effect->blink.removed || in.priority < 8)
        return SDL_SetError("Talisman requires its actual child parameter and live owner");
    SotnTalismanEffect e = *effect;
    SotnPlayerBlinkShared phases = *shared;
    SotnPlayerBlinkEvents events = {0};
    const bool birth = !e.blink.phase;
    if (birth)
    {
        e.x = Add((Sint32)in.x * 65536, (Sint32)((Uint32)e.x & 65535U));
        e.y = Add((Sint32)in.y * 65536, (Sint32)((Uint32)e.y & 65535U));
        e.facing_left = in.facing_left;
        e.blink.parameter = 0x26;
    }
    else
    {
        if (clock & 1U)
        {
            e.x = Add(e.x, e.velocity_x);
            e.y = Add(e.y, e.velocity_y);
            e.velocity_y = Add(e.velocity_y, 0x2800);
        }
        in.x = Pixel(e.x);
        in.y = Pixel(e.y);
        in.facing_left = e.facing_left;
    }
    // The original temporarily substitutes only player bank/frame (and later position/facing).
    // Shared locks/status/rotation remain live; no mutation escapes that substitution.
    in.wolf_form = false;
    if (!SotnPlayerBlink_Tick(&e.blink, &phases, in, &events))
        return false;
    if (birth)
    {
        if (in.allocation_failed)
            e.blink.removed = true;
        e.velocity_y = -3 * 65536;
        e.velocity_x = e.facing_left ? 0x1aaaa : -0x1aaaa;
        e.red = e.gray = 128;
    }
    else
    {
        if (e.gray > 4)
            --e.gray;
        if (e.blink.phase == 16 && e.velocity_y > 0)
            e.blink.phase = 17;
        if (e.blink.phase == 17 && e.red > 4)
            --e.red;
        if (e.blink.phase == 18)
            e.blink.removed = true;
    }
    if (!e.blink.removed)
        for (Uint16 i = 0; i < 8; ++i)
        {
            SotnPlayerEffectQuad *q = &e.blink.quads[i];
            q->priority = (Uint16)(in.priority - 8U);
            q->draw_mode = parameter ? 0x117 : 0x137;
            if (parameter)
                q->palette = 0x100;
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->u[v] = (Uint8)(q->u[v] + 128U);
                q->v[v] = (Uint8)(q->v[v] + 128U);
                q->red[v] = parameter ? e.gray : e.red;
                q->green[v] = q->blue[v] = parameter ? e.gray : 0;
            }
        }
    *effect = e;
    *shared = phases;
    *out = events;
    return true;
}
bool SotnTalismanEffect_Rebase(SotnTalismanEffect *e, Sint16 x, Sint16 y)
{
    if (!e)
        return SDL_InvalidParamError("effect");
    e->x = Add(e->x, (Sint32)x * 65536);
    e->y = Add(e->y, (Sint32)y * 65536);
    if (e->blink.phase)
        for (Uint16 i = 0; i < 8; ++i)
            for (Uint16 v = 0; v < 4; ++v)
            {
                e->blink.quads[i].x[v] = Word(e->blink.quads[i].x[v] + x);
                e->blink.quads[i].y[v] = Word(e->blink.quads[i].y[v] + y);
            }
    return true;
}
