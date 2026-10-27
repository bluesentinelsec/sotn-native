// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/692E8.c entry and 71830.c:PlayerStepUnmorphMist/PlayerStepStuck,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime artwork stays private.
#include "player_internal.h"
#include <sotn/form_collision.h>

static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static void Animate(SotnPlayer *p, Uint16 id)
{
    if (!SotnAnimation_Set(&p->animation, id))
        p->failed = true;
}
static Sint32 Decelerate(Sint32 velocity)
{
    return velocity > 0   ? SDL_max(0, velocity - 512)
           : velocity < 0 ? SDL_min(0, velocity + 512)
                          : 0;
}
bool SotnPlayer_SetMistContext(SotnPlayer *p, const SotnMistEffect *effect, Uint32 generation,
                               Uint32 game_timer, bool underwater)
{
    if (!p || (!effect && generation) ||
        (effect && (!generation || effect->removed || effect->phase > 5)) ||
        (p && p->mist.active && (!effect || generation != p->mist.generation)))
        return SDL_SetError("Mist context requires the current live generation-owned effect");
    p->mist_context = true;
    p->mist_visual_present = effect != NULL;
    p->mist_visual_phase = effect ? effect->phase : 0;
    p->mist_visual_generation = generation;
    p->mist_game_timer = game_timer;
    p->mist_underwater = underwater;
    return true;
}
void SotnPlayer_MistInput(SotnPlayer *p)
{
    p->mist.started = p->mist.request_restore = p->mist.retire = false;
    if (p->mist_lockout)
        --p->mist_lockout;
    if (!p->mist_context || p->mist_lockout || p->script_hold || p->mist_underwater ||
        !(p->tapped & SOTN_BUTTON_MIST) || p->grab.state != SOTN_GRAB_NONE)
        return;
    switch (p->step)
    {
    case SOTN_PLAYER_STAND:
    case SOTN_PLAYER_WALK:
    case SOTN_PLAYER_CROUCH:
    case SOTN_PLAYER_FALL:
    case SOTN_PLAYER_JUMP:
    case SOTN_PLAYER_STUCK:
    case SOTN_PLAYER_HIGH_JUMP:
        break;
    case SOTN_PLAYER_BAT:
        if (!p->substep)
            return;
        break;
    default:
        return;
    }
    SotnWorldState world = {0};
    SDL_memcpy(world.relics, p->relics, sizeof(world.relics));
    SotnTransformationCharge charge = {0};
    if (!SotnTransformation_MP(&world, SOTN_FORM_MIST, p->mp, p->mist_game_timer, false, &charge))
    {
        p->failed = true;
        return;
    }
    if (!charge.allowed)
        return;
    if (p->mist.generation == SDL_MAX_UINT32)
    {
        p->failed = true;
        SDL_SetError("Mist child generation exhausted; restart the native session");
        return;
    }
    const bool inverted = p->step == SOTN_PLAYER_HIGH_JUMP && p->gravity.upside_down;
    SotnPlayer_GravityCancel(p);
    const Sint16 settle = p->mist_motion.settle_y;
    p->mist_motion = (SotnMistMotion){
        .vx = p->vx, .vy = p->vy, .facing_left = p->facing_left, .settle_y = settle};
    p->mist = (SotnPlayerMist){.generation = p->mist.generation + 1U,
                               .capture_sprite = inverted ? 0x9d : p->animation.sprite,
                               .capture_bank = p->animation_bank,
                               .active = true,
                               .started = true};
    p->form_pose = p->mist.capture_sprite;
    p->bat.active = false;
    p->step = SOTN_PLAYER_MIST;
    p->substep = 0;
    p->sound = 0x6a0;
}
void SotnPlayer_MistTick(SotnPlayer *p)
{
    SotnWorldState world = {0};
    SDL_memcpy(world.relics, p->relics, sizeof(world.relics));
    SotnMistMotionEvents events = {0};
    const SotnMistInput input = {.world = &world,
                                 .game_timer = p->mist_game_timer,
                                 .mp = p->mp,
                                 .directions = p->buttons & 0xf000U,
                                 .cancel = (p->tapped & SOTN_BUTTON_MIST) != 0,
                                 .underwater = p->mist_underwater,
                                 .direction_locked = (p->movement_flags & 2U) != 0};
    if (!SotnMistMotion_Tick(&p->mist_motion, input, &events))
    {
        p->failed = true;
        return;
    }
    p->vx = p->mist_motion.vx;
    p->vy = p->mist_motion.vy;
    p->facing_left = p->mist_motion.facing_left;
    p->turned = p->turned || events.turned;
    p->mp_spent = (Uint16)(p->mp_spent + events.spent);
    p->mp = (Uint16)(p->mp - events.spent);
    p->y.parts.high = Word((Sint32)p->y.parts.high + events.shift_y);
    p->substep = p->mist_motion.phase;
    if (events.started)
    {
        p->movement_flags = 0;
        p->attack_mode = 0;
        p->attack = p->shield = (SotnPlayerAttack){0};
        p->shield_cursor = (SotnShield){0};
        p->subweapon_cast = false;
        Animate(p, 0xca);
    }
    if (events.ended)
    {
        p->step = SOTN_PLAYER_UNMORPH_MIST;
        p->substep = 0;
        p->mist.restoring = true;
    }
}
void SotnPlayer_UnmorphMistTick(SotnPlayer *p)
{
    if (!p->mist_visual_present || p->mist_visual_generation != p->mist.generation)
    {
        p->failed = true;
        SDL_SetError("Mist restoration lost its real generation-owned visual child");
        return;
    }
    if (((p->contact & 1) && p->vy > 0) || ((p->contact & 2) && p->vy < 0))
        p->vy = 0;
    p->vx = Decelerate(p->vx);
    p->vy = Decelerate(p->vy);
    const Sint16 floor_default[4] = {29, 25, 25, 25};
    Uint16 ready = 0;
    for (Uint16 i = 0; i < 4; ++i)
    {
        if (p->floor_sensors[i].y < floor_default[i])
            ++p->floor_sensors[i].y;
        else
            ++ready;
        if (p->ceiling_sensors[i].y > -22)
            --p->ceiling_sensors[i].y;
        else
            ++ready;
        if (i == 0)
        {
            if (p->substep)
            {
                if (p->mist_motion.settle_y < 12)
                    ++p->mist_motion.settle_y;
            }
            else if (p->contact & 0x8000)
                p->y.parts.high = Word((Sint32)p->y.parts.high - 1);
        }
    }
    if ((p->contact & 3) == 3)
    {
        p->substep = 1;
        p->vx = p->vy = 0;
    }
    if (ready == 8)
    {
        SotnFormBarrier barrier = {0};
        if (!SotnFormCollision_Barrier(&p->map, p->x.parts.high, p->y.parts.high, p->facing_left,
                                       &barrier))
        {
            p->failed = true;
            return;
        }
        if (barrier.blocked)
        {
            p->vx = barrier.velocity_x;
            return;
        }
        p->animation_bank = 1;
        Animate(p, p->substep ? 0xcc : 0xcb);
        if (p->mist_visual_phase < 3)
        {
            p->mist.request_restore = true;
            return;
        }
        if (p->mist_visual_phase == 5)
        {
            p->mist.active = p->mist.restoring = false;
            p->mist.retire = true;
            const bool cramped = p->substep != 0;
            SotnPlayer_FormReturn(p, cramped);
            if (cramped)
                return;
        }
    }
    SotnFormEdge edge = {0};
    if (!SotnFormCollision_FloorEdge(&p->map, p->x.parts.high, p->y.parts.high,
                                     p->floor_sensors[1].y, true, &edge))
    {
        p->failed = true;
        return;
    }
    if (edge.displaced)
    {
        p->x.parts.high = Word((Sint32)p->x.parts.high + edge.shift_x);
        p->vx = 0;
    }
}
void SotnPlayer_StuckTick(SotnPlayer *p)
{
    Uint16 contacts = (p->contact & 0x41) == 0x41 ? 1 : 0;
    for (Uint16 i = 0; i < 3; ++i)
    {
        const Sint32 x = p->x.parts.high + (i == 0 ? 0 : i == 1 ? 4 : -4);
        SotnCollider collider = {0};
        if (!SotnCollision_Query(&p->map, x, p->y.parts.high + 25, &collider))
        {
            p->failed = true;
            return;
        }
        if (collider.effects & SOTN_EFFECT_SOLID)
            ++contacts;
    }
    if (!(p->movement_flags & 2U))
    {
        if (p->buttons & SOTN_BUTTON_RIGHT)
            p->facing_left = false;
        else if (p->buttons & SOTN_BUTTON_LEFT)
            p->facing_left = true;
    }
    if (!contacts)
    {
        p->step = SOTN_PLAYER_FALL;
        p->substep = 0;
        Animate(p, 28);
        p->vx = 0;
        p->vy = 2 * 65536;
        p->timers[6] = 8;
        p->timers[5] = (p->previous_contact & 1) ? 8 : 0;
        p->movement_flags = 16;
        return;
    }
    if (!p->substep)
    {
        if (p->animation.remaining < 0)
        {
            Animate(p, 0xf6);
            p->substep = 1;
        }
    }
    else if (p->tapped & 0xf000U)
    {
        Animate(p, 0xc8);
        p->sound = 0x6ee;
        p->substep = 0;
    }
    else if (p->tight_space == 1 || p->tight_space == 2)
    {
        if (p->tight_space == 1 || p->animation.pose > 0)
        {
            p->animation.pose = p->tight_space == 1 ? 0 : 1;
            p->animation.remaining = 3;
        }
    }
    else if (p->tight_space >= 3 && p->animation.pose >= p->tight_space - 1U)
    {
        p->animation.pose = (Uint16)(p->tight_space - 1U);
        p->animation.remaining = 3;
        p->step = SOTN_PLAYER_CROUCH;
        p->substep = 5;
    }
}
