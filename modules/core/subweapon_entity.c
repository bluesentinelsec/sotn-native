// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/84B88.c:EntitySubwpnThrownVibhuti/EntitySubwpnReboundStone and rebound_stone.h,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// SDK trigonometry and collision tables remain private runtime inputs.
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/subweapon_entity.h>

static Sint32 Pixel(Sint32 v)
{
    return v >= 0 ? v / 65536 : (Sint32)(-((-(Sint64)v + 65535) / 65536));
}
// Arithmetic right shift (floor), as the MIPS sra the original compiles to.
static Sint32 Asr(Sint32 v, Uint32 n)
{
    return v >= 0 ? v >> n : (Sint32)(-((-(Sint64)v + ((Sint64)1 << n) - 1) >> n));
}
// Replace a 16.16 value's whole word, keeping its fraction (posX.i.hi = ...).
static Sint32 WithPixel(Sint32 v, Sint32 pixel)
{
    const Uint32 bits = ((Uint32)pixel << 16) | ((Uint32)v & 65535U);
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool Birth(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                  SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    next.flags = 0x08800000U; // FLAG_POS_CAMERA_LOCKED | FLAG_HAS_PRIMS
    next.hitbox_width = next.hitbox_height = 4;
    next.timer = 0x80;
    next.priority = (Uint16)(in.player_priority + 2U);
    const Sint32 x = Pixel(e->x), y = Pixel(e->y) - 8;
    Uint32 cursor = *random;
    if (!in.properties(in.context, &cursor))
        return false;
    for (Uint16 i = 0; i < SOTN_VIBHUTI_PARTICLES; ++i)
    {
        const Sint32 angle = (SotnCombat_Random(&cursor) & 0xff) + 0x100;
        const Sint32 speed = (SotnCombat_Random(&cursor) & 0xff) + 0x80;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(in.executable, angle, &trig))
            return false;
        Sint32 vx = Asr((Sint32)trig.cosine * 16 * speed, 9) + 0x8000;
        const Sint32 vy = -Asr((Sint32)trig.sine * 16 * speed, 9);
        vx = Asr(vx * 3, 1);
        next.particles[i] = (SotnVibhutiParticle){.x = x * 65536,
                                                  .y = (y - 4) * 65536,
                                                  .velocity_x = e->facing_left ? -vx : vx,
                                                  .velocity_y = vy,
                                                  .draw_mode = 2,
                                                  .width = 2,
                                                  .height = 2,
                                                  .falling = true};
    }
    next.step = 1;
    *e = next;
    *random = cursor;
    out->sound = 0x60c; // SFX_WEAPON_SWISH_C
    out->use_ticks = 4; // g_Player.timers[10]
    return true;
}
static bool Fly(SotnSubweaponEntity *e, SotnSubweaponInput in)
{
    SotnSubweaponEntity next = *e;
    const Sint32 probe = e->facing_left ? -2 : 2;
    if (--next.timer == 0)
    {
        next.removed = true;
        *e = next;
        return true;
    }
    for (Uint16 i = 0; i < SOTN_VIBHUTI_PARTICLES; ++i)
    {
        SotnVibhutiParticle *p = &next.particles[i];
        if (p->falling)
        {
            SotnCollider col = {0};
            if (p->velocity_x)
            {
                p->x += p->velocity_x;
                if (!SotnCollision_Query(&in.collision, Pixel(p->x) + probe, Pixel(p->y), &col))
                    return false;
                if (col.effects & SOTN_EFFECT_SIDE)
                    p->velocity_x = 0;
            }
            p->y += p->velocity_y;
            p->velocity_y += 0x1800; // FIX(12.0 / 128)
            if (p->velocity_y > 0x40000)
                p->velocity_y = 0x40000;
            if (p->velocity_y > 0)
            {
                if (!SotnCollision_Query(&in.collision, Pixel(p->x), Pixel(p->y), &col))
                    return false;
                if (col.effects & SOTN_EFFECT_SOLID)
                {
                    p->falling = false;
                    p->y = WithPixel(p->y, Pixel(p->y) + col.top - (Sint32)(i % 3U + 1U));
                    p->width = p->height = 3;
                }
            }
        }
        // Only particles0..7 ever carry the hitbox; it cycles with the timer.
        if ((next.timer & 7U) == (Uint32)i)
        {
            next.x = WithPixel(next.x, Pixel(p->x));
            next.y = WithPixel(next.y, Pixel(p->y));
            next.hitbox_width = next.hitbox_height = (p->draw_mode & 8U) ? 0 : 4;
            next.hitbox_y = p->falling ? 0 : -6;
        }
        // DRAW_HIDE replaces the whole mode: hidden rows stop following the camera.
        if (in.hit_flags && ((next.timer + 1U) & 7U) == (Uint32)i)
            p->draw_mode = 8;
        if (next.timer - 1U == (Uint32)i)
            p->draw_mode = 8;
    }
    *e = next;
    return true;
}
static Sint16 Word(Sint32 v)
{
    const Uint16 bits = (Uint16)(Uint32)v;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
// rebound_stone.h: the first helper always reflects; the second only before this update's bounce.
static void Reflect(SotnReboundStone *s, bool *bounced, Sint16 surface, bool always)
{
    if (always || !*bounced)
        s->angle = Word(surface * 2 - s->angle);
    if (!*bounced)
    {
        ++s->bounces;
        *bounced = true;
    }
}
// The slope effects (0x8000 floor, 0x0800 ceiling, 0x4000 left, 0x2000/0x1000 steepness).
static void Vertical(SotnReboundStone *s, bool *bounced, Uint32 flags, Sint32 dy)
{
    if (dy > 0)
    {
        if (!flags || (flags & 0x0800U))
            Reflect(s, bounced, 0x800, true);
        if (flags == 0x8000U)
            Reflect(s, bounced, 0x200, false);
        if (flags == 0x9000U)
            Reflect(s, bounced, 0x12e, false);
        if (flags == 0xa000U)
            Reflect(s, bounced, 0xa0, false);
        if (flags == 0xc000U)
            Reflect(s, bounced, 0x600, false);
        if (flags == 0xd000U)
            Reflect(s, bounced, 0x6d2, false);
        if (flags == 0xe000U)
            Reflect(s, bounced, 0x760, false);
    }
    if (dy < 0)
    {
        if (!flags || (flags & 0x8000U))
            Reflect(s, bounced, 0x800, true);
        if (flags == 0x0800U)
            Reflect(s, bounced, 0xe00, false);
        if (flags == 0x1800U)
            Reflect(s, bounced, 0xed2, false);
        if (flags == 0x2800U)
            Reflect(s, bounced, 0xf60, false);
        if (flags == 0x4800U)
            Reflect(s, bounced, 0xa00, false);
        if (flags == 0x5800U)
            Reflect(s, bounced, 0x92e, false);
        if (flags == 0x6800U)
            Reflect(s, bounced, 0x8a0, false);
    }
}
static void Horizontal(SotnReboundStone *s, bool *bounced, Uint32 flags, Sint32 dx)
{
    if (dx > 0)
    {
        if (!flags || (flags & 0x4800U) == 0x4800U || (flags & 0xc000U) == 0xc000U)
            Reflect(s, bounced, 0x400, true);
        if (flags == 0x0800U)
            Reflect(s, bounced, 0xe00, false);
        if (flags == 0x1800U)
            Reflect(s, bounced, 0xed2, false);
        if (flags == 0x2800U)
            Reflect(s, bounced, 0xf60, false);
        if (flags == 0x8000U)
            Reflect(s, bounced, 0x200, false);
        if (flags == 0x9000U)
            Reflect(s, bounced, 0x12e, false);
        if (flags == 0xa000U)
            Reflect(s, bounced, 0xa0, false);
    }
    if (dx < 0)
    {
        if (!flags || (flags & 0x4800U) == 0x0800U || (flags & 0xc000U) == 0x8000U)
            Reflect(s, bounced, 0x400, true);
        if (flags == 0x4800U)
            Reflect(s, bounced, 0xa00, false);
        if (flags == 0x5800U)
            Reflect(s, bounced, 0x92e, false);
        if (flags == 0x6800U)
            Reflect(s, bounced, 0x8a0, false);
        if (flags == 0xc000U)
            Reflect(s, bounced, 0x600, false);
        if (flags == 0xd000U)
            Reflect(s, bounced, 0x6d2, false);
        if (flags == 0xe000U)
            Reflect(s, bounced, 0x760, false);
    }
}
// CheckCollision's masked effects; zero unless the probe is solid.
static bool Probe(SotnCollisionMap map, Sint32 x, Sint32 y, Uint32 *flags)
{
    SotnCollider col = {0};
    if (!SotnCollision_Query(&map, x, y, &col))
        return false;
    *flags = col.effects & 0xf803U;
    *flags = (*flags & SOTN_EFFECT_SOLID) ? *flags & 0xff00U : 0xffffffffU;
    return true;
}
static bool Stone(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                  SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnReboundStone *s = &next.stone;
    bool bounced = false;
    const Sint32 start_x = Pixel(e->x), start_y = Pixel(e->y);
    Uint32 cursor = *random;
    if (!e->step)
    {
        for (Uint16 i = 0; i < SOTN_REBOUND_STONE_LINES; ++i)
            s->lines[i] = (SotnReboundLine){.x0 = start_x,
                                            .y0 = start_y,
                                            .x1 = start_x,
                                            .y1 = start_y,
                                            .draw_mode = (Uint16)(i ? 0x3b : 0x33),
                                            .grey = 0xff,
                                            .timer = 20};
        // FLAG_POS_CAMERA_LOCKED | FLAG_KEEP_ALIVE_OFFCAMERA | FLAG_HAS_PRIMS
        next.flags = 0x0c800000U;
        next.priority = (Uint16)(in.player_priority + 2U);
        s->angle = Word((in.player_facing_left ? 0x980 : 0xe80) +
                        (Sint32)(SotnCombat_Random(&cursor) & 0x7fU) - 0x40);
        s->life = 0x40;
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_width = next.hitbox_height = 4;
        out->use_ticks = 4;
        SotnCollider col = {0};
        if (!SotnCollision_Query(&in.collision, start_x, start_y, &col))
            return false;
        if (col.effects & SOTN_EFFECT_SOLID)
            s->embed = 4;
        next.step = 1;
        out->sound = 0x60c; // SFX_WEAPON_SWISH_C
    }
    else if (e->step == 1)
    {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, s->angle, &t))
            return false;
        const Sint32 dx = (Sint32)t.cosine * 16, dy = -(Sint32)t.sine * 16;
        Sint32 cx = e->x, cy = e->y;
        if (!s->embed)
            for (Uint16 i = 0; i < 6; ++i)
            {
                Uint32 flags = 0;
                if (!Probe(in.collision, Pixel(cx), Pixel(Add(cy, dy)), &flags))
                    return false;
                if (flags != 0xffffffffU)
                    Vertical(s, &bounced, flags, dy);
                if (!Probe(in.collision, Pixel(Add(cx, dx)), Pixel(cy), &flags))
                    return false;
                if (flags != 0xffffffffU)
                    Horizontal(s, &bounced, flags, dx);
                if (bounced)
                    break;
                cx = Add(cx, dx);
                cy = Add(cy, dy);
            }
        else
            --s->embed;
        if (bounced)
        {
            out->factory = true;
            out->factory_packed = SOTN_REBOUND_STONE_SPARKS;
            out->factory_x = e->x;
            out->factory_y = e->y;
            out->sound = 0x6a4; // SFX_UI_SUBWEAPON_TINK
        }
        const Sint32 sx = Pixel(e->x) - in.screen_x, sy = Pixel(e->y) - in.screen_y;
        if (sx < -0x40 || sx > 0x140 || sy < -0x40 || sy > 0x140 || s->bounces == 15)
            next.step = 2;
        else
        {
            if (!SotnMath_Trig(in.executable, s->angle, &t))
                return false;
            next.x = Add(next.x, Asr((Sint32)t.cosine * 16 * 0x400, 8));
            next.y = Add(next.y, Asr(-((Sint32)t.sine * 16 * 0x400), 8));
        }
    }
    else
    {
        if (--s->life == 0)
        {
            next.removed = true;
            *e = next;
            return true;
        }
        if (s->life == 0x20)
            out->disable_attack = true;
        for (Uint16 i = 0; i < SOTN_REBOUND_STONE_LINES; ++i)
            s->lines[i].timer = 0;
    }
    const Uint8 fade = next.step == 2 ? 4 : 2;
    for (Uint16 i = 0; i < SOTN_REBOUND_STONE_LINES; ++i)
    {
        SotnReboundLine *l = &s->lines[i];
        if (bounced && i == s->bounces)
        {
            l->x0 = start_x;
            l->y0 = start_y;
            l->draw_mode = (Uint16)(l->draw_mode & 0xfff7U);
        }
        if (i == s->bounces)
        {
            l->x1 = Pixel(next.x);
            l->y1 = Pixel(next.y);
        }
        if (!(l->draw_mode & 8U))
        {
            if (l->timer)
                --l->timer;
            else if (fade < l->grey)
                l->grey = (Uint8)(l->grey - fade);
        }
    }
    *e = next;
    *random = cursor;
    return true;
}
Uint16 SotnSubweaponEntity_Rows(Uint16 id)
{
    return id == SOTN_SUBWEAPON_VIBHUTI         ? SOTN_VIBHUTI_ROWS
           : id == SOTN_SUBWEAPON_REBOUND_STONE ? SOTN_REBOUND_STONE_LINES
                                                : 0;
}
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                              SotnSubweaponEvents *events)
{
    const bool vibhuti = e && e->id == SOTN_SUBWEAPON_VIBHUTI,
               stone = e && e->id == SOTN_SUBWEAPON_REBOUND_STONE;
    if (!e || !random || !events || e->removed || (!vibhuti && !stone) || !in.properties ||
        (vibhuti && (e->step > 1 || (e->step && (!e->timer || e->timer > 0x80)))) ||
        (stone && (e->step > 2 || (e->step && (e->stone.life < 1 || e->stone.life > 0x40 ||
                                               e->stone.bounces < 0 || e->stone.bounces > 15)))) ||
        in.hit_flags > 3)
        return SDL_SetError("Subweapon callback requires its actual live entity and outputs");
    *events = (SotnSubweaponEvents){0};
    if (!e->step && !in.allocated)
    {
        e->removed = true; // Row allocation failure: DestroyEntity before any RNG.
        return true;
    }
    if (stone)
        return Stone(e, in, random, events);
    if (e->step)
        return Fly(e, in);
    return Birth(e, in, random, events);
}
