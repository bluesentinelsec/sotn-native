// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/84B88.c:EntityHellfire, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Gradient pixels remain private.
#include <SDL3/SDL.h>
#include <sotn/hellfire_beam.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits <= 32767U ? bits : (Sint32)bits - 65536);
}
static Sint32 High(Sint32 value, Sint16 high)
{
    const Uint32 bits = (Uint32)(Uint16)high * 65536U | ((Uint32)value & 65535U);
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
bool SotnHellfireBeam_Tick(SotnHellfireBeam *beam, SotnHellfireBeamInput in,
                           SotnHellfireBeamEvents *out)
{
    if (!beam || !out || beam->removed || beam->phase > 7)
        return SDL_SetError("Hellfire beam requires its actual live controller and output");
    SotnHellfireBeam b = *beam;
    SotnHellfireBeamEvents e = {0};
    if (!in.casting)
    {
        b.removed = true;
        goto finish;
    }
    b.x = High(b.x, in.player_x);
    switch (b.phase)
    {
    case 0:
        e.signal_changed = true;
        if (!in.allocated)
        {
            e.signal = 0xffff;
            b.removed = true;
            goto finish;
        }
        b.flags = 0x04860000U;
        b.y = High(b.y, 120);
        e.upload = true;
        b.quad = (SotnPlayerEffectQuad){.u = {1, 48, 1, 48},
                                        .v = {248, 248, 248, 248},
                                        .blue = {0, 0, 0, 128},
                                        .page = 0x11c,
                                        .priority = 0x1c0,
                                        .draw_mode = 0x331};
        e.sound = 0x636;
        ++b.phase;
        break;
    case 1:
        b.height = Word((Sint32)b.height + 16);
        if (b.height >= 128)
            ++b.phase;
        break;
    case 2:
        b.width = Word((Sint32)b.width + 4);
        if (b.width >= 22)
        {
            b.timer = 32;
            ++b.phase;
        }
        break;
    case 3:
        b.timer = Word((Sint32)b.timer - 1);
        if (!b.timer)
        {
            e.palette = 0x810d;
            ++b.phase;
        }
        break;
    case 4:
        e.palette = 0x810d;
        b.width = Word((Sint32)b.width - 8);
        if (b.width < 0)
        {
            ++b.phase;
            b.timer = 42;
            b.quad.draw_mode |= 8U;
            e.signal_changed = true;
            e.signal = 1;
        }
        break;
    case 5:
        e.palette = 0x810d;
        if (b.timer == 16)
            e.factory = 38;
        b.timer = Word((Sint32)b.timer - 1);
        if (!b.timer)
        {
            ++b.phase;
            b.width = 0;
            b.quad.draw_mode &= 0xfff7U;
        }
        if (b.timer == 2)
        {
            e.signal_changed = true;
            e.signal = 2;
        }
        break;
    case 6:
        e.palette = 0x8100;
        b.width = Word((Sint32)b.width + 4);
        if (b.width >= 22)
        {
            ++b.phase;
            b.timer = 96;
        }
        break;
    case 7:
        if (b.timer == 80)
        {
            e.signal_changed = true;
            e.signal = 3;
            e.factory = 0x000a0004U;
        }
        if (b.timer == 48)
            e.factory = in.up ? 35 : 34;
        if (b.timer < 72)
        {
            b.width = Word((Sint32)b.width - 4);
            if (b.width < 0)
                b.width = 0;
        }
        b.timer = Word((Sint32)b.timer - 1);
        if (b.timer == 40)
        {
            b.removed = true;
            goto finish;
        }
        break;
    }
    b.y = High(b.y, 120);
    for (Uint16 v = 0; v < 4; ++v)
    {
        b.quad.v[v] = (in.game_timer & 1U) ? 248 : 252;
        b.quad.x[v] = Word((Sint32)in.player_x + ((v & 1U) ? b.width : -(Sint32)b.width));
        b.quad.y[v] = Word(120 + ((v & 2U) ? b.height : -(Sint32)b.height));
    }
    if (b.phase == 3 || b.phase == 7)
    {
        Uint8 value = b.quad.blue[3];
        if (b.phase == 3 && value < 248)
            value = (Uint8)(value + 4U);
        if (b.phase == 7 && value > 32)
            value = (Uint8)(value - 32U);
        for (Uint16 v = 0; v < 4; ++v)
            b.quad.red[v] = b.quad.green[v] = b.quad.blue[v] = value;
    }
finish:
    *beam = b;
    *out = e;
    return true;
}
