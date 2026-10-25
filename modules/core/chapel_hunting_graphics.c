// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_hunting_girl.h at sotn-decomp 7bc54d8157b6. Tables stay private.
#include "actors_internal.h"
bool ActorHuntingTip(SotnActors *s, const Actor *a, Sint32 *x, Sint32 *y)
{
    SotnMathTrig trig = {0};
    const Sint32 angle = a->state.facing_left ? -a->state.rotation : a->state.rotation;
    if (!SotnMath_Trig(s->executable, angle, &trig))
        return false;
    *x = Wrap16((Sint64)Pixel(a->state.x) + ActorShift((Sint64)trig.sine * -48, 12));
    *y = Wrap16((Sint64)Pixel(a->state.y) + ActorShift((Sint64)trig.cosine * 48, 12));
    return true;
}
bool ActorHuntingSpirit(SotnActors *s, Actor *a, Uint16 profile, Uint16 phase, bool *done)
{
    SotnDataView data = {0};
    if (profile >= 5 || phase >= 3 || !Slice(s->nz0, 0x2144U + profile * 12U, 12, &data))
        return SDL_SetError("Chapel Hunting Girl spirit profile or phase is invalid");
    const Sint32 u = Signed16(data.bytes), v = Signed16(data.bytes + 2),
                 width = Signed16(data.bytes + 4), height = Signed16(data.bytes + 6),
                 dx = Signed16(data.bytes + 8), dy = Signed16(data.bytes + 10);
    if (u < 0 || v < 0 || width <= 0 || height <= 0 || u + width > 255 || v + height > 255 ||
        dx < -512 || dx > 512 || dy < -512 || dy > 512)
        return SDL_SetError("Chapel Hunting Girl spirit rectangle exceeds its private page");
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    *done = false;
    const Sint32 left = a->state.facing_left ? -dx : dx;
    const Sint32 right = left + (a->state.facing_left ? -width : width);
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = Pixel(a->state.y) * 65536;
    p->state.offset_x[0] = p->state.offset_x[2] = (Sint16)left;
    p->state.offset_x[1] = p->state.offset_x[3] = (Sint16)right;
    p->state.offset_y[0] = p->state.offset_y[1] = (Sint16)dy;
    p->state.offset_y[2] = p->state.offset_y[3] = (Sint16)(dy + height);
    if (phase == 0)
    {
        p->state.u[0] = p->state.u[2] = (Uint8)u;
        p->state.u[1] = p->state.u[3] = (Uint8)(u + width);
        p->state.v[0] = p->state.v[1] = (Uint8)v;
        p->state.v[2] = p->state.v[3] = (Uint8)(v + height);
        p->state.palette = 0x231;
        p->state.priority = (Uint16)(a->state.priority + 1U);
        p->state.active = false;
        a->hunt.brightness = 0;
        return true;
    }
    p->state.page = 53; // Original page 21 with the selected additive draw-mode bit.
    p->state.active = p->state.blend = p->state.modulate = true;
    if (phase == 1)
    {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, a->hunt.sine_phase, &trig))
            return false;
        const Uint8 value = (Uint8)((a->hunt.brightness * SDL_abs(trig.sine)) / 4096);
        p->state.red = p->state.green = p->state.blue = value;
        if (a->hunt.brightness < 128)
            a->hunt.brightness = (Uint8)(a->hunt.brightness + 16);
        a->hunt.sine_phase = (Sint16)Wrap16((Sint64)a->hunt.sine_phase + 64);
        return true;
    }
    // PrimDecreaseBrightness retains its zero-color frame before reporting completion.
    const bool alive = p->state.red >= 7 || p->state.green >= 7 || p->state.blue >= 7;
    p->state.red = p->state.red < 7 ? 0 : (Uint8)(p->state.red - 7);
    p->state.green = p->state.green < 7 ? 0 : (Uint8)(p->state.green - 7);
    p->state.blue = p->state.blue < 7 ? 0 : (Uint8)(p->state.blue - 7);
    *done = !alive;
    p->state.active = alive;
    return true;
}
static Uint16 PatternCount(size_t pattern)
{
    switch (pattern)
    {
    case 0x2180:
    case 0x2198:
    case 0x21b0:
        return 3;
    case 0x21c8:
        return 2;
    case 0x21d8:
        return 8;
    case 0x2218:
    case 0x2250:
        return 7;
    default:
        return 0;
    }
}
bool ActorHuntingDrawAttack(SotnActors *s, Actor *a, size_t pattern, bool *done)
{
    SotnDataView data = {0};
    const Uint16 count = PatternCount(pattern);
    if (!count || a->hunt.attack_step >= count ||
        !Slice(s->nz0, pattern + a->hunt.attack_step * 8U, 8, &data))
        return SDL_SetError("Chapel Hunting Girl attack cursor exceeds its private pattern");
    *done = false;
    if (!a->hunt.frames)
    {
        const Sint32 dx = Signed16(data.bytes), dy = Signed16(data.bytes + 2);
        if (dx == 0x7fff)
        {
            *done = true;
            return true;
        }
        const Sint32 frames = Signed16(data.bytes + 6), angle = Signed16(data.bytes + 4);
        if (dx < -128 || dx > 128 || dy < -128 || dy > 128 || frames <= 0 || frames > 1024 ||
            angle < 0 || angle > 4095)
            return SDL_SetError("Chapel Hunting Girl motion has an invalid duration or delta");
        a->hunt.frames = (Uint16)frames;
        a->vx = (a->state.facing_left ? dx : -dx) * 65536 / frames;
        a->vy = dy * 65536 / frames;
        Sint32 difference = Wrap16((Sint64)angle - a->state.rotation);
        if (difference > 2048)
            difference -= 4096;
        if (difference < -2048)
            difference += 4096;
        a->hunt.turn = (Sint16)(difference / frames);
        if (a->hunt.attacking)
        {
            Sint32 x = 0, y = 0;
            if (!a->hunt.trail || a->hunt.trail >= 32 || !ActorHuntingTip(s, a, &x, &y))
                return SDL_SetError("Chapel Hunting Girl lost its reserved attack trail");
            ActorPrimitive *p = ActorPrimitiveGet(s, a, a->hunt.trail);
            if (!p || !ActorPositionalSound(s, a, 0x640))
                return false;
            p->phase = 8;
            p->state.active = true;
            p->state.screen_space = true;
            p->state.offset_x[0] = p->state.offset_x[1] = (Sint16)Wrap16((Sint64)x - s->camera_x);
            p->state.offset_y[0] = p->state.offset_y[1] = (Sint16)Wrap16((Sint64)y - s->camera_y);
            p->state.red = p->state.line_red = 192;
            p->state.green = p->state.line_green = 64;
            p->state.blue = p->state.line_blue = 128;
        }
        else
            a->hunt.trail = 0;
    }
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Hunting Girl attack exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + a->hunt.turn);
    if (a->hunt.trail)
    {
        Sint32 tip_x = 0, tip_y = 0;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, a->hunt.trail);
        if (!p || !ActorHuntingTip(s, a, &tip_x, &tip_y))
            return false;
        p->state.offset_x[0] = (Sint16)Wrap16((Sint64)tip_x - s->camera_x);
        p->state.offset_y[0] = (Sint16)Wrap16((Sint64)tip_y - s->camera_y);
        p->state.active = true;
    }
    if (!--a->hunt.frames)
    {
        ++a->hunt.attack_step;
        if (a->hunt.trail)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, a->hunt.trail);
            if (!p)
                return false;
            p->phase = 2;
            ++a->hunt.trail;
        }
        // Native portability: omit the reference's write through an absent trail,
        // but keep its subsequent cursor/END processing rather than returning early.
        SotnDataView next = {0};
        if (a->hunt.attack_step >= count ||
            !Slice(s->nz0, pattern + a->hunt.attack_step * 8U, 2, &next))
            return SDL_SetError("Chapel Hunting Girl pattern has no bounded terminator");
        if (Read16(next.bytes) == 0xfff)
        {
            a->hunt.attack_step = 0;
            *done = true;
        }
    }
    return true;
}
bool ActorHuntingTrails(SotnActors *s, Actor *a)
{
    SotnDataView fade = {0};
    if (!Slice(s->nz0, 0x213c, 3, &fade) || !fade.bytes[0] || !fade.bytes[1] || !fade.bytes[2])
        return SDL_SetError("Chapel Hunting Girl trail requires nonzero private fade rates");
    for (Uint16 i = 1; i < 32; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (p->phase != 2)
            continue;
        const Uint8 r = p->state.line_red, g = p->state.line_green, b = p->state.line_blue;
        p->state.red = r;
        p->state.green = g;
        p->state.blue = b;
        p->state.line_red = r < fade.bytes[0] ? 0 : (Uint8)(r - fade.bytes[0]);
        p->state.line_green = g < fade.bytes[1] ? 0 : (Uint8)(g - fade.bytes[1]);
        p->state.line_blue = b < fade.bytes[2] ? 0 : (Uint8)(b - fade.bytes[2]);
        if (!r && !g && !b)
        {
            p->state.active = false;
            p->phase = 0;
        }
    }
    return true;
}
