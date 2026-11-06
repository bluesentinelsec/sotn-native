// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/84B88.c:EntitySubwpnThrownVibhuti, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
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
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *e, SotnSubweaponInput in, Uint32 *random,
                              SotnSubweaponEvents *events)
{
    if (!e || !random || !events || e->removed || e->id != SOTN_SUBWEAPON_VIBHUTI || e->step > 1 ||
        (e->step && (!e->timer || e->timer > 0x80)) || in.hit_flags > 3)
        return SDL_SetError("Vibhuti callback requires its actual live entity and outputs");
    *events = (SotnSubweaponEvents){0};
    if (e->step)
        return Fly(e, in);
    if (!in.allocated)
    {
        e->removed = true; // func_800EDB58 failure: DestroyEntity before any RNG.
        return true;
    }
    return Birth(e, in, random, events);
}
