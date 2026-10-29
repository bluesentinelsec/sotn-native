// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/84B88.c:{func_80129864,EntitySummonSpirit}, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Shape/UV/angle tables stay private.
#include <SDL3/SDL.h>
#include <sotn/spirit_spell.h>
static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits <= 32767U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : (Sint32)((Sint64)value - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Uint32)value >> 16);
}
static Sint32 Shift(Sint64 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-value + scale - 1) / scale));
}
static Sint16 Read16(const Uint8 *p)
{
    return Word(p[0] | (Uint16)p[1] << 8);
}
static bool Trail(SotnSpiritTrail *p, SotnSpiritInput in)
{
    SotnPlayerEffectQuad *q = &p->quad;
    if (p->state < 0 || p->state > 2)
        return SDL_SetError("Spirit trail lost its original paired metadata phase");
    if (!p->state)
    {
        q->draw_mode |= 8;
        return true;
    }
    if (p->state == 1)
    {
        const size_t at = 0x10846U + in.frame * 2U;
        if (!in.dra.bytes || at > in.dra.size || in.dra.size - at < 2)
            return SDL_SetError("Spirit trail requires its bounded original UV neighborhood, "
                                "including birth frame zero");
        const Uint8 u = in.dra.bytes[at], v = in.dra.bytes[at + 1];
        q->u[0] = q->u[2] = u;
        q->u[1] = q->u[3] = (Uint8)(u + 31U);
        q->v[0] = q->v[1] = v;
        q->v[2] = q->v[3] = (Uint8)(v + 15U);
        for (Uint16 i = 0; i < 4; ++i)
            q->red[i] = q->green[i] = q->blue[i] = 128;
        q->draw_mode &= (Uint16)~8U;
        p->state = 2;
    }
    else
    {
        p->scale = Word((Sint32)p->scale - 16);
        const Uint8 color = q->green[3] > 4 ? (Uint8)(q->green[3] - 12U) : q->green[3];
        for (Uint16 i = 0; i < 4; ++i)
            q->red[i] = q->green[i] = q->blue[i] = color;
    }
    const Sint32 angles[4] = {p->angle - 0x200, p->angle - 0x734, p->angle + 0x200,
                              p->angle + 0x734};
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(in.executable, angles[i], &trig))
            return false;
        const Sint32 size = (i & 1U) ? 25 : 11;
        q->x[i] = Word(p->x +
                       Shift((Sint64)Shift((Sint64)Shift(trig.cosine, 4) * size, 8) * p->scale, 8));
        q->y[i] =
            Word(p->y - Shift((Sint64)Shift((Sint64)Shift(trig.sine, 4) * size, 8) * p->scale, 8));
    }
    return true;
}
bool SotnSpirit_Tick(SotnSpirit *spirit, SotnSpiritInput in, SotnSpiritEvents *out)
{
    if (!spirit || !out || spirit->removed || spirit->phase > 3 || (spirit->parameter & 255U) > 3 ||
        spirit->cursor < 0 || spirit->cursor >= 8 || in.frame > 8)
        return SDL_SetError("Spirit update requires its actual bounded child/context");
    SotnSpirit s = *spirit;
    SotnSpiritEvents events = {0};
    switch (s.phase)
    {
    case 0:
        if (!in.allocated)
        {
            s.removed = true;
            goto done;
        }
        if (!in.dra.bytes || in.dra.size < 0x10860U)
            return SDL_SetError("Spirit launch requires its private four-angle table");
        s.x = in.x;
        s.y = in.y;
        s.facing_left = !in.player_facing_left;
        s.angle = Read16(in.dra.bytes + 0x10858U + (s.parameter & 255U) * 2U);
        s.flags = 0xc930000U;
        for (Uint16 i = 0; i < 8; ++i)
            s.trails[i].quad = (SotnPlayerEffectQuad){
                .page = 0x1a, .palette = 0x19f, .priority = 450, .draw_mode = 0x17f};
        events.initialize_attack = true;
        events.spell = (s.parameter & 0x7f00U) ? 3 : 1;
        ++s.phase;
        break;
    case 1: {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(in.executable, s.wobble, &trig))
            return false;
        s.wobble = Word((Sint32)s.wobble + 0x80);
        s.angle = Word(s.angle + Shift(trig.cosine, 8));
        if (Pixel(s.y) < -32)
        {
            events.select_target = true;
            s.age = 0;
            s.angle = 0xc00;
            if ((s.parameter & 255U) >= 2)
                s.facing_left = !s.facing_left;
            ++s.phase;
        }
        break;
    }
    case 2: {
        Sint32 offset = s.age < 24 ? 16 : s.age < 40 ? 32 : s.age < 56 ? 64 : 128;
        if (in.target_bound && !in.target_alive)
        {
            ++s.phase;
            break;
        }
        const Sint32 tx = in.target_bound ? in.target_x
                          : s.facing_left ? 320
                                          : -64,
                     ty = in.target_bound ? in.target_y : 160 + ((s.parameter & 1U) ? 64 : 0);
        Sint16 bearing = 0;
        if (!SotnMath_Atan2Vector(in.executable, Pixel(s.y) - ty, tx - Pixel(s.x), &bearing))
            return false;
        const Sint32 target = (Uint16)bearing & 4095U;
        Sint32 angle = (Uint16)s.angle & 4095U;
        const Sint32 difference = SDL_abs(angle - target);
        offset = SDL_min(offset, difference);
        if (angle < target)
            angle += difference < 2048 ? offset : -offset;
        else
            angle += difference < 2048 ? -offset : offset;
        s.angle = (Sint16)((Uint32)angle & 4095U);
        s.age = Word((Sint32)s.age + 1);
        if (s.age > 96)
            ++s.phase;
        break;
    }
    case 3: {
        s.flags = 0x8910000U;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(in.executable, s.wobble, &trig))
            return false;
        s.wobble = Word((Sint32)s.wobble + 0x80);
        s.angle = Word(s.angle + Shift(trig.cosine, 8));
        break;
    }
    }
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(in.executable, s.angle, &trig))
        return false;
    s.vx = trig.cosine * 64;
    s.vy = -trig.sine * 64;
    s.x = Fixed((Uint32)s.x + (Uint32)s.vx);
    s.y = Fixed((Uint32)s.y + (Uint32)s.vy);
    s.rotation = s.facing_left ? s.angle : Word(0x800 - s.angle);
    for (Uint16 i = 0; i < 8; ++i)
    {
        SotnSpiritTrail *p = &s.trails[i];
        if (s.cursor == (Sint16)i)
        {
            p->state = 1;
            p->scale = 256;
            p->x = Pixel(s.x);
            p->y = Pixel(s.y);
            p->angle = s.angle;
        }
        if (!Trail(p, in))
            return false;
    }
    s.cursor = (Sint16)((s.cursor + 1) % 8);
done:
    *spirit = s;
    *out = events;
    return true;
}
bool SotnSpirit_Translate(SotnSpirit *s, Sint32 dx, Sint32 dy)
{
    if (!s || dx < SDL_MIN_SINT16 || dx > SDL_MAX_SINT16 || dy < SDL_MIN_SINT16 ||
        dy > SDL_MAX_SINT16)
        return SDL_SetError("Spirit rebase requires its actual owner and signed-word shifts");
    if (s->phase && !s->removed)
    {
        s->x = Fixed((Uint32)s->x + (Uint32)dx * 65536U);
        s->y = Fixed((Uint32)s->y + (Uint32)dy * 65536U);
        for (Uint16 i = 0; i < 8; ++i)
        {
            SotnSpiritTrail *p = &s->trails[i];
            p->x = Word(p->x + dx);
            p->y = Word(p->y + dy);
            for (Uint16 v = 0; v < 4; ++v)
            {
                p->quad.x[v] = Word(p->quad.x[v] + dx);
                p->quad.y[v] = Word(p->quad.y[v] + dy);
            }
        }
    }
    return true;
}
bool SotnSpiritPortal_Tick(SotnSpiritPortal *portal, SotnSpiritPortalInput in,
                           SotnSpiritPortalEvents *out)
{
    if (!portal || !out || portal->removed || portal->phase > 3 || (portal->parameter & 255U) ||
        (portal->phase == 1 && (portal->delay < 1 || portal->delay > 10)) ||
        (portal->phase >= 2 && (portal->timer < 0 || portal->timer > 11)))
        return SDL_SetError("Spirit portal requires its actual bounded root and lifecycle");
    SotnSpiritPortal s = *portal;
    SotnSpiritPortalEvents events = {0};
    switch (s.phase)
    {
    case 0:
        s.x = in.x;
        s.y = in.y;
        s.flags = 0xc030000U;
        s.delay = 10;
        ++s.phase;
        events.freeze = 3;
        events.flash = 13;
        goto done;
    case 1:
        if (--s.delay)
            goto done;
        if (!in.allocated)
        {
            s.removed = true;
            goto done;
        }
        if (!in.dra.bytes || in.dra.size < 0x10860U + 36U)
            return SDL_SetError(
                "Spirit portal requires its private original nine-point capture shape");
        s.x = Fixed(((Uint32)s.x & 65535U) | (Uint32)128 << 16);
        const Sint16 py = Word(in.player_y - 48 > 64 ? in.player_y - 48 : in.player_y + 64);
        s.y = Fixed(((Uint32)s.y & 65535U) | (Uint32)(Uint16)py << 16);
        events.capture = true;
        events.capture_x = 96;
        events.capture_y = Word(py - 32);
        for (Uint16 i = 0; i < 8; ++i)
        {
            SotnPlayerEffectQuad *q = &s.quads[i];
            q->page = 0x118;
            q->priority = (Uint16)(449U + (i >= 4 ? 3U : 0U));
            q->draw_mode = 0x102;
            const Sint16 x0 = Read16(in.dra.bytes + 0x10860U + i * 4U),
                         y0 = Read16(in.dra.bytes + 0x10862U + i * 4U),
                         x1 = Read16(in.dra.bytes + 0x10864U + i * 4U),
                         y1 = Read16(in.dra.bytes + 0x10866U + i * 4U);
            q->x[1] = Word(128 + x1);
            q->y[1] = Word(py + y1);
            q->x[3] = Word(128 + x0);
            q->y[3] = Word(py + y0);
            q->u[1] = (Uint8)(Uint32)(x1 + 32);
            q->v[1] = (Uint8)(Uint32)(y1 + 224);
            q->u[3] = (Uint8)(Uint32)(x0 + 32);
            q->v[3] = (Uint8)(Uint32)(y0 + 224);
            q->u[0] = q->u[2] = 32;
            q->v[0] = q->v[2] = 224;
        }
        s.timer = 0;
        s.flags = 0xc830000U;
        ++s.phase;
        break;
    case 2:
        ++s.timer;
        if (s.timer > 10)
        {
            events.factories[0] = s.parameter ? 118 : 116;
            events.factories[1] = 0x67002c;
            events.sound = 0x67d;
            ++s.phase;
        }
        break;
    case 3:
        if (--s.timer < 0)
        {
            events.release_freeze = true;
            s.removed = true;
            goto done;
        }
        break;
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        SotnPlayerEffectQuad *q = &s.quads[i];
        SotnMathTrig a = {0}, b = {0};
        if (!SotnMath_Trig(in.executable, i * 512, &a) ||
            !SotnMath_Trig(in.executable, (i + 1) * 512, &b))
            return false;
        q->x[2] = Word(Pixel(s.x) + Shift((Sint64)Shift(a.cosine, 4) * s.timer, 8));
        q->y[2] = Word(Pixel(s.y) - Shift((Sint64)Shift(a.sine, 4) * s.timer, 8));
        q->x[0] = Word(Pixel(s.x) + Shift((Sint64)Shift(b.cosine, 4) * s.timer, 8));
        q->y[0] = Word(Pixel(s.y) - Shift((Sint64)Shift(b.sine, 4) * s.timer, 8));
    }
done:
    *portal = s;
    *out = events;
    return true;
}
