// SPDX-License-Identifier: AGPL-3.0-or-later
// st/entity_splash_water.h and water_effects.h:EntitySurfacingWater/EntitySideWaterSplash,
// pinned sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables remain private.
#include <SDL3/SDL_error.h>
#include <sotn/combat.h>
#include <sotn/water_effect.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Sint16 Read16(const Uint8 *p)
{
    return Word(p[0] | (Sint32)p[1] << 8);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Read32(const Uint8 *p)
{
    return Fixed((Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Sint32 High(Sint16 word)
{
    return Fixed((Uint32)(Uint16)word << 16);
}
static void Grey(SotnPlayerEffectQuad *q, Uint16 v, Uint8 c)
{
    q->red[v] = q->green[v] = q->blue[v] = c;
}
static void Sound(SotnWaterEffectEvents *e, Sint16 x, Uint8 index, bool positional)
{
    const Sint32 dx = x - 120;
    const Sint32 pan = dx >= 0 ? dx / 16 : -((-dx + 15) / 16);
    e->sound = true;
    e->positional = positional;
    e->sound_index = index;
    e->volume = positional ? 127 : 0;
    e->pan = positional ? (Sint8)SDL_clamp(pan, -8, 8) : 0;
}
bool SotnWaterEffect_Rows(SotnWaterChildKind kind, Uint16 p, Uint16 *out)
{
    if (!out ||
        (kind != SOTN_WATER_SPLASH && kind != SOTN_WATER_SURFACING &&
         kind != SOTN_WATER_SIDE_SPLASH) ||
        (kind == SOTN_WATER_SPLASH && (p & 15U) >= 4) ||
        (kind == SOTN_WATER_SURFACING && ((p & 255U) >= 6 || ((p >> 8) & 127U) >= 8)) ||
        (kind == SOTN_WATER_SIDE_SPLASH && ((p & 15U) >= 8 || ((p >> 4) & 15U) >= 8)))
        return SDL_SetError("Water effect requires its actual role and bounded private indices");
    *out = kind == SOTN_WATER_SIDE_SPLASH        ? 1
           : kind == SOTN_WATER_SURFACING        ? 2
           : (p / 2048U && ((p >> 5) & 7U) != 7) ? 4
                                                 : 2;
    return true;
}
static bool InitSplash(SotnWaterEffect *s, SotnWaterEffectInput in, SotnWaterEffectEvents *e)
{
    const Uint16 width = s->parameter / 2048U, slope = (s->parameter >> 8) & 7U,
                 other = (s->parameter >> 5) & 7U;
    const Sint16 aspect = Read16(in.tables.bytes + slope * 2U),
                 adjacent = Read16(in.tables.bytes + other * 2U);
    if (slope && !aspect)
        return SDL_SetError("Original water splash slope divides by zero in the supplied data");
    const Sint16 x = Pixel(s->x), y = Pixel(s->y);
    for (Uint16 i = 0; i < s->count; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        *q = (SotnPlayerEffectQuad){.page = 0x1a,
                                    .palette = (i & 1U) ? 0x15f : 0x162,
                                    .priority = (Uint16)(in.priority + ((i & 1U) ? 4U : 2U)),
                                    .draw_mode = (i & 1U) ? 0x37 : 0x77};
        if (i & 1U)
        {
            const SotnPlayerEffectQuad *p = &s->quads[i - 1U];
            SDL_memcpy(q->x, p->x, sizeof(q->x));
            SDL_memcpy(q->u, p->u, sizeof(q->u));
            SDL_memcpy(q->v, p->v, sizeof(q->v));
            q->y[2] = p->y[2];
            q->y[3] = p->y[3];
        }
        else
        {
            q->u[1] = q->u[3] = 32;
            q->v[0] = q->v[1] = 96;
            q->v[2] = q->v[3] = 124;
            q->x[0] = q->x[2] = Word(x - 14);
            q->x[1] = q->x[3] = Word(x + 14);
            q->y[2] = q->y[3] = y;
            if (i)
            {
                const SotnPlayerEffectQuad *p = &s->quads[i - 1U];
                if (width > 14)
                {
                    q->u[0] = q->u[2] = p->u[1];
                    q->x[0] = q->x[2] = p->x[1];
                    q->y[2] = p->y[3];
                    q->y[3] = adjacent ? Word(q->y[2] - (q->x[1] - q->x[0]) / adjacent) : q->y[2];
                }
                else
                {
                    q->u[1] = q->u[3] = p->u[0];
                    q->x[1] = q->x[3] = p->x[0];
                    q->y[3] = p->y[2];
                    q->y[2] = adjacent ? Word(q->y[3] + (q->x[1] - q->x[0]) / adjacent) : q->y[3];
                }
            }
            else
            {
                if (width)
                {
                    if (width > 14)
                    {
                        q->u[1] = q->u[3] = (Uint8)(width * 32U / 28U);
                        q->x[1] = q->x[3] = Word(q->x[0] + width);
                    }
                    else
                    {
                        q->u[0] = q->u[2] = (Uint8)(width * 32U / 28U);
                        q->x[0] = q->x[2] = Word(q->x[0] + width);
                    }
                }
                if (slope)
                {
                    if (aspect < 0 && x >= q->x[1])
                        q->y[2] = Word(q->y[2] + (q->x[1] - q->x[0]) / aspect);
                    else if (aspect < 0 || q->x[0] < x)
                    {
                        q->y[2] = Word(q->y[2] + (x - q->x[0]) / aspect);
                        q->y[3] = Word(q->y[3] - (q->x[1] - x) / aspect);
                    }
                    else
                        q->y[3] = Word(q->y[3] - (q->x[1] - q->x[0]) / aspect);
                }
            }
        }
        for (Uint16 v = 0; v < 4; ++v)
            Grey(q, v, (i & 1U) ? (v < 2 ? 0 : 96) : (v < 2 ? 255 : 128));
    }
    s->world_y = Word(y + in.scroll_y);
    const Uint8 *motion = in.tables.bytes + 32U + (s->parameter & 15U) * 8U;
    s->velocity_y = Read32(motion);
    s->gravity = Read32(motion + 4);
    Sound(e, x, 0, true);
    e->drop = true;
    e->drop_x = High(x);
    e->drop_y = High(y);
    e->drop_velocity_y = s->velocity_y;
    s->phase = 1;
    return true;
}
static bool InitSurface(SotnWaterEffect *s, SotnWaterEffectInput in, Uint32 *rng,
                        SotnWaterEffectEvents *e)
{
    const Uint16 slope = (s->parameter >> 8) & 127U;
    const Sint16 aspect = Read16(in.tables.bytes + slope * 2U);
    if (slope && !aspect)
        return SDL_SetError("Original surfacing water slope divides by zero in the supplied data");
    const Sint16 x = Pixel(s->x), y = Pixel(s->y);
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        *q = (SotnPlayerEffectQuad){.page = 0x1a,
                                    .palette = i ? 0x161 : 0x162,
                                    .priority = (Uint16)(in.priority + (i ? 4U : 2U)),
                                    .draw_mode = 0x77};
        for (Uint16 v = 0; v < 4; ++v)
        {
            q->x[v] = Word(x + ((v & 1U) ? 9 : -9));
            q->u[v] = (v & 1U) ? 30 : 0;
            q->v[v] = (v & 2U) ? 124 : 96;
            Grey(q, v, v < 2 ? 255 : 128);
        }
        q->y[2] = Word(y + (slope ? 9 / aspect : 0));
        q->y[3] = Word(y - (slope ? 9 / aspect : 0));
    }
    s->world_y = Word(y + in.scroll_y);
    s->top = High(Word(Read16(in.tables.bytes + 64U + (s->parameter & 255U) * 2U) + 12 +
                       (Sint32)(SotnCombat_Random(rng) & 1U)));
    s->velocity_x = s->current * 16;
    if (slope)
    {
        s->velocity_y = s->velocity_x / aspect;
        if (s->velocity_y < 0)
            s->velocity_y = -s->velocity_y;
    }
    if (!(s->parameter & 0x8000U))
        Sound(e, x, 1, true);
    s->phase = 1;
    return true;
}
static bool InitSide(SotnWaterEffect *s, SotnWaterEffectInput in, SotnWaterEffectEvents *e)
{
    const Uint16 item = s->parameter & 15U, index = (s->parameter >> 4) & 15U;
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(in.executable, Read16(in.tables.bytes + 140U + index * 2U), &trig))
        return false;
    const Sint32 first = Read32(in.tables.bytes + 76U + item * 8U),
                 second = Read32(in.tables.bytes + 80U + item * 8U);
    s->velocity_x = Fixed((Uint32)((Sint64)trig.cosine * first + (Sint64)trig.sine * second +
                                   Word(s->parameter & 0xff00U) * 4));
    s->velocity_y = Fixed((Uint32)(-(Sint64)trig.sine * first + (Sint64)trig.cosine * second));
    s->gravity = 11264;
    s->age = 0;
    SotnPlayerEffectQuad *q = &s->quads[0];
    *q = (SotnPlayerEffectQuad){
        .page = 0x1a, .palette = 0x161, .priority = (Uint16)(in.priority + 2U), .draw_mode = 0x37};
    for (Uint16 v = 0; v < 4; ++v)
    {
        q->u[v] = (v & 1U) ? 255 : 240;
        q->v[v] = (v & 2U) ? 15 : 0;
        Grey(q, v, 128);
    }
    if (!item)
        Sound(e, Pixel(s->x), 0, false);
    s->phase = 1;
    return true;
}
static void SplashFrame(SotnWaterEffect *s, SotnWaterEffectInput in)
{
    const Sint16 height = Word(s->world_y - Pixel(s->y) - in.scroll_y);
    for (Uint16 i = 0; i < s->count; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        q->y[0] = Word(q->y[2] - height);
        q->y[1] = Word(q->y[3] - height);
        Uint8 bottom = q->blue[3], top = q->blue[1];
        if (i & 1U)
        {
            if (bottom >= 4)
                bottom = (Uint8)(bottom - 4U);
        }
        else
        {
            if (bottom >= 8)
                bottom = (Uint8)(bottom - 4U);
            if (bottom <= 8 && top >= 8)
                top = (Uint8)(top - 8U);
        }
        for (Uint16 v = 0; v < 4; ++v)
            Grey(q, v, v < 2 ? top : bottom);
    }
}
static bool SurfaceFrame(SotnWaterEffect *s, SotnWaterEffectInput in)
{
    s->x = Add(s->x, s->velocity_x);
    s->y = Add(s->y, s->velocity_y);
    if (s->velocity_x)
    {
        const size_t at = (size_t)s->water_row * 2U;
        if (!in.volumes.bytes || s->water_row % 8 || at > in.volumes.size ||
            in.volumes.size - at < 4)
            return SDL_SetError("Surfacing current requires its actual private volume row");
        const Sint16 left = Read16(in.volumes.bytes + at);
        const Sint16 edge = Word(
            left + (s->velocity_x < 0 ? 6 : Read16(in.volumes.bytes + at + 2) - 6) - in.scroll_x);
        if ((s->velocity_x < 0 && Pixel(s->x) < edge) || (s->velocity_x > 0 && Pixel(s->x) >= edge))
        {
            s->removed = true;
            return true;
        }
    }
    const Sint16 y = Word(s->world_y - Pixel(s->y) - in.scroll_y), x = Pixel(s->x);
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        q->y[2] = Word(q->y[2] - y);
        q->y[3] = Word(q->y[3] - y);
        q->y[0] = Word(q->y[2] - Pixel(s->top));
        q->y[1] = Word(q->y[3] - Pixel(s->top));
        q->x[0] = q->x[2] = Word(x - 9);
        q->x[1] = q->x[3] = Word(x + 9);
        for (Uint16 v = 0; v < 4; ++v)
            Grey(q, v, (Uint8)(q->blue[v] - (v < 2 ? 8U : 4U)));
        if (q->red[0] < 9)
        {
            s->removed = true;
            return true;
        }
    }
    s->world_y = Word(Pixel(s->y) + in.scroll_y);
    return true;
}
bool SotnWaterEffect_Tick(SotnWaterEffect *effect, SotnWaterEffectInput in, Uint32 *random,
                          SotnWaterEffectEvents *out)
{
    Uint16 rows = 0;
    if (!effect || !random || !out || effect->removed || effect->phase > 2 ||
        !SotnWaterEffect_Rows(effect->kind, effect->parameter, &rows) ||
        (effect->kind != SOTN_WATER_SPLASH && effect->phase > 1) ||
        in.priority > (effect->kind == SOTN_WATER_SIDE_SPLASH ? 509U : 507U) ||
        (effect->phase && effect->count != rows))
        return SDL_SetError("Water effect requires its actual bounded child/row lifecycle");
    SotnWaterEffect s = *effect;
    SotnWaterEffectEvents e = {0};
    Uint32 rng = *random;
    const bool birth = !s.phase;
    if (birth && !in.allocated_rows)
    {
        s.removed = true;
        goto done;
    }
    if (in.allocated_rows != rows || !in.tables.bytes || in.tables.size != 188)
        return SDL_SetError("Water effect requires its real rows and complete private US tables");
    if (birth)
    {
        s.count = (Uint8)rows;
        if ((s.kind == SOTN_WATER_SPLASH && !InitSplash(&s, in, &e)) ||
            (s.kind == SOTN_WATER_SURFACING && !InitSurface(&s, in, &rng, &e)) ||
            (s.kind == SOTN_WATER_SIDE_SPLASH && !InitSide(&s, in, &e)))
            return false;
    }
    else if (s.kind == SOTN_WATER_SURFACING)
        s.top = Add(s.top, -16384);
    else
    {
        s.x = Add(s.x, s.velocity_x);
        s.y = Add(s.y, s.velocity_y);
        if (s.phase == 1)
        {
            s.velocity_y = Add(s.velocity_y, s.gravity);
            if (s.kind == SOTN_WATER_SPLASH && s.velocity_y > 163840)
                s.phase = 2;
        }
        else if (s.quads[0].red[0] < 9)
        {
            s.removed = true;
            goto done;
        }
    }
    if (s.kind == SOTN_WATER_SPLASH)
        SplashFrame(&s, in);
    else if (s.kind == SOTN_WATER_SURFACING)
    {
        if (!SurfaceFrame(&s, in))
            return false;
    }
    else
    {
        SotnPlayerEffectQuad *q = &s.quads[0];
        const Sint32 radius = s.age / 2 + 4;
        for (Uint16 v = 0; v < 4; ++v)
        {
            q->x[v] = Word(Pixel(s.x) + ((v & 1U) ? radius : -radius));
            q->y[v] = Word(Pixel(s.y) + ((v & 2U) ? radius : -radius));
        }
        if (q->blue[1] < 3)
        {
            s.removed = true;
            goto done;
        }
        const Uint8 top = (Uint8)(q->blue[1] - 3U),
                    bottom = q->blue[3] >= 4 ? (Uint8)(q->blue[3] - 4U) : q->blue[3];
        for (Uint16 v = 0; v < 4; ++v)
            Grey(q, v, v < 2 ? top : bottom);
        ++s.age;
    }
done:
    *effect = s;
    *random = rng;
    *out = e;
    return true;
}
bool SotnWaterEffect_Translate(SotnWaterEffect *s, Sint32 dx, Sint32 dy)
{
    if (!s || s->count > 4 || dx < -32768 || dx > 32767 || dy < -32768 || dy > 32767)
        return SDL_SetError(
            "Water effect camera shift requires signed-word deltas and real packets");
    s->x = Add(s->x, High((Sint16)dx));
    s->y = Add(s->y, High((Sint16)dy));
    for (Uint16 i = 0; i < s->count; ++i)
        for (Uint16 v = 0; v < 4; ++v)
        {
            s->quads[i].x[v] = Word(s->quads[i].x[v] + dx);
            s->quads[i].y[v] = Word(s->quads[i].y[v] + dy);
        }
    return true;
}
