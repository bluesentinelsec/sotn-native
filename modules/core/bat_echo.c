// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/8BEF8.c:EntityBatEcho, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original shape/color and SDK tables remain private runtime data.
#include <SDL3/SDL_error.h>
#include <sotn/bat_echo.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Floor(Sint32 value, Sint32 divisor)
{
    return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnBatEcho_Tick(SotnBatEcho *echo, SotnBatEchoInput in, SotnBatEchoEvents *events)
{
    if (!echo || !events || echo->removed || echo->phase > 3 ||
        (echo->phase == 0 && (in.priority < 2 || in.priority > 511)) ||
        (echo->phase && (echo->intensity < 0 || echo->intensity > 256 ||
                         (echo->limit != 160 && echo->limit != 256) || echo->priority > 509)))
        return SDL_SetError("Echo requires its actual bounded child, lifetime and birth priority");
    SotnBatEcho e = *echo;
    SotnBatEchoEvents out = {0};
    if (e.age % 384 == 0)
        out.sound = 0x61f;
    e.age = Word(e.age + 1);
    if (e.retire_ticks)
        e.phase = 3;
    if (e.phase == 0)
    {
        if (!in.primitives_allocated)
        {
            e.removed = true;
            goto done;
        }
        if (!in.dra.bytes || in.dra.size < 0x108a8U + 34U)
            return SDL_SetError("Echo requires its private seventeen-point shape/color tables");
        for (Uint16 i = 0; i < 17; ++i)
        {
            const Uint16 shape = Read16(in.dra.bytes + 0x10884U + i * 2U),
                         color = Read16(in.dra.bytes + 0x108a8U + i * 2U);
            if (shape > 5 || color > 255)
                return SDL_SetError("Echo private shape/color value is invalid");
            e.drivers[i].shape = (Sint16)shape;
            e.drivers[i].color = (Sint16)color;
            e.drivers[i].angle = (Sint16)((i << 10) & 4095U);
            e.drivers[i].color_angle = (Sint16)((i << 7) & 4095U);
        }
        e.priority = (Uint16)(in.priority - 2);
        e.limit = in.force_enabled ? 256 : 160;
        out.initialize_attack = in.force_enabled;
        e.phase = 1;
    }
    else if (e.phase == 1)
    {
        e.intensity = (Sint16)(e.intensity + 10);
        if (e.intensity > e.limit)
        {
            e.intensity = e.limit;
            e.phase = 2;
            out.disable_attack = true;
        }
    }
    else if (e.phase == 2)
    {
        e.intensity = (Sint16)(e.intensity - 10);
        if (e.intensity < 0)
        {
            e.intensity = 0;
            e.removed = true;
            goto done;
        }
    }
    else
    {
        e.hidden = true;
        e.retire_ticks = Word(e.retire_ticks - 1);
        if (!e.retire_ticks)
            e.removed = true;
        goto done;
    }
    Sint32 center[17] = {0};
    Uint8 red[17] = {0}, green[17] = {0}, blue[17] = {0};
    SotnMathTrig pose = {0};
    if (!SotnMath_Trig(in.executable, in.rotation, &pose))
        return false;
    const Sint32 origin_y = in.y + Floor(Floor(pose.sine, 16) * 10, 256) - 6;
    for (Uint16 i = 0; i < 17; ++i)
    {
        if (e.drivers[i].shape < 0 || e.drivers[i].shape > 5 || e.drivers[i].color < 0 ||
            e.drivers[i].color > 255)
            return SDL_SetError("Echo driver metadata exceeds its private table bounds");
        e.drivers[i].angle = Word(e.drivers[i].angle + 256);
        e.drivers[i].color_angle = Word(e.drivers[i].color_angle - 64);
        SotnMathTrig wave = {0}, hue = {0}, shifted = {0};
        if (!SotnMath_Trig(in.executable, e.drivers[i].angle, &wave) ||
            !SotnMath_Trig(in.executable, e.drivers[i].color_angle, &hue) ||
            !SotnMath_Trig(in.executable, e.drivers[i].color_angle + 256, &shifted))
            return false;
        center[i] = origin_y + Floor(Floor(wave.sine, 16) * e.drivers[i].shape, 512);
        const Sint32 tint = e.drivers[i].color * e.intensity;
        red[i] = (Uint8)((Floor(hue.cosine, 64) + 65) * tint / 65536);
        green[i] = (Uint8)((Floor(shifted.cosine, 64) + 65) * tint / 65536);
        blue[i] = (Uint8)((-Floor(hue.cosine, 64) + 65) * tint / 65536);
    }
    const Sint32 direction = e.facing_left ? -1 : 1, origin_x = in.x + direction * 16;
    for (Uint16 i = 0; i < 16; ++i)
    {
        const Sint32 left = origin_x + direction * i * 8, right = left + direction * 8;
        const Sint32 previous_width = i * 3 + 1, next_width = (i + 1) * 3 + 1;
        for (Uint16 side = 0; side < 2; ++side)
        {
            SotnBatEchoQuad q = {.x = {Word(left), Word(right), Word(left), Word(right)},
                                 .y = {Word(center[i]), Word(center[i + 1]),
                                       Word(center[i] + (side ? previous_width : -previous_width)),
                                       Word(center[i + 1] + (side ? next_width : -next_width))},
                                 .red = {red[i], red[i + 1], 0, 0},
                                 .green = {green[i], green[i + 1], 0, 0},
                                 .blue = {blue[i], blue[i + 1], 0, 0}};
            e.quads[i * 2U + side] = q;
        }
    }
done:
    *echo = e;
    *events = out;
    return true;
}
