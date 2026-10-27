// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntityPlayerBlinkWhite and 4B758.c:func_800EB758,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles remain private.
#include <SDL3/SDL.h>
#include <sotn/player_blink.h>

static Sint32 Shift(Sint64 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-value + scale - 1) / scale));
}
static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint16 Read16(const Uint8 *p)
{
    return Word(p[0] | (Uint16)p[1] << 8);
}
bool SotnPlayerEffect_Rotate(SotnDataView executable, Sint16 x, Sint16 y, Sint16 rotation,
                             bool facing_left, SotnPlayerEffectQuad *quad)
{
    if (!quad)
        return SDL_InvalidParamError("quad");
    SotnPlayerEffectQuad next = *quad;
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Sint16 dx = Word((Sint32)next.x[i] - x), dy = Word((Sint32)next.y[i] - y);
        const Sint64 square = (Sint64)dx * dx + (Sint64)dy * dy;
        Uint32 distance = 0;
        Sint16 angle = 0;
        SotnMathTrig trig = {0};
        if (square > 0x7ffff || !SotnMath_Sqrt12(executable, (Uint32)square * 4096U, &distance) ||
            !SotnMath_Atan2(executable, dy, dx, &angle) ||
            !SotnMath_Trig(executable, angle + (facing_left ? -(Sint32)rotation : rotation), &trig))
            return SDL_SetError("Player effect rotation exceeds original SDK geometry bounds");
        next.x[i] = Word(x + Shift((Sint64)Shift(trig.cosine, 4) * distance + 524288, 20));
        next.y[i] = Word(y + Shift((Sint64)Shift(trig.sine, 4) * distance + 524288, 20));
    }
    *quad = next;
    return true;
}
static bool Geometry(SotnPlayerBlink *e, SotnPlayerBlinkShared *shared, SotnPlayerBlinkInput in,
                     const Sint16 profile[10], Uint16 parameter)
{
    const Sint16 origin_x = Word(in.x + (in.facing_left ? -in.image.pivot_x : in.image.pivot_x)),
                 origin_y = Word(in.y + in.image.pivot_y);
    const Sint32 width = in.image.width, height = in.image.height;
    Uint16 wave = e->wave;
    e->angle = (Uint16)(e->angle + e->speed);
    for (Uint16 i = 0; i < 8; ++i)
    {
        SotnPlayerEffectQuad *q = &e->quads[i];
        q->page = 0x18;
        Sint32 upper = 0, lower = 0;
        const bool morph = parameter == 0x20 || parameter == 0x21;
        if (morph)
        {
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(in.executable, wave, &trig))
                return false;
            upper = Shift(trig.sine, 7) * e->amplitude / 256;
            wave = (Uint16)(wave + 0x600);
            if (!SotnMath_Trig(in.executable, wave, &trig))
                return false;
            lower = Shift(trig.sine, 7) * e->amplitude / 256;
        }
        const Sint32 left = in.facing_left ? origin_x - width + 1 : origin_x,
                     right = in.facing_left ? origin_x + 1 : origin_x + width;
        q->x[0] = Word(left + upper);
        q->x[1] = Word(right + upper);
        q->x[2] = Word(left + lower);
        q->x[3] = Word(right + lower);
        q->y[0] = q->y[1] = Word(origin_y + height * i / 8);
        q->y[2] = q->y[3] = Word(origin_y + height * (i + 1U) / 8);
        q->u[0] = q->u[2] = (Uint8)(in.facing_left ? 3 + width : 4);
        q->u[1] = q->u[3] = (Uint8)(in.facing_left ? 3 : 4 + width);
        q->v[0] = q->v[1] = (Uint8)(1 + height * i / 8);
        q->v[2] = q->v[3] = (Uint8)(1 + height * (i + 1U) / 8);
        if (in.rotated &&
            !SotnPlayerEffect_Rotate(in.executable, in.x, in.y, in.rotation, in.facing_left, q))
            return false;
        if (!morph)
        {
            Uint8 *colors[3] = {q->red, q->green, q->blue};
            const Uint16 offsets[3] = {(Uint16)profile[0], (Uint16)profile[2], (Uint16)profile[1]};
            const Sint16 divisors[3] = {profile[4], profile[6], profile[5]};
            for (Uint16 channel = 0; channel < 3; ++channel)
                for (Uint16 edge = 0; edge < 2; ++edge)
                {
                    SotnMathTrig trig = {0};
                    if (!SotnMath_Trig(
                            in.executable,
                            (Sint32)(shared->angles[(i + offsets[channel] + edge) % 8] & 4095U),
                            &trig))
                        return false;
                    const Uint8 color =
                        (Uint8)(((trig.sine + 4096) / 64) * e->brightness / divisors[channel]);
                    colors[channel][edge * 2U] = colors[channel][edge * 2U + 1U] = color;
                }
            shared->angles[i] += e->speed;
        }
    }
    return true;
}
bool SotnPlayerBlink_Tick(SotnPlayerBlink *effect, SotnPlayerBlinkShared *shared,
                          SotnPlayerBlinkInput in, SotnPlayerBlinkEvents *out)
{
    if (!effect || !shared || !out || effect->removed || in.signal > 3 ||
        (effect->parameter != 0x20 && effect->parameter != 0x21 && effect->parameter != 0x1b))
        return SDL_SetError("Player blink requires its actual selected child and owner handshake");
    SotnPlayerBlink next = *effect;
    SotnPlayerBlinkShared phases = *shared;
    SotnPlayerBlinkEvents events = {0};
    const bool incompatible_form = (in.status & (2U | 0x1000000U)) != 0;
    if (incompatible_form || !in.image_present || (phases.bat_lock && next.parameter == 0x1b))
    {
        next.removed = true;
        if (incompatible_form || !in.image_present)
            phases.bat_lock = false;
        goto done;
    }
    if (next.brightness < 0 || next.brightness > 256 || next.amplitude < -8 ||
        next.amplitude > 128 ||
        (next.parameter == 0x20 && next.phase != 0 && next.phase != 8 && next.phase != 9) ||
        (next.parameter == 0x21 && next.phase != 0 && next.phase != 10 && next.phase != 11) ||
        (next.parameter == 0x1b && next.phase > 3))
        return SDL_SetError("Player blink has an invalid selected controller cursor");
    const Uint16 parameter = next.parameter;
    const size_t at = 0xd670 + (size_t)parameter * 20U;
    if (!in.dra.bytes || at > in.dra.size || in.dra.size - at < 20 || !in.image.width ||
        in.image.width > 252 || !in.image.height || in.image.height > 254 ||
        in.image.pivot_x < -512 || in.image.pivot_x > 512 || in.image.pivot_y < -512 ||
        in.image.pivot_y > 512 || in.priority > 507)
        return SDL_SetError(
            "Player blink requires bounded actual image geometry and private profile");
    Sint16 profile[10] = {0};
    for (Uint16 i = 0; i < 10; ++i)
        profile[i] = Read16(in.dra.bytes + at + i * 2U);
    if (profile[0] < 0 || profile[1] < 0 || profile[2] < 0 || profile[3] < 0 || profile[3] >= 768 ||
        profile[4] <= 0 || profile[5] <= 0 || profile[6] <= 0 || profile[7] <= 0 ||
        (profile[7] >= 0x7000 && profile[7] != 0x7008))
        return SDL_SetError("Selected player blink profile has invalid color or duration fields");
    switch (next.phase)
    {
    case 0:
        for (Uint16 i = 0; i < 8; ++i)
        {
            phases.angles[i] = i * 512U;
            SotnPlayerEffectQuad *q = &next.quads[i];
            q->palette = (Uint16)profile[3];
            q->priority = (Uint16)(in.priority + (parameter == 0x1b ? 2U : 4U));
            q->draw_mode = (Uint16)(profile[8] + 0x304);
            for (Uint16 v = 0; v < 4; ++v)
                q->red[v] = q->green[v] = q->blue[v] = 128;
        }
        next.speed = (Uint16)profile[9];
        next.brightness = next.amplitude = 0;
        next.phase = parameter == 0x20 ? 8 : parameter == 0x21 ? 10 : 1;
        break;
    case 1:
        next.brightness = (Sint16)(next.brightness + (profile[7] == 0x7008 ? 80 : 10));
        if (next.brightness > 256)
        {
            next.brightness = 256;
            next.hold = (Uint16)profile[7];
            ++next.phase;
        }
        break;
    case 2:
        if (profile[7] == 0x7008)
        {
            next.hold = 8;
            if (!(in.status & 0x400000U))
                ++next.phase;
        }
        if (!next.hold)
            return SDL_SetError("Player blink finishing hold timer is invalid");
        if (!--next.hold)
            ++next.phase;
        break;
    case 3:
        next.brightness = (Sint16)(next.brightness - 10);
        if (next.brightness < 0)
        {
            next.removed = true;
            goto done;
        }
        break;
    case 8:
        phases.bat_lock = true;
        next.wave = (Uint16)(next.wave + 256U);
        next.amplitude = (Sint16)(next.amplitude + 8);
        if (next.amplitude > 128)
        {
            next.amplitude = 128;
            events.signal = 2;
            ++next.phase;
        }
        break;
    case 9:
    case 11:
        next.amplitude = (Sint16)(next.amplitude - 8);
        next.wave = (Uint16)(next.wave + 256U);
        if (next.amplitude < 0)
        {
            events.signal = 3;
            next.parameter = 0x1b;
            next.phase = 1;
            for (Uint16 i = 0; i < 8; ++i)
                next.quads[i].palette = 0x15f;
            phases.bat_lock = false;
            goto done;
        }
        break;
    case 10:
        phases.bat_lock = true;
        next.amplitude = (Sint16)SDL_min(128, next.amplitude + 8);
        next.wave = (Uint16)(next.wave + 256U);
        if (in.signal == 1)
            ++next.phase;
        break;
    default:
        return SDL_SetError("Selected player blink has an unsupported controller phase");
    }
    if (!Geometry(&next, &phases, in, profile, parameter))
        return false;
    events.reset_afterimages = true;
done:
    *effect = next;
    *shared = phases;
    *out = events;
    return true;
}
