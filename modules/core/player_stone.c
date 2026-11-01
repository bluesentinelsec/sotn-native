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
static Sint32 Add(Sint32 value, Sint32 amount)
{
    const Uint32 bits = (Uint32)value + (Uint32)amount;
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
static void Palette(SotnPlayer *p, Uint16 palette)
{
    p->stone.palette = palette;
    p->palette = (Uint16)(0x8000U | palette);
}
static void Wiggle(SotnPlayer *p)
{
    if (p->animation.remaining < 0)
    {
        p->substep = 2;
        p->form_rotated = false;
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
    p->form_rotated = true;
    p->stone.pivot = p->stone.rotation ? 20 : 24;
    p->draw_pivot_x = 0;
    p->draw_pivot_y = p->stone.pivot;
}
void SotnPlayer_StoneTick(SotnPlayer *p)
{
    const bool impact = p->stone.impact;
    p->stone.impact = false;
    if (p->substep == 1)
    {
        SotnPlayer_Protect(p, 4);
        Palette(p, 0x161);
        if (SotnPlayer_StoneLanding(p))
        {
            p->step = SOTN_PLAYER_STONE;
            p->vx = p->vy = 0;
            SotnPlayer_ReactionShake(p, 1);
            SotnPlayer_ReactionSound(p, 0x644);
            SotnPlayer_ReactionFactory(p, 39);
            Uint16 variant = (Uint16)(SotnCombat_Random(&p->damage_random) % 64);
            if (!p->hp)
            {
                p->stone.death_wait = 32;
                variant = 2;
            }
            if (variant)
            {
                variant &= 1;
                Palette(p, (Uint16)(0x19e + variant));
                Animate(p, (Uint16)(0x38 + variant));
                SotnPlayer_ReactionFactory(p, 0x30010U);
            }
            else
            {
                Palette(p, 0x19e);
                Animate(p, 0x3a);
                SotnPlayer_ReactionFactory(p, 0x7001dU);
            }
            p->substep = 2;
        }
    }
    else if (p->substep == 2)
    {
        if (!(p->contact & 1))
        {
            p->vy = SDL_min(7 * 65536, Add(p->vy, p->water.feet > 40 ? 2816 : 11264));
            SotnPlayer_Protect(p, 4);
            goto done;
        }
        p->vy = 0;
        if (!(p->previous_contact & 1))
        {
            SotnPlayer_ReactionShake(p, 1);
            SotnPlayer_ReactionSound(p, 0x644);
        }
        if (!p->hp)
        {
            p->stone.death_wait = Add(p->stone.death_wait, -1);
            if (!p->stone.death_wait)
            {
                p->step = SOTN_PLAYER_KILL;
                p->substep = 16;
                p->death.phase = 16;
                SotnPlayer_ReactionSound(p, 0x6f6);
            }
            SotnPlayer_Protect(p, 4);
            goto done;
        }
        const Uint16 directions =
            SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT;
        if ((p->tapped & directions) || impact)
        {
            p->tapped |= directions;
            p->stone.shakes = (Uint16)(p->stone.shakes - 1U);
            p->animation.remaining = 16;
            SotnPlayer_ReactionSound(p, 0x608);
            if (!p->stone.shakes)
            {
                Animate(p, 0x3b);
                // SetPlayerAnim updates the live animation before the original comparison.
                SotnPlayer_ReactionFactory(p, p->animation.id != 0x3a ? 0x30010U : 32U);
                p->step = SOTN_PLAYER_HIT;
                p->substep = 6;
                p->stone = (SotnPlayerStone){0};
                p->palette = 0x8100;
                SotnPlayer_ReactionSound(p, 0x6e7);
                goto done;
            }
            SotnPlayer_Protect(p, 28);
            p->substep = 3;
            SotnPlayer_ReactionFactory(p, p->animation.id != 0x3a ? 0x3000dU : 0x8000dU);
            SotnPlayer_ReactionFactory(p, p->animation.id != 0x3a ? 0x3001fU : 0x7001eU);
        }
        Palette(p, (Uint16)(0x19e + ((p->animation.id - 0x38U) & 1U)));
    }
    else if (p->substep == 3)
        Wiggle(p);
    else
    {
        SDL_SetError("Petrified player entered invalid substate %u", p->substep);
        p->failed = true;
    }
done:
    if (p->animation.id == 0x3a)
        SotnPlayer_Protect(p, 4);
}
void SotnPlayer_StoneDeathTick(SotnPlayer *p)
{
    // PlayerStepKill writes DRAW_COLORS (4) to entity flags: here that bit is ROTATE.
    p->form_rotated = true;
    if (p->death.phase == 16)
    {
        p->death.timer = 80;
        p->death.phase = 17;
    }
    else if (p->death.phase == 17)
    {
        p->stone.shakes = 5;
        if (p->death.timer % 16 == 7)
        {
            p->tapped = SOTN_BUTTON_UP;
            SotnPlayer_ReactionSound(p, 0x608);
        }
        if (p->death.timer && --p->death.timer == 0)
        {
            p->stone.hidden = true;
            Palette(p, 0x10d);
            Animate(p, 0x3e);
            SotnPlayer_ReactionFactory(p, 0x30010U);
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
    p->substep = p->death.phase;
    if (p->vx > 0)
        p->vx = SDL_max(0, p->vx - 1024);
    else if (p->vx < 0)
        p->vx = SDL_min(0, p->vx + 1024);
    if (p->animation.pose >= 15)
    {
        if (p->animation.pose == 22 && p->animation.remaining == 1)
            p->stone.rotation = (Sint16)(p->stone.rotation - 256);
        p->stone.rotation = (Sint16)SDL_max(-640, p->stone.rotation - 6);
    }
}
bool SotnPlayer_Petrify(SotnPlayer *p, Uint16 amount, Sint32 source_x, Sint32 source_velocity)
{
    if (p == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * 65536 || source_velocity > 16 * 65536)
        return SDL_SetError("Petrification requires bounded damage and contact coordinates");
    if (!p->hp || SotnPlayer_Protected(p) || p->step == SOTN_PLAYER_HIT ||
        SotnPlayerStep_IsDead(p->step) || p->step == SOTN_PLAYER_BOSS_GRAB ||
        p->grab.state != SOTN_GRAB_NONE)
        return true;
    SotnPlayer next = *p;
    next.gravity.upside_down = false;
    SotnPlayer_BatCancel(&next);
    SotnPlayer_WolfCancel(&next);
    next.hp = (Uint16)(amount >= next.hp ? 0 : next.hp - amount);
    if (next.step == SOTN_PLAYER_STONE)
    {
        next.stone.impact = true;
        if (!next.hp)
        {
            bool present = false;
            if (next.dissolve_present && !next.dissolve_present(next.dissolve_context, &present))
                return false;
            if (present)
            {
                next.step = SOTN_PLAYER_KILL;
                next.substep = next.death.phase = 16;
                next.sound = 0x6f6;
                next.vx = next.vy = 0;
                next.form_rotated = true;
            }
            else
            {
                SotnPlayer_StonePhysicalDeath(
                    &next, SotnPlayer_KnockRight(&next, source_x, source_velocity));
                if (next.failed ||
                    (!next.contact_dispatch && !SotnAnimation_Tick(next.dra, &next.animation)))
                    return false;
            }
        }
    }
    else
    {
        next.subweapon_cast = false;
        next.curse_failure = false;
        next.attack_mode = 0;
        next.attack = next.shield = (SotnPlayerAttack){0};
        next.shield_cursor = (SotnShield){0};
        next.movement_flags = 0;
        next.animation_bank = 1;
        next.form_rotated = false;
        next.bat.rotation = 0;
        next.vy = next.contact & 1 ? -4 * 65536 : -2 * 65536;
        const bool right = SotnPlayer_KnockRight(&next, source_x, source_velocity);
        next.vx = right ? 81920 : -81920;
        next.hurt.knock_right = right;
        next.step = SOTN_PLAYER_STONE;
        next.substep = 1;
        Animate(&next, (Uint16)(0x2e + SotnCombat_Random(&next.damage_random) % 3));
        const Sint32 adjustment =
            ((Sint32)next.attributes.total[1] + SotnCombat_Random(&next.damage_random) % 12 - 9) /
            10;
        next.stone = (SotnPlayerStone){
            .active = true, .palette = 0x161, .shakes = (Uint16)(8 - SDL_clamp(adjustment, 0, 4))};
        next.palette = 0x8161;
        next.timers[0] = next.timers[1] = next.timers[2] = 0;
        next.invincibility = 4;
        next.sound = 0x6ec;
        SotnPlayer_ReactionFactory(&next, 0x3002fU);
        if (next.tight_space)
            next.vy = 0;
        if (next.failed ||
            (!next.contact_dispatch && !SotnAnimation_Tick(next.dra, &next.animation)))
            return false;
    }
    *p = next;
    return true;
}
