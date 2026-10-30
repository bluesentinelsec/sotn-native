// SPDX-License-Identifier: AGPL-3.0-or-later
// st/water_effects.h:EntitySmallWaterDrop and st/entity_water_drop.h,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/combat.h>
#include <sotn/water_drops.h>

static Sint16 Word(Sint32 v)
{
    const Uint16 b = (Uint16)(Uint32)v;
    return b < 32768U ? (Sint16)b : (Sint16)((Sint32)b - 65536);
}
static Sint32 Fixed(Uint32 b)
{
    return b <= SDL_MAX_SINT32 ? (Sint32)b : (Sint32)((Sint64)b - 4294967296LL);
}
static Sint16 Pixel(Sint32 v)
{
    return Word((Sint32)((Uint32)v >> 16));
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Sint32 WithHigh(Sint32 v, Sint16 hi)
{
    return Fixed(((Uint32)v & 65535U) | (Uint32)(Uint16)hi << 16);
}
static Sint32 Read32(const Uint8 *p)
{
    return Fixed((Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24);
}
static Uint32 StageRandom(Uint32 *s)
{
    *s = *s * 0x01010101U + 1U;
    return *s >> 24;
}
static bool Small(SotnWaterDrops *s, SotnWaterDropsInput in, Uint32 *rng)
{
    SotnWaterDropTile *p = &s->tiles[0];
    if (!s->phase)
    {
        const Sint16 upper = Word(s->parameter & 0xff00U);
        const Sint16 y = Word(Pixel(s->y) - (Sint32)(StageRandom(rng) & 3U));
        const Sint32 dx = (Sint32)(StageRandom(rng) & 3U);
        const Sint16 x = Word(Pixel(s->x) + (upper > 0 ? dx : -dx));
        s->x = WithHigh(s->x, x);
        s->y = WithHigh(s->y, y);
        *p = (SotnWaterDropTile){.x = x,
                                 .y = y,
                                 .width = 2,
                                 .height = 2,
                                 .red = 96,
                                 .green = 96,
                                 .blue = 128,
                                 .priority = (Uint16)(in.priority + 2U),
                                 .draw_mode = 0x33};
        const Uint8 *v = in.tables.bytes + 156U + (s->parameter & 255U) * 8U;
        const Sint32 vx = Read32(v);
        s->velocity_x = Add(upper > 0 ? Fixed(0U - (Uint32)vx) : vx, upper * 16);
        s->velocity_y = Read32(v + 4);
        s->gravity = 16384;
        s->phase = 1;
    }
    else
    {
        s->x = Add(s->x, s->velocity_x);
        s->y = Add(s->y, s->velocity_y);
        s->velocity_y = Add(s->velocity_y, s->gravity);
    }
    p->x = Pixel(s->x);
    p->y = Pixel(s->y);
    if (p->blue >= 8)
    {
        p->blue = (Uint8)(p->blue - 8U);
        p->red = p->green = (Uint8)(p->green - 6U);
    }
    else
        s->removed = true;
    return true;
}
bool SotnWaterDrops_Tick(SotnWaterDrops *drops, SotnWaterDropsInput in, Uint32 *random)
{
    if (!drops || !random || drops->removed || drops->phase > 1 || in.priority > 509 ||
        (drops->small && (drops->parameter & 255U) >= 4) ||
        (!drops->small && drops->phase && (!drops->timer || drops->timer > 47)))
        return SDL_SetError("Water drops require actual bounded role/lifetime/priority");
    SotnWaterDrops s = *drops;
    Uint32 rng = *random;
    if (!s.phase && !in.allocated_rows)
    {
        s.removed = true;
        goto done;
    }
    if (in.allocated_rows != (s.small ? 1 : 33))
        return SDL_SetError("Water drops require one real tile or33 contiguous sentinel rows");
    if (s.small)
    {
        if (!in.tables.bytes || in.tables.size != 188)
            return SDL_SetError("Small water drop requires the complete private US water block");
        if (!Small(&s, in, &rng))
            return false;
    }
    else if (!s.phase)
    {
        s.phase = 1;
        s.timer = 47;
        for (Uint16 i = 0; i < 33; ++i)
        {
            SotnWaterDropTile *p = &s.tiles[i];
            *p = (SotnWaterDropTile){.priority = (Uint16)(in.priority + 2U), .draw_mode = 0x73};
            if (i == 32)
                break;
            p->velocity_y = Add((Sint32)SotnCombat_Random(&rng) * 8, s.velocity_y);
            p->y = Word(Pixel(s.y) + (Sint32)(SotnCombat_Random(&rng) & 15U));
            p->x = Word(Pixel(s.x) + (Sint32)(SotnCombat_Random(&rng) & 31U) - 16);
            p->delay = (Sint16)((SotnCombat_Random(&rng) & 15U) + 32U);
            p->fixed_x = WithHigh(0, p->x);
            p->fixed_y = WithHigh(0, p->y);
            p->red = p->green = p->blue = 255;
            p->width = p->height = 2;
        }
    }
    else if (!--s.timer)
        s.removed = true;
    else
    {
        for (Uint16 i = 0; i < 32; ++i)
        {
            SotnWaterDropTile *p = &s.tiles[i];
            p->fixed_x = WithHigh(p->fixed_x, p->x);
            p->fixed_y = WithHigh(p->fixed_y, p->y);
            p->delay = Word(p->delay - 1);
            if (!p->delay)
                p->draw_mode |= 8U;
            p->fixed_y = Add(p->fixed_y, p->velocity_y);
            if (p->velocity_y > 32768)
            {
                p->red = (Uint8)(p->red - 4U);
                p->green = (Uint8)(p->green - 4U);
                p->blue = (Uint8)(p->blue - 4U);
            }
            else
                p->velocity_y = Add(p->velocity_y, 14336);
            p->x = Pixel(p->fixed_x);
            p->y = Pixel(p->fixed_y);
        }
        s.tiles[32].draw_mode &= (Uint16)~8U;
        s.tiles[32].x = s.tiles[32].y = 0;
        s.tiles[32].width = 0;
    }
done:
    *drops = s;
    *random = rng;
    return true;
}
bool SotnWaterDrops_Translate(SotnWaterDrops *s, Sint32 dx, Sint32 dy)
{
    if (!s || dx < SDL_MIN_SINT16 || dx > SDL_MAX_SINT16 || dy < SDL_MIN_SINT16 ||
        dy > SDL_MAX_SINT16)
        return SDL_SetError("Water drop camera shift requires an owner and signed-word deltas");
    s->x = Add(s->x, Fixed((Uint32)dx * 65536U));
    s->y = Add(s->y, Fixed((Uint32)dy * 65536U));
    if (s->phase)
        for (Uint16 i = 0; i < (s->small ? 1 : 33); ++i)
        {
            s->tiles[i].x = Word(s->tiles[i].x + dx);
            s->tiles[i].y = Word(s->tiles[i].y + dy);
        }
    return true;
}
