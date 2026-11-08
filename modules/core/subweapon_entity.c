// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/84B88.c:EntitySubwpnKnife/EntitySubwpnThrownAxe/EntitySubwpnHolyWater/
// EntitySubwpnHolyWaterBreakGlass/EntitySubwpnHolyWaterFlame/CheckHolyWaterCollision/EntitySubwpnThrownVibhuti/EntitySubwpnReboundStone/EntitySubwpnAgunea/
// EntityAguneaHitEnemy/EntitySubwpnCrashCross/EntitySubwpnCrashCrossParticles, wolfForm.c:
// EntityGiantSpinningCross, 7879C.c:func_80119E78, 8A0A4.c:EntitySubwpnBible/
// EntitySubwpnBibleTrail and rebound_stone.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// SDK trigonometry, collision tables and the cross model remain private runtime inputs.
#include "boss_projection.h"
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
            out->factories = 1;
            out->factory_packed[0] = SOTN_REBOUND_STONE_SPARKS;
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
// The retained raw stage pointer: DestroyEntity zeroes an emptied slot, so it reads (0, 0).
static bool Target(SotnSubweaponInput in, Uint16 slot, SotnSpellTarget *out)
{
    if (!in.targets || slot >= SOTN_STAGE_TARGETS)
        return SDL_SetError("Agunea requires the actual stage view of its retained target slot");
    *out = in.targets[slot];
    if (!out->identity)
        *out = (SotnSpellTarget){0};
    return true;
}
static void Fade(Uint8 *channel)
{
    if (*channel >= 4)
        *channel = (Uint8)(*channel - 4U);
}
// EntitySubwpnAgunea. A hit binds the bolt to the enemy collision wrote to unkB8; while Up and
// Square stay held it strikes it every twelfth update (the first strike free, later ones five
// hearts) and sits on it with its live hitbox.
static bool Agunea(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                   SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnAgunea *a = &next.agunea;
    Uint32 cursor = *random;
    SotnSpellTarget t = {0};
    switch (e->step)
    {
    case 0: {
        next.flags = 0x0c800000U; // FLAG_POS_CAMERA_LOCKED | KEEP_ALIVE_OFFCAMERA | HAS_PRIMS
        next.facing_left = in.player_facing_left;
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_width = next.hitbox_height = 4;
        next.hitbox_x = 4;
        next.hitbox_y = 0;
        const Sint32 x = Pixel(in.player_x), y = Pixel(in.player_y) + in.player_hitbox_y - 8;
        next.x = WithPixel(next.x, x);
        next.y = WithPixel(next.y, y);
        a->start_x = x;
        a->start_y = y;
        a->priority = (Uint16)(in.player_priority + 2U);
        a->draw_mode = 0x331; // DRAW_UNK_200 | DRAW_UNK_100 | TPAGE2 | TPAGE | TRANSP: ABR1
        a->red = 0x60;
        a->green = 0;
        a->blue = 0x80;
        next.velocity_x = next.facing_left ? -0x60000 : 0x60000; // SetSpeedX(FIX(6))
        out->sound = 0x60c;                                      // SFX_WEAPON_SWISH_C
        out->factories = 1;
        out->factory_packed[0] = SOTN_AGUNEA_FLASH;
        out->factory_x = next.x;
        out->factory_y = next.y;
        out->use_ticks = 4;
        next.step = 1;
        break;
    }
    case 1: {
        next.x = Add(next.x, next.velocity_x);
        const Sint16 sx = Word(Pixel(next.x) - in.screen_x), sy = Word(Pixel(next.y) - in.screen_y);
        if (sx < -0x40 || sx > 0x140 || sy < -0x20 || sy > 0x120)
            next.step = 2;
        if (in.hit_flags)
        {
            a->target = in.hit_target;
            next.step = 3;
        }
        break;
    }
    case 4:
        if (!Target(in, a->target, &t))
            return false;
        next.x = WithPixel(next.x, t.x + in.screen_x);
        next.y = WithPixel(next.y, t.y + in.screen_y);
        a->counter = Word(a->counter + 1);
        if (a->counter < 16)
            break;
        SDL_FALLTHROUGH;
    case 2:
        if (a->red < 5)
        {
            next.removed = true;
            *e = next;
            return true;
        }
        break;
    case 3: {
        if ((in.buttons & SOTN_AGUNEA_HELD) != SOTN_AGUNEA_HELD)
            next.step = 4;
        if (!Target(in, a->target, &t))
            return false;
        // Both exits return before the primitive update.
        if (!t.identity || (a->counter && (t.hp > 0x7000 || !t.hp || (t.flags & 0x100U))))
        {
            next.step = 2;
            *e = next;
            return true;
        }
        const Sint32 x = t.x + in.screen_x, y = t.y + in.screen_y;
        if (a->counter % 12 == 0)
        {
            next.x = WithPixel(next.x, x + (Sint32)(SotnCombat_Random(&cursor) & 0xfU) - 8);
            next.y = WithPixel(next.y, y + (Sint32)(SotnCombat_Random(&cursor) & 0xfU) - 8);
            bool strike = !a->struck;
            if (strike)
                a->struck = true;
            else if (in.hearts >= in.bolt_cost)
            {
                out->hearts_spent = in.bolt_cost;
                strike = true;
            }
            else
                next.step = 4;
            if (strike)
            {
                out->factories = 2;
                out->factory_packed[0] = SOTN_AGUNEA_BOLT_BLUEPRINT;
                out->factory_packed[1] = SOTN_AGUNEA_OUTLINE;
                out->factory_x = next.x;
                out->factory_y = next.y;
                out->sound = 0x665; // SFX_THUNDER_B
            }
        }
        next.x = WithPixel(next.x, x);
        next.y = WithPixel(next.y, y);
        a->counter = Word(a->counter + 1);
        break;
    }
    }
    Fade(&a->red);
    Fade(&a->green);
    Fade(&a->blue);
    if (a->blue < 5)
        a->draw_mode |= 8U;
    a->end_x = Pixel(next.x);
    a->end_y = Pixel(next.y);
    *e = next;
    *random = cursor;
    return true;
}
static Sint16 Abs(Sint16 v)
{
    return (Sint16)(v < 0 ? -v : v);
}
// EntityAguneaHitEnemy: a forty-row lightning chain grown two segments per update from the
// player toward its raw parent bolt, flashed white, then faded.
static bool Bolt(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random)
{
    SotnSubweaponEntity next = *e;
    SotnAguneaBolt *b = &next.bolt;
    Uint32 cursor = *random;
    const Sint32 px = Pixel(in.player_x), py = Pixel(in.player_y) + in.player_hitbox_y - 8;
    next.x = WithPixel(next.x, px);
    next.y = WithPixel(next.y, py);
    if (in.parent_entity != SOTN_AGUNEA_ENTITY)
    {
        if (!next.step)
        {
            next.removed = true;
            *e = next;
            return true;
        }
        next.step = 3;
    }
    switch (next.step)
    {
    case 0:
        next.flags = 0x04800000U; // FLAG_KEEP_ALIVE_OFFCAMERA | FLAG_HAS_PRIMS
        next.facing_left = in.player_facing_left;
        // The original's facing adjustment (unk84 + 0x800) is discarded.
        b->angle = (Sint16)(((Sint32)(SotnCombat_Random(&cursor) & 0x3ffU) - 0x200) & 0xfff);
        b->x[0][0] = b->x[0][1] = b->x[0][2] = b->x[0][3] = Word(px);
        b->y[0][0] = b->y[0][2] = Word(py);
        b->y[0][3] = Word(py - 16);
        b->grey = 0x80;
        b->count = 0;
        b->end = 0;
        next.step = 1;
        break;
    case 1:
        for (Uint16 i = 0; i < 2; ++i)
        {
            Sint16 angle = b->angle;
            const Uint8 end = b->end;
            const Sint16 dx = Word(in.parent_x - b->x[end][2]),
                         dy = Word(in.parent_y - b->y[end][2]);
            if (Abs(dx) < 8 && Abs(dy) < 8)
            {
                next.step = 2;
                break;
            }
            const bool near = Abs(dx) < 0x40 && Abs(dy) < 0x40;
            if (!b->count)
            {
                b->count = near ? 2 : 4;
                Sint16 heading = 0;
                if (!SotnMath_Atan2(in.executable, -(Sint32)dy, dx, &heading))
                    return false;
                Sint16 turn = Word(heading - angle);
                if (turn > 0x800)
                    turn = Word(0x1000 - turn);
                if (turn < -0x800)
                    turn = Word(0x1000 + turn);
                b->turn = (Sint16)(near ? turn / 2 : turn / 4);
            }
            angle = Word(angle + b->turn);
            if (!near)
                angle = Word(angle + 0x180 - (Sint32)((SotnCombat_Random(&cursor) & 3U) << 8));
            angle = (Sint16)(angle & 0xfff);
            if (end + 1 >= SOTN_AGUNEA_BOLT_ROWS)
            {
                next.step = 2;
                break;
            }
            const Uint8 n = (Uint8)(end + 1);
            b->x[n][0] = b->x[end][2];
            b->y[n][0] = b->y[end][2];
            b->x[n][1] = b->x[end][3];
            b->y[n][1] = b->y[end][3];
            b->angle = angle;
            b->end = n;
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, angle, &t))
                return false;
            b->x[n][2] = Word(b->x[n][0] + Asr(t.cosine * 12, 12));
            b->y[n][2] = Word(b->y[n][0] - Asr(t.sine * 12, 12));
            if (!SotnMath_Trig(in.executable, Word(angle - 0x400), &t))
                return false;
            b->x[n][3] = Word(b->x[n][2] + Asr(t.cosine * 16, 12));
            b->y[n][3] = Word(b->y[n][2] - Asr(t.sine * 16, 12));
            b->count = Word(b->count - 1);
        }
        break;
    case 2:
        if (!b->flashed)
        {
            b->white = b->flashed = true;
            break;
        }
        b->white = b->flashed = false;
        b->grey = 0x60;
        next.step = 3;
        break;
    default:
        // PrimDecreaseBrightness(prim, 4) on every row: they all share one grey, clamped at 16.
        if (b->grey - 4 < 16)
        {
            b->grey = 16;
            next.removed = true;
        }
        else
            b->grey = (Uint8)(b->grey - 4U);
        break;
    }
    *e = next;
    *random = cursor;
    return true;
}
// EntitySubwpnBible. The US build reads its third rotation term (var_s4) uninitialised: no
// function on the gameplay call chain (UpdatePlayerEntities up to the SLUS start routine) writes
// that callee-saved register, so it holds its boot value. Zero, as the PSP/FIX_UB builds
// initialise it, keeps the book on its visible orbit and is used here.
static bool Bible(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                  SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnBible *b = &next.bible;
    Uint32 cursor = *random;
    switch (e->step)
    {
    case 0:
        next.flags = 0x04820000U; // KEEP_ALIVE_OFFCAMERA | HAS_PRIMS | FLAG_UNK_20000
        b->swing = next.facing_left ? 32 : -32;
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_width = next.hitbox_height = 6;
        next.step = 1;
        break;
    case 1:
        b->mode = 1;
        out->use_ticks = 4;
        next.step = 2;
        SDL_FALLTHROUGH;
    case 2:
        b->count = Word(b->count + 1);
        b->radius = Word(b->radius + 1);
        if (b->radius >= 0x30)
            next.step = 3;
        break;
    case 3:
        b->count = Word(b->count + 1);
        if (b->count >= 0x12c)
        {
            next.flags &= ~0x04000000U;
            next.velocity_x = next.facing_left ? -0xc0000 : 0xc0000;
            b->velocity_y = -0xc0000;
            out->sound = 0x6b2; // SFX_BIBLE_SCRAPE
            b->mode = 2;
            next.step = 4;
        }
        break;
    default:
        break;
    }
    if (b->mode == 1)
    {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, b->angle, &t))
            return false;
        Sint32 s5 = Asr(t.sine * b->radius, 12), s3 = Asr(t.cosine * b->radius, 12);
        const Sint32 turned = t.cosine * s5 + t.sine * s3, across = t.cosine * s3 - t.sine * s5;
        s5 = Asr(turned, 12);
        s3 = Asr(across, 12);
        if (!SotnMath_Trig(in.executable, b->tilt, &t))
            return false;
        const Sint32 depth = 0; // var_s4: see above
        const Sint32 x = Asr(t.cosine * s5 + t.sine * depth, 12);
        const Sint32 tilted = Asr(t.cosine * depth - t.sine * s5, 12);
        const Sint32 y = next.facing_left ? Asr(t.cosine * tilted + t.sine * s3, 12)
                                          : Asr(t.cosine * tilted - t.sine * s3, 12);
        b->angle = (Sint16)((b->angle + (next.facing_left ? 0x80 : -0x80)) & 0xfff);
        b->tilt = Word(b->tilt + b->swing);
        if (b->tilt >= 0x200 || b->tilt <= -0x200)
            b->swing = Word(-b->swing);
        next.x = WithPixel(next.x, Pixel(in.player_x) + x);
        next.y = WithPixel(next.y, Pixel(in.player_y) + y);
        next.priority = (Uint16)(in.player_priority + (s3 < 0 ? 2 : -2));
    }
    else if (b->mode == 2)
    {
        next.x = Add(next.x, next.velocity_x);
        next.y = Add(next.y, b->velocity_y);
        b->velocity_y = Add(b->velocity_y, -0x20000);
    }
    if (b->mode)
    {
        out->factories = 1;
        out->factory_packed[0] = SOTN_BIBLE_TRAIL_BLUEPRINT;
        out->factory_x = next.x;
        out->factory_y = next.y;
        if (in.game_timer % 10U == 0)
        {
            if (out->sound)
                out->sound_after = 0x8c3;
            else
                out->sound = 0x8c3; // SFX_BIBLE_SUBWPN_SWISH
        }
    }
    *e = next;
    *random = cursor;
    return true;
}
// EntitySubwpnBibleTrail: one half-transparent quad placed at birth, fading for seven updates.
static void BibleTrail(SotnSubweaponEntity *e)
{
    SotnBibleTrail *t = &e->trail;
    switch (e->step)
    {
    case 0:
        e->flags = 0x04820000U; // FLAG_UNK_20000 | KEEP_ALIVE_OFFCAMERA | HAS_PRIMS
        t->x = Pixel(e->x);
        t->y = Pixel(e->y);
        t->grey = 0x60;
        e->step = 1;
        break;
    case 1:
        t->count = Word(t->count + 1);
        if (t->count > 5)
            e->step = 2;
        t->grey = Word(t->grey - 8);
        break;
    default:
        e->removed = true;
        break;
    }
}
// EntitySubwpnCrashCross: func_8010DFF0(1, 1) runs first every update (the owner applies it).
// Geometry is in screen words; left/right are only read on step4 updates, which assign them.
static bool Beam(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                 SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnCrossBeam *b = &next.beam;
    Uint32 cursor = *random;
    const Sint16 x = Word(Pixel(e->x) - in.screen_x);
    Sint16 left = 0, right = 0;
    switch (e->step)
    {
    case 0:
        next.flags = 0x00800000U; // FLAG_HAS_PRIMS
        next.y = WithPixel(next.y, in.screen_y + 0x78);
        b->width = 1;
        next.priority = 0xc2;
        if (!in.properties(in.context, &cursor))
            return false;
        b->crash = (Uint8)in.crash;
        out->upload_gradient = true;
        b->grey = 0x80;
        b->draw_mode = 0x31;      // DRAW_TPAGE2 | DRAW_TPAGE | DRAW_TRANSP
        out->sound = 0x6df;       // SFX_CRASH_CROSS
        out->sound_after = 0x636; // SFX_TELEPORT_BANG_B
        next.step = 1;
        out->spell_use_ticks = 4;
        break;
    case 1:
        b->half_height = Word(b->half_height + 3);
        b->height = (Uint8)(b->height + 6);
        if ((b->half_height & 0xff) >= 0x70)
        {
            out->factories = 2;
            out->factory_packed[0] = SOTN_GIANT_CROSS_BLUEPRINT;
            out->factory_argument[0] = (Uint16)(b->crash << 9);
            out->factory_packed[1] = SOTN_CROSS_PARTICLES_BLUEPRINT;
            out->factory_x = next.x;
            out->factory_y = next.y;
            next.step = 2;
        }
        break;
    case 2:
        if (!(in.engine_timer & 1U))
            break;
        b->half_width = Word(b->half_width + 1);
        b->width = (Uint8)(b->width + 2);
        if (b->width >= 0x2c)
        {
            next.step = 3;
            b->hold = 0x80;
        }
        break;
    case 3:
        b->hold = Add(b->hold, -1);
        if (!b->hold)
        {
            left = Word(x - b->half_width);
            if (left < 0)
                left = 0;
            right = Word(x + b->half_width);
            if (right > 0xff)
                right = 0xff;
            next.step = 4;
            out->sound = 0x62f; // SFX_WEAPON_APPEAR
        }
        break;
    default: {
        const Sint32 distance = Word(SDL_abs(x - 0x80));
        b->half_width = Word(b->half_width + Word(3 * (distance + 0x80) / 112));
        left = Word(x - b->half_width);
        right = Word(x + b->half_width);
        if (right > 0x180 && left < -0x80)
        {
            next.removed = true;
            *e = next;
            *random = cursor;
            return true;
        }
        break;
    }
    }
    next.hitbox_y = 0;
    next.hitbox_height = (Uint8)b->half_height;
    if (next.step == 4)
    {
        next.hitbox_width = (Uint8)Asr(right - left, 1);
        next.hitbox_x = Word(Asr(left + right, 1) - x);
    }
    else
    {
        next.hitbox_width = (Uint8)b->half_width;
        next.hitbox_x = 0;
    }
    b->x0 = Word(x - b->half_width);
    b->y0 = Word(Word(Pixel(next.y) - in.screen_y) - b->half_height);
    b->x1 = Word(b->x0 + b->width);
    b->y2 = Word(b->y0 + b->height);
    if (next.step == 4)
    {
        if (b->grey < 0xf8)
            b->grey = (Uint8)(b->grey + 4);
        b->x0 = left;
        b->x1 = right;
        b->draw_mode = 0x35; // adds DRAW_COLORS
    }
    *e = next;
    *random = cursor;
    return true;
}
static Uint32 Read32(const Uint8 *p)
{
    return (Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
// D_800B0CB4[row]'s four SVECTOR pointers into D_800B0B34..D_800B0CAC (DRA at 0x800A0000).
static bool CrossVertices(SotnDataView dra, Uint16 row, Sint16 out[4][3])
{
    const size_t table = 0x10cb4U + (size_t)row * 16U;
    if (row >= SOTN_GIANT_CROSS_ROWS || !dra.bytes || dra.size < table + 16U)
        return SDL_SetError("Giant cross row %u is outside the private DRA model", row);
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Uint32 address = Read32(dra.bytes + table + i * 4U);
        if (address < 0x800b0b34U || address > 0x800b0cacU || (address & 7U) != 4U)
            return SDL_SetError("Giant cross row %u names a vertex outside its private model", row);
        const Uint8 *v = dra.bytes + (address - 0x800a0000U);
        for (Uint16 k = 0; k < 3; ++k)
            out[i][k] = (Sint16)(Uint16)(v[k * 2U] | v[k * 2U + 1U] << 8);
    }
    return true;
}
bool SotnSubweaponEntity_CrossFace(SotnDataView dra, Uint16 row, Uint8 u[4], Uint8 v[4],
                                   Uint16 *palette)
{
    const size_t at = 0x10f94U + (size_t)row * 5U;
    if (!u || !v || !palette || row >= SOTN_GIANT_CROSS_ROWS || !dra.bytes || dra.size < at + 5U)
        return SDL_SetError("Giant cross face %u is outside the private DRA table", row);
    const Uint8 *c = dra.bytes + at;
    const Uint8 l = c[0], t = c[1], r = (Uint8)(c[0] + c[2]), b = (Uint8)(c[1] + c[3]);
    // Each case writes the corners of one mirrored/rotated variant of the rectangle.
    static const Uint8 corners[8][4] = {{0, 1, 2, 3}, {1, 0, 3, 2}, {2, 3, 0, 1}, {3, 2, 1, 0},
                                        {3, 1, 2, 0}, {2, 0, 3, 1}, {1, 3, 0, 2}, {0, 2, 1, 3}};
    const Uint8 cu[4] = {l, r, l, r}, cv[4] = {t, t, b, b};
    const Uint8 kind = (Uint8)((c[4] & 0xf0U) >> 4);
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Uint8 corner = corners[kind < 8 ? kind : 0][i];
        u[i] = cu[corner];
        v[i] = cv[corner];
    }
    *palette = (Uint16)((c[4] & 0xfU) + 0x1a0U);
    return true;
}
bool SotnSubweaponEntity_SparkFace(SotnDataView dra, Uint16 frame, Uint8 u[4], Uint8 v[4])
{
    const size_t at = 0xd094U + (size_t)frame * 8U;
    if (!u || !v || frame >= 6 || !dra.bytes || dra.size < at + 8U)
        return SDL_SetError("Cross particle frame %u is outside the private DRA table", frame);
    for (Uint16 i = 0; i < 4; ++i)
    {
        u[i] = dra.bytes[at + i * 2U];
        v[i] = dra.bytes[at + i * 2U + 1U];
    }
    return true;
}
// EntityGiantSpinningCross: RotMatrix(0, spin, 0x40), TransMatrix of its orbit, SetGeomOffset
// (posX, 120), SetGeomScreen(320). Each row projects vectors 0/1/3 (RTPT, NCLIP) then 2.
static bool GiantCross(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                       SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnGiantCross *c = &next.cross;
    Uint32 cursor = *random;
    if (!e->step)
    {
        c->visible = 0; // tpage 0x1C, DRAW_UNK_100 | DRAW_HIDE
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_height = 0x50;
        next.hitbox_width = 0xc;
        next.facing_left = false;
        next.y = WithPixel(next.y, in.screen_y + 0x160);
        c->velocity_y = -0x60000;
        next.flags = 0x04800000U; // KEEP_ALIVE_OFFCAMERA | HAS_PRIMS
        c->angle = 0;
        c->spin = 0x400;
        out->sound = 0x661; // SFX_FIREBALL_SHOT_B
        next.step = 1;
        *e = next;
        *random = cursor;
        return true;
    }
    if (Word(Pixel(e->y) - in.screen_y) < -0x40)
    {
        e->removed = true;
        return true;
    }
    if (Word(Pixel(e->y) - in.screen_y) < 0)
        c->velocity_y = Add(c->velocity_y, -0x8000);
    next.y = Add(next.y, c->velocity_y);
    c->angle = Word(c->angle + 0x60);
    c->spin = Word(c->spin + 0x60);
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(in.executable, c->angle, &t))
        return false;
    const Sint16 x = Word(Pixel(next.x) - in.screen_x), y = Word(Pixel(next.y) - in.screen_y);
    const Sint32 transfer[3] = {Asr(-(t.cosine * 0x60), 12), y - 0x80,
                                Asr(t.sine * 0x60, 12) + 0x180};
    const Sint16 axis[3] = {0, c->spin, 0x40}, centre[4][3] = {{0}};
    SotnProjectedPoint p[4] = {{0}};
    Sint32 z = 0;
    if (!SotnShieldStar_Project(in.executable, axis, transfer, x, 120, 320, centre, p, &z))
        return false;
    next.hitbox_x = Word(Word(p[0].x) - x);
    next.hitbox_y = Word(Word(p[0].y) - y);
    c->priority = z < 16 ? 0x1b6 : z > 998 ? 0x10 : (Uint16)(0xd0 - (z - 0x50));
    c->visible = 0;
    for (Uint16 row = 0; row < SOTN_GIANT_CROSS_ROWS; ++row)
    {
        Sint16 m[4][3] = {{0}};
        if (!CrossVertices(in.dra, row, m))
            return false;
        // RTPT takes vectors 0, 1 and 3; RTPS then vector 2.
        const Sint16 order[4][3] = {{m[0][0], m[0][1], m[0][2]},
                                    {m[1][0], m[1][1], m[1][2]},
                                    {m[3][0], m[3][1], m[3][2]},
                                    {m[2][0], m[2][1], m[2][2]}};
        Sint32 depth = 0;
        if (!SotnShieldStar_Project(in.executable, axis, transfer, x, 120, 320, order, p, &depth))
            return false;
        const Sint64 nclip = (Sint64)p[0].x * p[1].y + (Sint64)p[1].x * p[2].y +
                             (Sint64)p[2].x * p[0].y - (Sint64)p[0].x * p[2].y -
                             (Sint64)p[1].x * p[0].y - (Sint64)p[2].x * p[1].y;
        if (nclip < 0)
            continue;
        for (Uint16 v = 0; v < 4; ++v)
        {
            c->x[row][v] = Word(p[v].x);
            c->y[row][v] = Word(p[v].y);
        }
        c->visible |= (Uint64)1 << row;
    }
    *e = next;
    *random = cursor;
    return true;
}
// EntitySubwpnCrashCrossParticles and func_80119E78. Each born spark takes three rand calls.
static void Sparks(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random)
{
    SotnCrossParticles *s = &e->sparks;
    if (!e->step)
    {
        e->flags = 0x04800000U; // KEEP_ALIVE_OFFCAMERA | HAS_PRIMS
        s->timer = 192;
        e->step = 1;
        return;
    }
    s->timer = Word(s->timer - 1);
    if (!s->timer)
    {
        e->removed = true;
        return;
    }
    if (s->timer > 8 && !(s->timer & 3))
        for (Uint16 i = 0; i < SOTN_CROSS_PARTICLE_ROWS; ++i)
            if (!s->rows[i].active)
            {
                s->rows[i].active = 1;
                s->rows[i].born = 0;
                break;
            }
    for (Uint16 i = 0; i < SOTN_CROSS_PARTICLE_ROWS; ++i)
    {
        SotnCrossParticle *p = &s->rows[i];
        if (!p->active)
            continue;
        if (!p->born)
        {
            const Uint16 roll = SotnCombat_Random(random) & 0x3f;
            p->x = (Uint8)(SotnCombat_Random(random) % 237 + 9);
            p->y = (Uint8)(0xf0 - (SotnCombat_Random(random) & 0x20));
            p->frame = p->parity = 0;
            p->priority = (Uint16)(in.player_priority + roll - 0x20);
            p->visible = true;
            p->speed = (Uint8)((roll >> 2) + 4);
            ++p->born;
        }
        else
        {
            p->y = (Uint8)(p->y - p->speed);
            if (p->frame >= 6 || p->y < 24)
            {
                p->visible = false;
                p->active = 0;
            }
        }
        if (p->active)
        {
            p->shown = p->frame;
            p->placed = p->y;
            ++p->parity;
            if (!(p->parity & 1))
                ++p->frame;
        }
    }
}
// EntitySubwpnKnife: eight one-pixel steps per update with a probe eight pixels ahead; a wall
// (or hitFlags 2) bounces it back spinning, an enemy hit holds it four updates.
static bool Dagger(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                   SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnDagger *d = &next.dagger;
    Uint32 cursor = *random;
    switch (e->step)
    {
    case 0:
        next.flags = 0x08800000U; // FLAG_POS_CAMERA_LOCKED | FLAG_HAS_PRIMS
        next.facing_left = in.player_facing_left;
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_width = 4;
        next.hitbox_height = 2;
        next.hitbox_x = 4;
        next.hitbox_y = 0;
        if (!(in.player_status & 0x20U)) // PLAYER_STATUS_CROUCH
            next.y = WithPixel(next.y, Pixel(next.y) - 9);
        d->row_priority = (Uint16)(in.player_priority + 2U);
        d->quad_mode = 0x0a; // DRAW_HIDE | DRAW_UNK02
        d->line_mode = 0x3b; // DRAW_TPAGE2 | DRAW_TPAGE | DRAW_HIDE | DRAW_UNK02 | DRAW_TRANSP
        next.velocity_x = next.facing_left ? -0x80000 : 0x80000;
        out->sound = 0x60c; // SFX_WEAPON_SWISH_C
        out->use_ticks = 4;
        next.step = 1;
        break;
    case 1: {
        d->timer = Word(d->timer + 1);
        const Sint32 probe = next.velocity_x > 0 ? 8 : -8;
        if (in.hit_flags == 1)
        {
            d->timer = 4;
            next.step = 3;
            out->disable_attack = true;
            break;
        }
        for (Uint16 i = 0; i < 8; ++i)
        {
            next.x = WithPixel(next.x, Pixel(next.x) + (next.velocity_x > 0 ? 1 : -1));
            SotnCollider col = {0};
            if (!SotnCollision_Query(&in.collision, Pixel(next.x) + probe, Pixel(next.y), &col))
                return false;
            if (in.hit_flags == 2 || (col.effects & SOTN_EFFECT_BLOCK))
            {
                d->timer = 64;
                next.velocity_x = -Asr(next.velocity_x, 3);
                d->velocity_y = -0x28000;
                out->disable_attack = true;
                out->factories = 1;
                out->factory_packed[0] = SOTN_REBOUND_STONE_SPARKS;
                out->factory_x = WithPixel(next.x, Pixel(next.x) + probe);
                out->factory_y = next.y;
                out->sound = 0x6a4; // SFX_UI_SUBWEAPON_TINK
                next.step = 2;
                *e = next;
                *random = cursor;
                return true;
            }
        }
        const Sint32 x = Pixel(next.x), y = Pixel(next.y);
        const Sint32 ox = next.facing_left ? -12 : 12, oy = next.facing_left ? -8 : 8;
        const Sint32 qx[4] = {x - ox, x + ox, x - ox, x + ox}, qy[4] = {y - 4, y - 4, y + 4, y + 4};
        SDL_memcpy(d->x, qx, sizeof(qx));
        SDL_memcpy(d->y, qy, sizeof(qy));
        d->quad_mode &= 0xfff7U;
        d->line_x[0] = x - oy;
        d->line_y[0] = d->line_y[1] = y - 1;
        d->line_x[1] = x - ox * (d->timer / 2);
        d->line_mode &= 0xfff7U;
        break;
    }
    case 2: {
        d->timer = Word(d->timer - 1);
        if (!d->timer)
        {
            next.removed = true;
            break;
        }
        if (d->timer == 0x20)
        {
            d->quad_mode |= 0x35; // DRAW_TPAGE2 | DRAW_TPAGE | DRAW_COLORS | DRAW_TRANSP
            d->grey = 0x60;
        }
        next.x = Add(next.x, next.velocity_x);
        next.y = Add(next.y, d->velocity_y);
        d->velocity_y = Add(d->velocity_y, 0x2000);
        const Sint32 x = Pixel(next.x), y = Pixel(next.y);
        Sint16 angle[4] = {0x800 - 0xd2, 0xd2, 0x800 + 0xd2, -0xd2};
        if (next.facing_left)
        {
            const Sint16 flipped[4] = {0xd2, 0x800 - 0xd2, -0xd2, 0x800 + 0xd2};
            SDL_memcpy(angle, flipped, sizeof(angle));
            d->rotate = Word(d->rotate + 0x80);
        }
        else
            d->rotate = Word(d->rotate - 0x80);
        for (Uint16 v = 0; v < 4; ++v)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, Word(angle[v] + d->rotate), &t))
                return false;
            d->x[v] = x + Word(Asr(t.cosine * 0xca0, 20));
            d->y[v] = y - Word(Asr(-(t.sine * 0xca0), 20));
        }
        if (d->timer < 0x21)
            d->grey = (Uint8)(d->grey - 2);
        d->quad_mode &= 0xfff7U;
        d->line_mode |= 8U;
        break;
    }
    default:
        d->timer = Word(d->timer - 1);
        next.removed = !d->timer;
        break;
    }
    *e = next;
    *random = cursor;
    return true;
}
bool SotnSubweaponEntity_AxeTint(SotnDataView dra, Uint16 index, Uint8 rgb[3])
{
    const size_t at = 0x10628U + (size_t)index * 4U;
    if (!rgb || index >= 10 || !dra.bytes || dra.size < at + 3U)
        return SDL_SetError("Axe afterimage colour %u is outside the private DRA table", index);
    SDL_memcpy(rgb, dra.bytes + at, 3);
    return true;
}
// EntitySubwpnThrownAxe. Every update, including birth, ends with the afterimage pass: a new
// afterimage every third update copies row 0's words from that pass, then fades over ten.
static bool Axe(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnAxe *a = &next.axe;
    Uint32 cursor = *random;
    switch (e->step)
    {
    case 0:
        // FLAG_POS_CAMERA_LOCKED | KEEP_ALIVE_OFFCAMERA | HAS_PRIMS | FLAG_UNK_20000
        next.flags = 0x0c820000U;
        next.facing_left = !in.player_facing_left;
        next.velocity_x = next.facing_left ? 0x20000 : -0x20000; // SetSpeedX(FIX(-2))
        a->velocity_y = -0x60000;
        a->angle = next.facing_left ? 0x400 : 0xc00;
        if (!(in.player_status & 0x20U)) // PLAYER_STATUS_CROUCH
            next.y = WithPixel(next.y, Pixel(next.y) - 12);
        a->row_priority = (Uint16)(in.player_priority - 2U);
        a->visible = 0;
        SDL_zeroa(a->shade);
        SDL_zeroa(a->active);
        SDL_zeroa(a->fresh);
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_width = next.hitbox_height = 12;
        out->sound = 0x60c; // SFX_WEAPON_SWISH_C
        out->use_ticks = 4;
        a->volume = 0x7f;
        next.step = 1;
        break;
    case 1:
        a->angle = Word(a->angle + (next.facing_left ? -0x80 : 0x80));
        if (!(a->angle & 0x3ff))
        {
            out->sound = 0x60c;
            out->volume_sound = true;
            out->volume = (Uint16)a->volume;
            a->volume -= 8;
            if (a->volume < 0)
                a->volume = 0;
        }
        a->velocity_y = SDL_min(Add(a->velocity_y, 0x4400), 0x80000);
        next.x = Add(next.x, next.velocity_x);
        next.y = Add(next.y, a->velocity_y);
        if (Word(Pixel(next.y) - in.screen_y) > 0x100)
        {
            next.removed = true;
            break;
        }
        if (in.hit_flags == 2)
        {
            out->disable_attack = true;
            next.velocity_x = -(next.velocity_x / 2);
            a->velocity_y = -0x30000;
            next.step = 2;
        }
        break;
    default:
        a->angle = Word(a->angle + (next.facing_left ? 0xc0 : -0xc0));
        a->velocity_y = SDL_min(Add(a->velocity_y, 0x2400), 0x80000);
        next.x = Add(next.x, next.velocity_x);
        next.y = Add(next.y, a->velocity_y);
        next.removed = Word(Pixel(next.y) - in.screen_y) > 0x100;
        break;
    }
    if (next.removed)
    {
        *e = next;
        *random = cursor;
        return true;
    }
    if (!a->pose_timer)
    {
        const Uint16 pose = a->pose & 3U;
        a->shade[pose] = 0;
        a->active[pose] = a->fresh[pose] = 1;
        a->pose = (Uint16)((pose + 1U) & 3U);
        a->pose_timer = 2;
    }
    else
        a->pose_timer = Word(a->pose_timer - 1);
    a->palette = (Uint16)(((in.game_timer >> 1) & 1U) + 0x1abU);
    static const Sint16 right[4] = {0x2a0, 0x560, 0xd60, 0xaa0},
                        left[4] = {0x560, 0x2a0, 0xaa0, 0xd60};
    const Sint16 x = Word(Pixel(next.x) - in.screen_x), y = Word(Pixel(next.y) - in.screen_y);
    for (Uint16 v = 0; v < 4; ++v)
    {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, Word((next.facing_left ? left : right)[v] + a->angle),
                           &t))
            return false;
        a->x[0][v] = Word(x + Asr(t.cosine * 16 * 21, 16));
        a->y[0][v] = Word(y - Asr(t.sine * 16 * 21, 16));
    }
    a->visible |= 1U;
    for (Uint16 i = 1; i < SOTN_AXE_ROWS; ++i)
    {
        if (!a->active[i - 1U])
            continue;
        if (a->fresh[i - 1U])
        {
            a->fresh[i - 1U] = 0;
            SDL_memcpy(a->x[i], a->x[0], sizeof(a->x[i]));
            SDL_memcpy(a->y[i], a->y[0], sizeof(a->y[i]));
        }
        const Uint8 shade = a->shade[i - 1U]++;
        if (shade < 10)
        {
            a->tint[i - 1U] = shade;
            a->visible |= (Uint8)(1U << i);
        }
        else
        {
            a->active[i - 1U] = 0;
            a->visible &= (Uint8) ~(1U << i);
        }
    }
    *e = next;
    *random = cursor;
    return true;
}
bool SotnSubweaponEntity_GlassShard(SotnDataView dra, Uint16 shard, Sint16 out[6])
{
    const size_t at = 0x10658U + (size_t)shard * 12U;
    if (!out || shard >= 4 || !dra.bytes || dra.size < at + 12U)
        return SDL_SetError("Holy water shard %u is outside the private DRA table", shard);
    for (Uint16 i = 0; i < 6; ++i)
        out[i] = (Sint16)(Uint16)(dra.bytes[at + i * 2U] | dra.bytes[at + i * 2U + 1U] << 8);
    return true;
}
bool SotnSubweaponEntity_FlameTexture(SotnDataView dra, Uint16 frame, Uint8 *u, Uint8 *v)
{
    const size_t at = 0x10688U + (size_t)frame * 8U;
    if (!u || !v || frame >= 8 || !dra.bytes || dra.size < at + 8U)
        return SDL_SetError("Holy water flame frame %u is outside the private DRA table", frame);
    *u = dra.bytes[at];
    *v = dra.bytes[at + 4U];
    return true;
}
// CheckHolyWaterCollision(0, 0): the floor below the bottle; lands it on the surface and returns
// the masked effects (bit0 = landed).
static bool HolyFloor(SotnCollisionMap map, SotnSubweaponEntity *n, Uint32 *result)
{
    const Sint32 x = Pixel(n->x), y = Pixel(n->y);
    SotnCollider a = {0}, b = {0};
    if (!SotnCollision_Query(&map, x, y, &a) || !SotnCollision_Query(&map, x, y - 1 + a.top, &b))
        return false;
    const Uint32 effects = a.effects & 0xf801U, kind = effects & 0x8801U;
    const Sint32 landed = y + a.top;
    *result = 0;
    if (kind == 1 || kind == 0x801)
    {
        if (!(b.effects & 1U))
        {
            n->y = WithPixel(n->y, landed);
            *result = 1;
        }
        else if ((b.effects & 0x8001U) == 0x8001U)
        {
            n->y = WithPixel(n->y, landed - 1 + b.top);
            *result = b.effects & 0xf001U;
        }
    }
    else if ((effects & 0x8001U) == 0x8001U)
    {
        n->y = WithPixel(n->y, landed);
        *result = effects & 0xf001U;
    }
    return true;
}
// EntitySubwpnHolyWater: the sprite arcs until func_80125B6C or CheckHolyWaterCollision lands
// it, breaks (FACTORY(59) glass), releases four flames (one every fourth update, FACTORY(28)
// with the D_8013841C counter) and retires four updates later. Its 4x4 hitbox stays live.
static bool HolyWater(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                      SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnHolyWater *h = &next.holy;
    Uint32 cursor = *random;
    switch (e->step)
    {
    case 0:
        next.flags = 0x08000000U; // FLAG_POS_CAMERA_LOCKED
        h->shown = true;
        next.priority = (Uint16)(in.player_priority - 2U);
        next.y = WithPixel(next.y, Pixel(next.y) + 8);
        next.velocity_x = next.facing_left ? -0x14000 : 0x14000; // SetSpeedX(FIX(1.25))
        h->velocity_y = -0x32000;
        if (!in.properties(in.context, &cursor))
            return false;
        h->crash = (Uint8)in.crash;
        next.hitbox_width = next.hitbox_height = 4;
        out->use_ticks = 4;
        next.step = 1;
        break;
    case 1: {
        next.y = Add(next.y, h->velocity_y);
        if (h->velocity_y < 0x40000)
            h->velocity_y = Add(h->velocity_y, 0x3800);
        Uint32 floor = 0;
        if (!HolyFloor(in.collision, &next, &floor))
            return false;
        next.x = Add(next.x, next.velocity_x);
        bool wall = false;
        if (next.velocity_x)
        {
            SotnCollider c = {0};
            if (!SotnCollision_Query(&in.collision, Pixel(next.x) + (next.velocity_x < 0 ? -4 : 4),
                                     Pixel(next.y) - 7, &c))
                return false;
            if (c.effects & SOTN_EFFECT_SIDE)
            {
                next.x = Pixel(next.x) + (next.velocity_x > 0 ? c.left : c.right);
                next.x = WithPixel(0, next.x);
                wall = true;
            }
        }
        if ((floor & 1U) || wall)
        {
            out->sound = 0x69a; // SFX_ALU_HOLY_WATER_ATTACK
            out->factories = 1;
            out->factory_packed[0] = SOTN_HOLY_WATER_GLASS_BLUEPRINT;
            out->factory_x = next.x;
            out->factory_y = next.y;
            h->shown = false;
            h->timer = 16;
            next.step = 2;
        }
        break;
    }
    case 2:
        if (!(h->timer & 3))
        {
            out->factories = 1;
            out->factory_packed[0] = SOTN_HOLY_WATER_FLAME_BLUEPRINT | (in.flame_counter & 0xffffU)
                                                                           << 16;
            out->factory_argument[0] = (Uint16)(h->crash << 9);
            out->factory_x = next.x;
            out->factory_y = next.y;
            out->flame_counter_step = true;
        }
        h->timer = Word(h->timer - 1);
        if (!h->timer)
        {
            h->timer = 4;
            next.step = 3;
        }
        break;
    default:
        h->timer = Word(h->timer - 1);
        next.removed = !h->timer;
        break;
    }
    *e = next;
    *random = cursor;
    return true;
}
// EntitySubwpnHolyWaterBreakGlass: 48 rand calls at birth (two per carrier, four per shard),
// then twenty updates of falling shards.
static bool HolyGlass(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random)
{
    SotnSubweaponEntity next = *e;
    SotnHolyGlass *g = &next.glass;
    if (!e->step)
    {
        for (Uint16 i = 0; i < 8; ++i)
        {
            g->x[i] = Pixel(next.x) * 65536;
            g->y[i] = Pixel(next.y) * 65536;
            g->velocity_x[i] = (Sint32)(SotnCombat_Random(random) & 0x3fffU) + 0x4000;
            if (i & 1U)
                g->velocity_x[i] = -g->velocity_x[i];
            g->velocity_y[i] = -((Sint32)(SotnCombat_Random(random) & 0x7fffU) * 2 + 0x28000);
        }
        for (Uint16 i = 0; i < 8; ++i)
        {
            g->red[i] = (Uint8)((SotnCombat_Random(random) & 0xfU) + 0x30);
            g->blue[i] = (Uint8)((SotnCombat_Random(random) & 0x7fU) + 0x80);
            g->green[i] = (Uint8)((SotnCombat_Random(random) & 0x1fU) + 0x30);
            g->draw_mode[i] = (SotnCombat_Random(random) & 1U) ? 0x37 : 0x06;
        }
        g->row_priority = (Uint16)(in.player_priority + 2U);
        next.flags = 0x08800000U; // FLAG_POS_CAMERA_LOCKED | FLAG_HAS_PRIMS
        g->timer = 20;
        next.step = 1;
        *e = next;
        return true;
    }
    g->timer = Word(g->timer - 1);
    if (!g->timer)
    {
        next.removed = true;
        *e = next;
        return true;
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        g->x[i] = Add(g->x[i], g->velocity_x[i]);
        g->y[i] = Add(g->y[i], g->velocity_y[i]);
        g->velocity_y[i] = Add(g->velocity_y[i], 0x4800);
    }
    *e = next;
    return true;
}
// EntitySubwpnHolyWaterFlame: three colour rolls before AllocPrimitives(GT4,4) (a failed
// allocation still consumes them), func_8011A290, FACTORY(4,7) smoke ten pixels up and the
// height roll; then a waving column in screen words, its hitbox live every fourth update.
static bool HolyFlame(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                      SotnSubweaponEvents *out)
{
    SotnSubweaponEntity next = *e;
    SotnHolyFlame *f = &next.flame;
    Uint32 cursor = *random;
    if (!e->step)
    {
        f->red = (Uint8)((SotnCombat_Random(&cursor) & 0x1fU) + 0x40);
        f->green = (Uint8)((SotnCombat_Random(&cursor) & 0x1fU) + 0x80);
        f->blue = (Uint8)((SotnCombat_Random(&cursor) & 0x1fU) + 0x60);
        if (!in.allocated)
        {
            next.removed = true;
            *e = next;
            *random = cursor;
            return true;
        }
        f->row_priority = (Uint16)(in.player_priority + 2U);
        next.flags = 0x08800000U; // FLAG_POS_CAMERA_LOCKED | FLAG_HAS_PRIMS
        if (!in.properties(in.context, &cursor))
            return false;
        next.hitbox_width = 4;
        out->factories = 1;
        out->factory_packed[0] = SOTN_HOLY_WATER_SMOKE;
        out->factory_x = next.x;
        out->factory_y = WithPixel(next.y, Pixel(next.y) - 10);
        f->timer = 0x50;
        f->height = (Sint16)((SotnCombat_Random(&cursor) & 0xfU) + 0x12);
        next.step = 1;
        *e = next;
        *random = cursor;
        return true;
    }
    const Sint16 x = Word(Pixel(next.x) - in.screen_x), y = Word(Pixel(next.y) - in.screen_y);
    Sint16 px[5] = {0}, py[5] = {0};
    const Sint16 phase = f->angle;
    f->angle = Word(f->angle + 0x180);
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, Word(phase + (i << 10)), &t))
            return false;
        px[i] = Word(x + Asr(t.sine, 10));
    }
    px[0] = px[4] = x;
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(in.executable, Word((f->timer - 0x10) * 64 + 0xc00), &t))
        return false;
    const Sint16 height = Word(Asr(t.sine, 8) + f->height);
    py[0] = Word(y - height);
    py[4] = y;
    py[2] = Word((py[0] + py[4]) / 2);
    py[1] = Word((py[0] + py[2]) / 2);
    py[3] = Word((py[2] + py[4]) / 2);
    out->attack_set = true;
    out->attack_on = !(f->timer & 3);
    f->timer = Word(f->timer - 1);
    if (f->timer < 0x11)
    {
        next.removed = true;
        *e = next;
        return true;
    }
    if (f->timer < 0x29)
    {
        if (f->green > 16)
            f->green = (Uint8)(f->green - 5);
        if (f->blue > 16)
            f->blue = (Uint8)(f->blue - 5);
        if (f->red > 16)
            f->red = (Uint8)(f->red - 5);
    }
    for (Uint16 i = 0; i < 5; ++i)
    {
        f->x[i] = px[i] + in.screen_x;
        f->y[i] = py[i] + in.screen_y;
    }
    f->frame = (Uint8)(in.game_timer & 7U);
    f->shown = py[4] - py[0] >= 7;
    next.hitbox_height = (Uint8)Asr(height, 1);
    next.hitbox_y = Word(Asr(-height, 1));
    *e = next;
    *random = cursor;
    return true;
}
Uint16 SotnSubweaponEntity_Rows(Uint16 id)
{
    return id == SOTN_HOLY_WATER_FLAME                            ? SOTN_HOLY_WATER_FLAME_ROWS
           : id == SOTN_HOLY_WATER_GLASS                          ? SOTN_HOLY_WATER_GLASS_ROWS
           : id == SOTN_SUBWEAPON_AXE                             ? SOTN_AXE_ROWS
           : id == SOTN_SUBWEAPON_DAGGER                          ? SOTN_DAGGER_ROWS
           : id == SOTN_SUBWEAPON_VIBHUTI                         ? SOTN_VIBHUTI_ROWS
           : id == SOTN_SUBWEAPON_REBOUND_STONE                   ? SOTN_REBOUND_STONE_LINES
           : id == SOTN_SUBWEAPON_AGUNEA                          ? SOTN_AGUNEA_ROWS
           : id == SOTN_AGUNEA_BOLT                               ? SOTN_AGUNEA_BOLT_ROWS
           : id == SOTN_SUBWEAPON_BIBLE || id == SOTN_BIBLE_TRAIL ? 1
           : id == SOTN_SUBWEAPON_CROSS                           ? 1
           : id == SOTN_GIANT_CROSS                               ? SOTN_GIANT_CROSS_ROWS
           : id == SOTN_CROSS_PARTICLES                           ? SOTN_CROSS_PARTICLE_ROWS
                                                                  : 0;
}
bool SotnSubweaponEntity_Known(Uint16 id)
{
    return id == SOTN_SUBWEAPON_HOLY_WATER || SotnSubweaponEntity_Rows(id) != 0;
}
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                              SotnSubweaponEvents *events)
{
    const bool vibhuti = e && e->id == SOTN_SUBWEAPON_VIBHUTI,
               stone = e && e->id == SOTN_SUBWEAPON_REBOUND_STONE,
               agunea = e && e->id == SOTN_SUBWEAPON_AGUNEA, bolt = e && e->id == SOTN_AGUNEA_BOLT,
               bible = e && e->id == SOTN_SUBWEAPON_BIBLE, trail = e && e->id == SOTN_BIBLE_TRAIL,
               beam = e && e->id == SOTN_SUBWEAPON_CROSS, cross = e && e->id == SOTN_GIANT_CROSS,
               sparks = e && e->id == SOTN_CROSS_PARTICLES,
               dagger = e && e->id == SOTN_SUBWEAPON_DAGGER, axe = e && e->id == SOTN_SUBWEAPON_AXE,
               holy = e && e->id == SOTN_SUBWEAPON_HOLY_WATER,
               glass = e && e->id == SOTN_HOLY_WATER_GLASS,
               flame = e && e->id == SOTN_HOLY_WATER_FLAME;
    if (!e || !random || !events || e->removed ||
        (!vibhuti && !stone && !agunea && !bolt && !bible && !trail && !beam && !cross && !sparks &&
         !dagger && !axe && !holy && !glass && !flame) ||
        (holy && e->step > 3) || (glass && (e->step > 1 || (e->step && e->glass.timer < 1))) ||
        (flame && (e->step > 1 || (e->step && e->flame.timer < 0x11))) || (dagger && e->step > 3) ||
        (axe && (e->step > 2 || e->axe.pose > 3)) ||
        (!in.properties && !bolt && !trail && !sparks && !glass) || (beam && e->step > 4) ||
        (cross && e->step > 1) ||
        (sparks && (e->step > 1 || (e->step && (e->sparks.timer < 1 || e->sparks.timer > 192)))) ||
        (bible && (e->step > 4 || e->bible.mode < 0 || e->bible.mode > 2)) ||
        (trail && e->step > 2) ||
        (vibhuti && (e->step > 1 || (e->step && (!e->timer || e->timer > 0x80)))) ||
        (stone && (e->step > 2 || (e->step && (e->stone.life < 1 || e->stone.life > 0x40 ||
                                               e->stone.bounces < 0 || e->stone.bounces > 15)))) ||
        (agunea && (e->step > 4 || e->agunea.target >= SOTN_STAGE_TARGETS)) ||
        (bolt &&
         (e->step > 3 || e->bolt.end >= SOTN_AGUNEA_BOLT_ROWS || (e->step && e->bolt.grey < 16))) ||
        in.hit_flags > 3 || in.hit_target >= SOTN_STAGE_TARGETS)
        return SDL_SetError("Subweapon callback requires its actual live entity and outputs");
    *events = (SotnSubweaponEvents){0};
    events->reset_afterimages = beam; // func_8010DFF0(1, 1) precedes the allocation.
    // E_SUBWPN_AGUNEA retires during any transformation (and status 0x10000) before allocating.
    if (agunea && (in.player_status & 0x10007U))
    {
        e->removed = true;
        return true;
    }
    // The flame rolls its colours before AllocPrimitives; the bottle allocates no rows.
    if (flame)
        return HolyFlame(e, in, random, events);
    if (holy)
        return HolyWater(e, in, random, events);
    if (!e->step && !in.allocated)
    {
        e->removed = true; // Row allocation failure: DestroyEntity before any RNG.
        return true;
    }
    if (trail)
    {
        BibleTrail(e);
        return true;
    }
    if (sparks)
    {
        Sparks(e, in, random);
        return true;
    }
    if (glass)
        return HolyGlass(e, in, random);
    if (beam)
        return Beam(e, in, random, events);
    if (dagger)
        return Dagger(e, in, random, events);
    if (axe)
        return Axe(e, in, random, events);
    if (cross)
        return GiantCross(e, in, random, events);
    if (bible)
        return Bible(e, in, random, events);
    if (agunea)
        return Agunea(e, in, random, events);
    if (bolt)
        return Bolt(e, in, random);
    if (stone)
        return Stone(e, in, random, events);
    if (e->step)
        return Fly(e, in);
    return Birth(e, in, random, events);
}
