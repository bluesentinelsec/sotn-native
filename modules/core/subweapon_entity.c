// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/84B88.c:EntitySubwpnThrownVibhuti/EntitySubwpnReboundStone/EntitySubwpnAgunea/
// EntityAguneaHitEnemy and rebound_stone.h,
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
Uint16 SotnSubweaponEntity_Rows(Uint16 id)
{
    return id == SOTN_SUBWEAPON_VIBHUTI         ? SOTN_VIBHUTI_ROWS
           : id == SOTN_SUBWEAPON_REBOUND_STONE ? SOTN_REBOUND_STONE_LINES
           : id == SOTN_SUBWEAPON_AGUNEA        ? SOTN_AGUNEA_ROWS
           : id == SOTN_AGUNEA_BOLT             ? SOTN_AGUNEA_BOLT_ROWS
                                                : 0;
}
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                              SotnSubweaponEvents *events)
{
    const bool vibhuti = e && e->id == SOTN_SUBWEAPON_VIBHUTI,
               stone = e && e->id == SOTN_SUBWEAPON_REBOUND_STONE,
               agunea = e && e->id == SOTN_SUBWEAPON_AGUNEA, bolt = e && e->id == SOTN_AGUNEA_BOLT;
    if (!e || !random || !events || e->removed || (!vibhuti && !stone && !agunea && !bolt) ||
        (!in.properties && !bolt) ||
        (vibhuti && (e->step > 1 || (e->step && (!e->timer || e->timer > 0x80)))) ||
        (stone && (e->step > 2 || (e->step && (e->stone.life < 1 || e->stone.life > 0x40 ||
                                               e->stone.bounces < 0 || e->stone.bounces > 15)))) ||
        (agunea && (e->step > 4 || e->agunea.target >= SOTN_STAGE_TARGETS)) ||
        (bolt &&
         (e->step > 3 || e->bolt.end >= SOTN_AGUNEA_BOLT_ROWS || (e->step && e->bolt.grey < 16))) ||
        in.hit_flags > 3 || in.hit_target >= SOTN_STAGE_TARGETS)
        return SDL_SetError("Subweapon callback requires its actual live entity and outputs");
    *events = (SotnSubweaponEvents){0};
    // E_SUBWPN_AGUNEA retires during any transformation (and status 0x10000) before allocating.
    if (agunea && (in.player_status & 0x10007U))
    {
        e->removed = true;
        return true;
    }
    if (!e->step && !in.allocated)
    {
        e->removed = true; // Row allocation failure: DestroyEntity before any RNG.
        return true;
    }
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
