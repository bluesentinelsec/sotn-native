// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:EntityMist/func_80121F58, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/artwork remain private inputs.
#include <SDL3/SDL_error.h>
#include <sotn/mist_effect.h>

static Sint32 Shift(Sint64 value, Uint8 count)
{
    const Sint64 divisor = (Sint64)1 << count;
    return (Sint32)(value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor));
}
static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Fixed(Sint64 value)
{
    const Uint32 bits = (Uint32)(Uint64)value;
    return (Sint32)(bits <= SDL_MAX_SINT32 ? (Sint64)bits : (Sint64)bits - 4294967296LL);
}
static Sint32 Whole(Sint32 value)
{
    return Shift(value, 16);
}
static void SetWhole(Sint32 *value, Sint32 pixel)
{
    *value = Fixed((Sint64)Word(pixel) * 65536 + ((Uint32)*value & 65535U));
}
static Sint16 Read16(const Uint8 *p)
{
    return Word(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool Distance(SotnDataView executable, Sint32 x, Sint32 y, Sint16 *out)
{
    const Sint64 square = ((Sint64)x * x + (Sint64)y * y) * 4096;
    if (square > SDL_MAX_SINT32)
        return SDL_SetError("Mist effect point exceeds the original square-root input range");
    Uint32 root = 0;
    if (!SotnMath_Sqrt12(executable, (Uint32)square, &root))
        return false;
    *out = Word(root >> 12);
    return true;
}
static bool Capture(SotnMistEffect *s, SotnMistEffectInput in, bool returning)
{
    SDL_memcpy(s->capture, in.capture, sizeof(s->capture));
    const size_t permutation = in.facing_left && !in.alternate_sheet ? 0xe230 : 0xe250;
    Sint16 center_x = 0, center_y = 0;
    for (Uint16 i = 0; i < 16; ++i)
    {
        const Uint32 index = Read32(in.dra.bytes + permutation + (i >> 1) * 4U);
        if (index >= 8)
            return SDL_SetError("Mist snapshot permutation exceeds its eight captured wedges");
        const SotnMistWedge *w = &s->capture[index];
        const bool left = in.facing_left && !in.alternate_sheet;
        const Sint16 x = (i & 1) ? Word(((Sint32)w->x[0] + w->x[1]) / 2) : w->x[left ? 0 : 1],
                     y = (i & 1) ? Word(((Sint32)w->y[0] + w->y[1]) / 2) : w->y[left ? 0 : 1];
        SotnMistPoint *p = &s->points[i];
        center_x = w->x[2];
        center_y = w->y[2];
        if (returning)
        {
            p->target_x = Word((Sint32)x - in.x);
            p->target_y = Word((Sint32)y - in.y);
        }
        else
        {
            // Original high-word assignments retain global ring-point fractions on reuse.
            SetWhole(&p->x, x);
            SetWhole(&p->y, y);
            p->angle = (Sint16)(i * 256);
            p->phase = (Sint16)(i * 512);
            if (!Distance(in.executable, Word((Sint32)x - center_x), Word((Sint32)y - center_y),
                          &p->size))
                return false;
        }
    }
    if (returning)
    {
        s->target_x = Word((Sint32)center_x - in.x);
        s->target_y = Word((Sint32)center_y - in.y);
        SetWhole(&s->x, in.x);
        SetWhole(&s->y, in.y);
        s->opacity = 255;
        s->phase = 4;
    }
    else
    {
        s->offset_x = Word((Sint32)center_x - in.x);
        s->offset_y = Word((Sint32)center_y - in.y);
        SetWhole(&s->x, center_x);
        SetWhole(&s->y, center_y);
        for (Uint16 i = 0; i < 4; ++i)
        {
            s->color_phase[i] = Word(in.random[i]);
            s->color_base[i] = 239;
        }
        s->opacity = 0;
        s->phase = 1;
    }
    s->timer = 0;
    for (Uint16 j = 0; j < 16; ++j)
    {
        const Uint32 index = Read32(in.dra.bytes + permutation + (j >> 1) * 4U);
        const SotnMistWedge *w = &s->capture[index];
        const Uint16 first = in.facing_left && !in.alternate_sheet ? 0 : 1, last = 1U - first;
        const Uint8 mu = (Uint8)(((Uint32)w->u[0] + w->u[1]) / 2),
                    mv = (Uint8)(((Uint32)w->v[0] + w->v[1]) / 2);
        SotnMistQuad *q = &s->quads[j];
        *q = (SotnMistQuad){.page = in.alternate_sheet ? 0x118 : 0x18,
                            .palette = 0x10f,
                            .priority = (Uint16)(in.priority + 2U)};
        q->u[0] = j & 1 ? mu : w->u[first];
        q->v[0] = j & 1 ? mv : w->v[first];
        q->u[1] = j & 1 ? w->u[last] : mu;
        q->v[1] = j & 1 ? w->v[last] : mv;
        q->u[2] = q->u[3] = w->u[2];
        q->v[2] = q->v[3] = w->v[2];
        SotnMathTrig a = {0}, b = {0};
        if (!SotnMath_Trig(in.executable, s->points[j].angle, &a) ||
            !SotnMath_Trig(in.executable, s->points[(j + 1U) % 16].angle, &b))
            return false;
        for (Uint16 i = 0; i < 4; ++i)
        {
            q = &s->quads[16U + j * 4U + i];
            *q = (SotnMistQuad){
                .page = 0x18, .palette = 0x10f, .priority = (Uint16)(in.priority + 4U)};
            for (Uint16 vertex = 0; vertex < 4; ++vertex)
            {
                const SotnMathTrig t = vertex & 1 ? b : a;
                const Sint32 ring = i + (vertex < 2 ? 1 : 0);
                q->u[vertex] = (Uint8)(96 + Shift((Sint64)Shift(t.cosine, 4) * 8 * ring, 8));
                q->v[vertex] = (Uint8)(160 - Shift((Sint64)Shift(t.sine, 4) * 8 * ring, 8));
                if (i == 3 && vertex < 2)
                {
                    if (q->u[vertex] < 4)
                        q->u[vertex] = 255;
                    if (q->v[vertex] < 4)
                        q->v[vertex] = 255;
                }
            }
        }
    }
    return true;
}
static bool MovePoint(SotnMistPoint *p, SotnDataView executable, Sint32 x, Sint32 y, bool still)
{
    if (still && !x && !y)
        return true;
    Sint16 angle = 0;
    SotnMathTrig trig = {0};
    if (!SotnMath_Atan2(executable, -y, x, &angle) || !SotnMath_Trig(executable, angle, &trig))
        return false;
    p->x = Fixed((Sint64)p->x + (Sint32)trig.cosine * 32);
    p->y = Fixed((Sint64)p->y - (Sint32)trig.sine * 32);
    return true;
}
static bool Expand(SotnMistEffect *s, SotnMistEffectInput in, Sint16 *center_x, Sint16 *center_y)
{
    *center_x = Word((Sint32)in.x + s->offset_x);
    *center_y = Word((Sint32)in.y + s->offset_y);
    const Sint16 dx = Word((Sint32)*center_x - Whole(s->x)),
                 dy = Word((Sint32)*center_y - Whole(s->y));
    Sint16 angle = 0, magnitude = 0;
    if (!SotnMath_Atan2(in.executable, -dy, dx, &angle) ||
        !Distance(in.executable, dx, dy, &magnitude))
        return false;
    if (magnitude > 10)
    {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        const Sint32 scale = magnitude > 60 ? 32 : magnitude > 35 ? 16 : 8;
        s->x = Fixed((Sint64)s->x + (Sint32)t.cosine * scale);
        s->y = Fixed((Sint64)s->y - (Sint32)t.sine * scale);
    }
    const Sint16 mx = Word((Whole(s->x) + *center_x) / 2), my = Word((Whole(s->y) + *center_y) / 2);
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnMistPoint *p = &s->points[i];
        if (p->size < 40 && (in.game_timer & 1))
            p->size = Word((Sint32)p->size + 1);
        const Uint16 index = (Uint16)(((Uint32)((Sint32)angle - p->angle) & 4095U) >> 7);
        const Sint32 weight = Read16(in.dra.bytes + 0xe1b0 + index * 2U),
                     sx = ((p->size + SDL_abs((Sint32)dx) / 2) * weight) / 80,
                     sy = ((p->size + SDL_abs((Sint32)dy) / 2) * weight) / 80;
        SotnMathTrig t = {0}, wave = {0};
        p->phase = Word((Sint32)p->phase + 8);
        if (!SotnMath_Trig(in.executable, p->angle, &t) ||
            !SotnMath_Trig(in.executable, p->phase, &wave))
            return false;
        const Sint32 ripple = s->timer < 16 ? 2 : 6;
        const Sint16 x = Word(mx + Shift((Sint64)Shift(t.cosine, 4) * sx, 8)),
                     y = Word(my - Shift((Sint64)Shift(t.sine, 4) * sy, 8)),
                     tx = Word(x + Shift((Sint64)Shift(wave.cosine, 4) * ripple, 8)),
                     ty = Word(y - Shift((Sint64)Shift(wave.sine, 4) * ripple, 8));
        if (!MovePoint(p, in.executable, Word((Sint32)tx - Whole(p->x)),
                       Word((Sint32)ty - Whole(p->y)), false))
            return false;
        if (Read16(in.dra.bytes + 0xe1f0 + index * 2U) == -1)
        {
            p->x = Fixed((Sint64)p->x + Shift(in.vx, 1));
            p->y = Fixed((Sint64)p->y + Shift(in.vy, 1));
        }
    }
    s->opacity = (Uint8)SDL_min(255, (Sint32)s->opacity + 8);
    return true;
}
static bool Contract(SotnMistEffect *s, SotnMistEffectInput in, Sint16 *cx, Sint16 *cy)
{
    s->offset_x = Word(s->offset_x + (s->offset_x < s->target_x) - (s->offset_x > s->target_x));
    s->offset_y = Word(s->offset_y + (s->offset_y < s->target_y) - (s->offset_y > s->target_y));
    *cx = Word((Sint32)in.x + s->offset_x);
    *cy = Word((Sint32)in.y + s->offset_y);
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnMistPoint *p = &s->points[i];
        const Sint16 dx = Word((Sint32)p->target_x + in.x - Whole(p->x)),
                     dy = Word((Sint32)p->target_y + in.y - Whole(p->y));
        if (!MovePoint(p, in.executable, dx, dy, true))
            return false;
        p->x = Fixed((Sint64)p->x + in.vx);
        p->y = Fixed((Sint64)p->y + in.vy);
    }
    if (s->timer > 24)
    {
        s->phase = 5;
        s->timer = 16;
    }
    s->opacity = (Uint8)SDL_max(0, (Sint32)s->opacity - 16);
    return true;
}
static void Mesh(SotnMistEffect *s, Sint16 cx, Sint16 cy, const Sint32 colors[4])
{
    for (Uint16 j = 0; j < 16; ++j)
    {
        const Sint32 x[2] = {Whole(s->points[j].x), Whole(s->points[(j + 1U) % 16].x)},
                     y[2] = {Whole(s->points[j].y), Whole(s->points[(j + 1U) % 16].y)};
        SotnMistQuad *q = &s->quads[j];
        for (Uint16 v = 0; v < 4; ++v)
        {
            q->x[v] = v < 2 ? (Sint16)x[v] : cx;
            q->y[v] = v < 2 ? (Sint16)y[v] : cy;
            q->grey[v] = v < 2 ? 0 : (Uint8)(255U - s->opacity);
        }
        q->hidden = 255U - s->opacity < 16;
        for (Uint16 i = 0; i < 4; ++i)
        {
            q = &s->quads[16U + j * 4U + i];
            for (Uint16 v = 0; v < 4; ++v)
            {
                const Sint32 ring = i + (v < 2 ? 1 : 0);
                q->x[v] = Word(cx + ring * (x[v & 1] - cx) / 4);
                q->y[v] = Word(cy + ring * (y[v & 1] - cy) / 4);
                q->grey[v] =
                    i == 3 && v < 2 ? 0 : (Uint8)((s->opacity * colors[i + (v < 2 ? 1 : 0)]) / 256);
            }
        }
    }
}
bool SotnMistEffect_Tick(SotnMistEffect *effect, SotnMistEffectInput in, SotnMistEffectEvents *out)
{
    if (!effect || !out || effect->phase > 5 || effect->removed || !in.dra.bytes ||
        in.dra.size < 0xe270 || ((effect->phase == 0 || effect->phase == 3) && !in.capture))
        return SDL_SetError(
            "Mist visual tick requires a live cursor, private tables and real capture");
    for (Uint16 i = 0; i < 32; ++i)
        if (Read16(in.dra.bytes + 0xe1b0 + i * 2U) < 0 ||
            Read16(in.dra.bytes + 0xe1b0 + i * 2U) > 256 ||
            SDL_abs((Sint32)Read16(in.dra.bytes + 0xe1f0 + i * 2U)) > 1)
            return SDL_SetError("Mist visual weights/flow exceed their bounded private tables");
    SotnMistEffect next = *effect;
    SotnMistEffectEvents events = {0};
    Sint32 colors[4] = {0};
    for (Uint16 i = 0; i < 4; ++i)
    {
        if (next.color_base[i] < 0 || next.color_base[i] > 255)
            return SDL_SetError("Mist visual color base exceeds its original byte range");
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, next.color_phase[i], &t))
            return false;
        colors[i] = next.color_base[i] + Shift(t.sine, 8);
        next.color_phase[i] = Word((Sint32)next.color_phase[i] + (i & 1 ? -32 : 32));
    }
    next.timer = Word((Sint32)next.timer + 1);
    Sint16 cx = 0, cy = 0;
    switch (next.phase)
    {
    case 0:
    case 3: {
        const bool returning = next.phase == 3;
        if (!Capture(&next, in, returning))
            return false;
        events.random_count = returning ? 0 : 4;
        cx = Word((Sint32)in.x + next.offset_x);
        cy = Word((Sint32)in.y + next.offset_y);
        break;
    }
    case 1:
        for (Uint16 i = 0; i < 4; ++i)
        {
            const Sint32 minimum = i == 0 ? 208 : i == 1 ? 160 : i == 2 ? 112 : 48;
            next.color_base[i] = Word(SDL_max(minimum, next.color_base[i] - (Sint32)i - 1));
        }
        if (next.timer == 79)
        {
            next.phase = 2;
            events.cancel_palette = true;
        }
        /* Original phase 1 falls through into the expanding cloud. */
        SDL_FALLTHROUGH;
    case 2:
        if (!Expand(&next, in, &cx, &cy))
            return false;
        break;
    case 4:
        if (next.timer == 12)
            events.cancel_palette = events.restore_palette = true;
        if (!Contract(&next, in, &cx, &cy))
            return false;
        break;
    case 5:
        // The original increments this signed halfword; it does not count down from 16.
        // Readiness precedes destruction, which occurs only when the counter wraps negative.
        next.removed = next.timer <= 0;
        *effect = next;
        *out = events;
        return true;
    default:
        return SDL_SetError("Mist visual cursor exceeds its original phases");
    }
    Mesh(&next, cx, cy, colors);
    *effect = next;
    *out = events;
    return true;
}

bool SotnMistEffect_Translate(SotnMistEffect *effect, Sint32 x, Sint32 y, bool live)
{
    if (!effect || x < SDL_MIN_SINT16 || x > SDL_MAX_SINT16 || y < SDL_MIN_SINT16 ||
        y > SDL_MAX_SINT16 || (live && (effect->removed || effect->phase > 5)))
        return SDL_SetError("Mist translation requires bounded camera deltas and its actual owner");
    for (Uint16 i = 0; i < 16; ++i)
    {
        effect->points[i].x = Fixed((Sint64)effect->points[i].x + (Sint64)x * 65536);
        effect->points[i].y = Fixed((Sint64)effect->points[i].y + (Sint64)y * 65536);
    }
    if (live)
    {
        effect->x = Fixed((Sint64)effect->x + (Sint64)x * 65536);
        effect->y = Fixed((Sint64)effect->y + (Sint64)y * 65536);
        for (Uint16 i = 0; i < 80; ++i)
            for (Uint16 vertex = 0; vertex < 4; ++vertex)
            {
                effect->quads[i].x[vertex] = Word((Sint32)effect->quads[i].x[vertex] + x);
                effect->quads[i].y[vertex] = Word((Sint32)effect->quads[i].y[vertex] + y);
            }
    }
    return true;
}
