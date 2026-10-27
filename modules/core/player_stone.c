// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected PlayerStepStoned / PlayerStepKill control flow, dra/71830.c, and
// GetStatusAilmentTimer, dra/5D5BC.c, at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Motion tables remain private.
#include "player_internal.h"
#include <sotn/combat.h>

static void Animate(SotnPlayer *p, Uint16 id)
{
    if (!SotnAnimation_Set(&p->animation, id))
        p->failed = true;
}
static void Gravity(SotnPlayer *p, bool apex)
{
    const Sint32 acceleration = apex && p->vy < 24576 && p->vy > -8192 ? 5632 : 11264;
    p->vy = SDL_min(7 * 65536, p->vy + acceleration);
    if ((p->contact & 2) && p->vy < -65536)
        p->vy = -65536;
}
static void Wiggle(SotnPlayer *p)
{
    if (p->animation.remaining < 0)
    {
        p->substep = 2;
        p->stone.rotation = 0;
        return;
    }
    const Uint32 index = (Uint32)p->animation.remaining;
    const size_t offset = 0xcf94 + index * 2U;
    if (index >= 16 || p->dra.bytes == NULL || offset > p->dra.size || p->dra.size - offset < 2)
    {
        SDL_SetError("Petrification wiggle cursor exceeds its private rotation table");
        p->failed = true;
        return;
    }
    const Uint16 bits = (Uint16)(p->dra.bytes[offset] | (Uint16)p->dra.bytes[offset + 1] << 8);
    const Sint32 angle = bits < 32768 ? bits : (Sint32)bits - 65536;
    if (angle < -512 || angle > 512)
    {
        SDL_SetError("Petrification wiggle rotation exceeds its original bounds");
        p->failed = true;
        return;
    }
    p->stone.rotation = (Sint16)(angle >= 0 ? angle / 16 : -((-angle + 15) / 16));
    p->stone.pivot = p->stone.rotation ? 20 : 24;
}
void SotnPlayer_StoneTick(SotnPlayer *p)
{
    const bool impact = p->stone.impact;
    p->stone.impact = false;
    if (p->substep == 1)
    {
        p->invincibility = (Uint16)SDL_max(p->invincibility, 4);
        Gravity(p, true);
        if (p->vy >= 0 && (p->contact & 1))
        {
            p->vx = p->vy = 0;
            p->sound = 0x644;
            Uint16 variant = (Uint16)(SotnCombat_Random(&p->damage_random) % 64);
            if (!p->hp)
            {
                p->stone.death_wait = 32;
                variant = 2;
            }
            if (variant)
            {
                variant &= 1;
                p->stone.palette = (Uint16)(0x19e + variant);
                Animate(p, (Uint16)(0x38 + variant));
            }
            else
            {
                p->stone.palette = 0x19e;
                Animate(p, 0x3a);
            }
            p->substep = 2;
        }
    }
    else if (p->substep == 2)
    {
        if (!(p->contact & 1))
        {
            Gravity(p, false);
            return;
        }
        p->vy = 0;
        if (!(p->previous_contact & 1))
            p->sound = 0x644;
        if (!p->hp)
        {
            if (p->stone.death_wait && --p->stone.death_wait == 0)
            {
                p->step = SOTN_PLAYER_KILL;
                p->death.phase = 16;
                p->sound = 0x6f6;
            }
            return;
        }
        const Uint16 directions =
            SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT;
        if ((p->tapped & directions) || impact)
        {
            if (p->stone.shakes)
                --p->stone.shakes;
            p->animation.remaining = 16;
            p->sound = 0x608;
            if (!p->stone.shakes)
            {
                Animate(p, 0x3b);
                p->step = SOTN_PLAYER_HIT;
                p->substep = 6;
                p->stone = (SotnPlayerStone){0};
                p->sound = 0x6e7;
                return;
            }
            p->invincibility = (Uint16)SDL_max(p->invincibility, 28);
            p->substep = 3;
        }
        p->stone.palette = (Uint16)(0x19e + ((p->animation.id - 0x38U) & 1U));
    }
    else if (p->substep == 3)
        Wiggle(p);
    else
    {
        SDL_SetError("Petrified player entered invalid substate %u", p->substep);
        p->failed = true;
    }
}
void SotnPlayer_StoneDeathTick(SotnPlayer *p)
{
    if (p->death.phase == 16)
    {
        p->death.timer = 80;
        p->death.phase = 17;
    }
    else if (p->death.phase == 17)
    {
        if (p->death.timer % 16 == 7)
            p->sound = 0x608;
        if (p->death.timer && --p->death.timer == 0)
        {
            p->stone.hidden = true;
            Animate(p, 0x3e);
            p->death.phase = 18;
        }
    }
    else if (p->death.phase == 18)
    {
        if (p->animation.remaining < 0)
            p->death.phase = 128;
    }
    else if (p->death.phase != 128)
    {
        SDL_SetError("Petrified death entered invalid phase %u", p->death.phase);
        p->failed = true;
    }
}
bool SotnPlayer_Petrify(SotnPlayer *p, Uint16 amount, Sint32 source_x, Sint32 source_velocity)
{
    if (p == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * 65536 || source_velocity > 16 * 65536)
        return SDL_SetError("Petrification requires bounded damage and contact coordinates");
    if (!p->hp || p->invincibility || p->step == SOTN_PLAYER_HIT || p->step == SOTN_PLAYER_KILL ||
        p->step == SOTN_PLAYER_BOSS_GRAB || p->grab.state != SOTN_GRAB_NONE)
        return true;
    SotnPlayer next = *p;
    SotnPlayer_GravityCancel(&next);
    SotnPlayer_BatCancel(&next);
    next.hp = (Uint16)(amount >= next.hp ? 0 : next.hp - amount);
    if (next.step == SOTN_PLAYER_STONE)
    {
        next.stone.impact = true;
        if (!next.hp)
        {
            next.step = SOTN_PLAYER_KILL;
            next.death.phase = 16;
            next.sound = 0x6f6;
        }
    }
    else
    {
        next.subweapon_cast = false;
        next.curse_failure = false;
        next.attack_mode = 0;
        next.attack = next.shield = (SotnPlayerAttack){0};
        next.movement_flags = 0;
        next.vy = next.contact & 1 ? -4 * 65536 : -2 * 65536;
        const Sint32 center = next.x.parts.high + (next.facing_left ? -next.animation.hitbox_x
                                                                    : next.animation.hitbox_x);
        const Sint32 delta = center - source_x;
        const bool right =
            SDL_abs(delta) < 16 && source_velocity ? source_velocity > 0 : delta >= 0;
        next.vx = right ? 81920 : -81920;
        next.step = SOTN_PLAYER_STONE;
        next.substep = 1;
        Animate(&next, (Uint16)(0x2e + SotnCombat_Random(&next.damage_random) % 3));
        const Sint32 adjustment =
            ((Sint32)next.attributes.total[1] + SotnCombat_Random(&next.damage_random) % 12 - 9) /
            10;
        next.stone = (SotnPlayerStone){
            .active = true, .palette = 0x161, .shakes = (Uint16)(8 - SDL_clamp(adjustment, 0, 4))};
        next.timers[0] = next.timers[1] = next.timers[2] = 0;
        next.invincibility = 4;
        next.sound = 0x6ec;
        if (next.failed || !SotnAnimation_Tick(next.dra, &next.animation))
            return false;
    }
    *p = next;
    return true;
}
