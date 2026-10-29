// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected Gravity Boots paths in dra/{704D0,6E42C,71830,7879C}.c,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork stays private.
#include "player_internal.h"

static void Animate(SotnPlayer *p, Uint16 id)
{
    if (!SotnAnimation_Set(&p->animation, id))
        p->failed = true;
}

void SotnPlayer_GravityInput(SotnPlayer *p)
{
    switch (p->gravity_combo)
    {
    case 0:
        if ((p->tapped & SOTN_BUTTON_DOWN) && p->held == 0)
        {
            p->gravity_combo_timer = 16;
            p->gravity_combo = 1;
        }
        break;
    case 1:
        if (p->tapped & SOTN_BUTTON_UP)
        {
            p->gravity_combo_timer = 16;
            p->gravity_combo = 2;
        }
        else
        {
            p->gravity_combo_timer = (Uint16)(p->gravity_combo_timer - 1U);
            if (p->gravity_combo_timer == 0)
                p->gravity_combo = 0;
        }
        break;
    case 2:
        if (p->gravity_combo_timer && --p->gravity_combo_timer == 0)
        {
            p->gravity_combo = 0;
            break;
        }
        if ((p->relics[SOTN_RELIC_GRAVITY] & 2) && (p->tapped & SOTN_BUTTON_JUMP) &&
            !(p->attack_mode & 0x8000) &&
            (p->step == SOTN_PLAYER_CROUCH ||
             (p->step == SOTN_PLAYER_JUMP && (p->movement_flags & 1))))
        {
            if (p->tight_space)
            {
                p->gravity_combo = 0;
                break;
            }
            // HandleGravityBootsMP requires strictly more than four MP.
            if (p->mp > 4)
            {
                p->mp = (Uint16)(p->mp - 4);
                p->mp_spent = 4;
                SotnPlayer_GravityLaunch(p);
                p->gravity_combo = 0;
            }
        }
        break;
    default:
        p->failed = true;
        SDL_SetError("Gravity Boots command cursor is invalid");
        break;
    }
}

static void Ceiling(SotnPlayer *p)
{
    // CreateCeilingHitEffect(3) captures these high-word shifts before its later facing flip.
    const Uint32 x = (Uint32)((Sint64)p->x.value + (p->facing_left ? -3LL : 3LL) * 65536),
                 y = (Uint32)((Sint64)p->y.value - 22LL * 65536);
    p->gravity.impact_source = (SotnPlayerFactorySource){
        .x = (Sint32)(x <= SDL_MAX_SINT32 ? (Sint64)x : (Sint64)x - 4294967296LL),
        .y = (Sint32)(y <= SDL_MAX_SINT32 ? (Sint64)y : (Sint64)y - 4294967296LL),
        .priority = 148,
        .facing_left = p->facing_left};
    p->gravity.ceiling_impact = true;
    p->vx = p->vy = 0;
    p->sound = 0x644;
}

void SotnPlayer_GravityTick(SotnPlayer *p)
{
    p->gravity.timer = (Uint16)(p->gravity.timer + 1U);
    const Sint32 elapsed =
        p->gravity.timer < 32768 ? p->gravity.timer : (Sint32)p->gravity.timer - 65536;
    p->vy = SDL_min(7 * 65536, p->vy);
    Uint16 finish = 0;
    switch (p->substep)
    {
    case 0:
        if (p->contact & 2)
        {
            Ceiling(p);
            if (elapsed > 4)
            {
                p->substep = 2;
                p->gravity.upside_down = true;
                p->draw_pivot_x = 0;
                p->draw_pivot_y = 2;
                p->facing_left = !p->facing_left;
                Animate(p, 43);
            }
            else
                p->substep = 3;
        }
        else if (elapsed > 28)
        {
            p->substep = 1;
            p->vy = -6 * 65536;
            Animate(p, 27);
        }
        break;
    case 1:
        if (p->contact & 2)
        {
            p->substep = 2;
            Ceiling(p);
        }
        else
        {
            p->vy += 24576;
            if (p->vy > 32768)
                finish = 1;
        }
        break;
    case 2:
        p->draw_pivot_x = 0;
        p->draw_pivot_y = 2;
        if (elapsed > 56)
        {
            Animate(p, 45);
            p->gravity.upside_down = false;
            // Retain the original unconditional flip, including a late phase-one impact.
            p->facing_left = !p->facing_left;
            p->substep = 4;
        }
        break;
    case 3:
        if (elapsed > 20)
            finish = 1;
        break;
    case 4:
        p->vy += 4096;
        if (p->animation.remaining < 0)
            finish = 2;
        break;
    default:
        p->failed = true;
        SDL_SetError("Gravity Boots movement substate is invalid");
        break;
    }
    if (finish)
    {
        if (finish == 2)
            Animate(p, 28);
        p->step = SOTN_PLAYER_JUMP;
        p->substep = 1;
    }
}

void SotnPlayer_GravityBeams(SotnPlayer *p)
{
    Uint32 newborn = SOTN_PLAYER_GRAVITY_BEAMS;
    if (p->gravity_spawn)
    {
        for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
            if (!p->gravity.beams[i].active)
            {
                p->gravity.beams[i] =
                    (SotnPlayerGravityBeam){.x = p->gravity_origin_x,
                                            .y = (p->y.parts.high + 37) * 65536 +
                                                 (Sint32)((Uint32)p->gravity_origin_y & 65535U),
                                            .width = 1536,
                                            .phase = 1,
                                            .active = true};
                newborn = i;
                break;
            }
        // The original optional primitive allocation can omit the cosmetic beam.
        p->gravity_spawn = false;
    }
    for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
    {
        SotnPlayerGravityBeam *b = &p->gravity.beams[i];
        if (!b->active || i == newborn)
            continue;
        if (b->phase == 1)
        {
            if (p->vy > -98304)
                b->phase = 2;
            if (p->status & 7U)
                b->phase = 3;
        }
        else
        {
            const Uint16 drain = b->phase == 3 ? 256 : 96;
            if (b->width < drain)
                *b = (SotnPlayerGravityBeam){0};
            else
                b->width = (Uint16)(b->width - drain);
        }
    }
}

void SotnPlayer_GravityCancel(SotnPlayer *p)
{
    if (p->step == SOTN_PLAYER_HIGH_JUMP && p->gravity.upside_down)
    {
        p->gravity.upside_down = false;
        p->facing_left = !p->facing_left;
    }
}
