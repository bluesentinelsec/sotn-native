// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c and 71830.c:ControlBatForm/BatFormFinished/PlayerStepUnmorphBat,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/artwork stay private.
#include "player_internal.h"
#include <sotn/form_collision.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Decelerate(Sint32 value, Sint32 amount)
{
    return value > 0 ? SDL_max(0, value - amount) : value < 0 ? SDL_min(0, value + amount) : 0;
}
static void Animate(SotnPlayer *p, Uint16 id)
{
    if (!SotnAnimation_Set(&p->animation, id))
        p->failed = true;
}
static void Rotate(SotnPlayer *p, Sint16 target)
{
    if (p->bat.rotation < target)
        p->bat.rotation = (Sint16)SDL_min(target, p->bat.rotation + 16);
    if (p->bat.rotation > target)
        p->bat.rotation = (Sint16)SDL_max(target, p->bat.rotation - 16);
}
static bool Generation(SotnPlayer *p)
{
    if (!p->bat_factory_available || p->bat_counter == SDL_MAX_UINT32)
    {
        p->failed = true;
        return SDL_SetError("Bat transformation cannot reserve its real visual child");
    }
    p->bat.generation = ++p->bat_counter;
    return true;
}
static bool Bind(SotnPlayer *p, const SotnPlayerBlink *effect, const SotnPlayerFactory *pending,
                 Uint32 generation, bool available, bool specials)
{
    if (!p || (effect && pending) || (!effect && !pending && generation) ||
        (pending && (!generation || generation != p->bat.generation || pending->removed ||
                     !pending->entity_id || pending->blueprint != 44 ||
                     (pending->parameter != 0x2000 && pending->parameter != 0x2100))) ||
        (effect &&
         (!generation || generation != p->bat.generation || effect->removed ||
          (effect->parameter != 0x20 && effect->parameter != 0x21 && effect->parameter != 0x1b))) ||
        (p && p->bat.active && p->bat.generation && p->bat.signal < 3 && !p->mist.active &&
         ((!effect && !pending) || generation != p->bat.generation)))
        return SDL_SetError("Bat context requires the current actual generation-owned effect");
    p->bat_context = true;
    p->bat_specials = specials;
    p->bat_factory_available = available;
    if (effect && generation == p->bat.generation)
    {
        if (effect->parameter == 0x20 && effect->phase == 9)
            p->bat.signal = 2;
        if (effect->parameter == 0x1b)
            p->bat.signal = 3;
    }
    return true;
}
bool SotnPlayer_SetBatContext(SotnPlayer *p, const SotnPlayerBlink *effect, Uint32 generation,
                              bool available)
{
    return Bind(p, effect, NULL, generation, available, false);
}
bool SotnPlayer_SetBatRuntimeContext(SotnPlayer *p, const SotnPlayerBlink *effect,
                                     const SotnPlayerFactory *pending, Uint32 generation,
                                     bool available)
{
    return Bind(p, effect, pending, generation, available, true);
}
void SotnPlayer_BatCancel(SotnPlayer *p)
{
    p->bat.active = p->bat.hidden = false;
    p->bat.rotation = 0;
    p->form_rotated = false;
    p->animation_bank = 1;
    p->status &= ~1U;
}
void SotnPlayer_BatInput(SotnPlayer *p)
{
    p->bat.started = p->bat.return_started = false;
    if (!p->bat_context || p->mist.started || p->mist_lockout || p->script_hold ||
        p->mist_underwater || p->grab.state != SOTN_GRAB_NONE || !(p->tapped & SOTN_BUTTON_BAT))
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
    case SOTN_PLAYER_MIST:
        break;
    case SOTN_PLAYER_WOLF:
        if (!p->substep || p->substep == 8)
            return;
        break;
    default:
        return;
    }
    SotnWorldState world = {0};
    SDL_memcpy(world.relics, p->relics, sizeof(world.relics));
    SotnTransformationCharge charge = {0};
    if (!SotnTransformation_MP(&world, SOTN_FORM_BAT, p->mp, p->mist_game_timer, false, &charge))
    {
        p->failed = true;
        return;
    }
    if (!charge.allowed)
        return;
    const Sint16 settling = p->step == SOTN_PLAYER_CROUCH || p->step == SOTN_PLAYER_STUCK
                                ? 12
                                : p->mist_motion.settle_y;
    if (p->step == SOTN_PLAYER_HIGH_JUMP && p->gravity.upside_down)
        p->animation.sprite = 0x9d;
    p->dark.ticks = 0;
    SotnPlayer_GravityCancel(p);
    p->bat = (SotnPlayerBat){.active = true, .hidden = p->mist.active, .settle_y = settling};
    p->wolf.active = false;
    p->mist_motion.settle_y = settling;
    p->step = SOTN_PLAYER_BAT;
    p->substep = 0;
    p->sound = 0x6a0;
}
static bool Finished(SotnPlayer *p)
{
    if (!p->substep)
        return false;
    SotnWorldState world = {0};
    SDL_memcpy(world.relics, p->relics, sizeof(world.relics));
    SotnTransformationCharge charge = {0};
    const bool cancel = p->mist_underwater || (p->tapped & SOTN_BUTTON_BAT);
    if (!cancel &&
        !SotnTransformation_MP(&world, SOTN_FORM_BAT, p->mp, p->mist_game_timer, true, &charge))
    {
        p->failed = true;
        return true;
    }
    if (!cancel && charge.allowed)
    {
        p->mp_spent = (Uint16)(p->mp_spent + charge.spent);
        p->mp = (Uint16)(p->mp - charge.spent);
        return false;
    }
    if (!Generation(p))
        return true;
    p->step = SOTN_PLAYER_UNMORPH_BAT;
    p->substep = 0;
    Animate(p, 0xca);
    p->form_pose = 6;
    p->bat.hidden = true;
    p->bat.signal = 0;
    p->bat.return_started = true;
    p->vy = p->vy >= 0 ? p->vy / 2 : (Sint32)(-((-(Sint64)p->vy + 1) / 2));
    return true;
}
void SotnPlayer_BatTick(SotnPlayer *p)
{
    if (Finished(p))
        return;
    p->form_rotated = true;
    p->draw_pivot_y = 0;
    const Uint16 directions = p->buttons & 0xf000U;
    const bool jump = (p->buttons & SOTN_BUTTON_JUMP) != 0;
    bool screech = false;
    if (p->bat_specials)
    {
        SotnBatCommandFrame frame = {.animation = p->animation,
                                     .substep = p->substep,
                                     .velocity_x = p->vx,
                                     .velocity_y = p->vy,
                                     .rotation = p->bat.rotation,
                                     .buttons = p->buttons,
                                     .tapped = p->tapped,
                                     .contacts = (Uint16)p->contact,
                                     .mp = p->mp,
                                     .game_timer = p->mist_game_timer,
                                     .facing_left = p->facing_left,
                                     .fire_enabled = (p->relics[SOTN_RELIC_FIRE_BAT] & 2U) != 0,
                                     .echo_enabled = (p->relics[SOTN_RELIC_ECHO_BAT] & 2U) != 0};
        if (!SotnBatCommand_Tick(&p->bat_command, p->dra, &frame, &p->bat_commands))
        {
            p->failed = true;
            return;
        }
        p->animation = frame.animation;
        p->substep = frame.substep;
        p->vx = frame.velocity_x;
        p->vy = frame.velocity_y;
        p->bat.rotation = frame.rotation;
        p->mp = frame.mp;
        p->mp_spent = (Uint16)(p->mp_spent + p->bat_commands.mp_spent);
        if (p->bat_commands.return_requested)
        {
            p->tapped = SOTN_BUTTON_BAT;
            (void)Finished(p);
            SotnPlayer_HoldInput(p, 32);
            if (p->bat_commands.wall_impact)
                p->sound = 0x644;
        }
        if (p->bat_commands.handled)
            goto settling;
    }
    switch (p->substep)
    {
    case 0:
        p->bat.rotation = 0;
        if (p->animation_bank == 15)
        {
            p->wolf.rotation = 0;
            p->wolf.draw_flags = 4;
            p->wolf.blend = 0;
        }
        p->movement_flags = 0;
        if (p->mist.active)
        {
            p->animation_bank = 13;
            p->form_pose = 6;
            p->animation.id = 0xca;
            SotnFormBarrier barrier = {0};
            if (!SotnFormCollision_Barrier(&p->map, p->x.parts.high, p->y.parts.high,
                                           p->facing_left, &barrier))
            {
                p->failed = true;
                return;
            }
            if (barrier.blocked)
                p->vx = barrier.velocity_x;
            if (p->mist_visual_phase == 5)
            {
                if (barrier.blocked)
                    return;
                p->mist.active = false;
                p->mist.retire = true;
            }
            else
            {
                if (p->mist_visual_phase > 0 && p->mist_visual_phase <= 2)
                    p->mist.request_restore = true;
                return;
            }
        }
        else
        {
            if (!p->bat.signal)
            {
                if (!p->bat_factory_available)
                    return;
                if (!Generation(p))
                    return;
                p->bat.started = true;
                p->bat.signal = 1;
                p->attack_mode = 0;
                p->attack = p->shield = (SotnPlayerAttack){0};
                p->shield_cursor = (SotnShield){0};
                p->subweapon_cast = false;
            }
            p->vx = Decelerate(p->vx, 0x480);
            p->vy = SDL_clamp(Decelerate(p->vy, 0x480), -81920, 81920);
            p->form_pose = p->animation.sprite;
            Animate(p, 0xca);
            p->bat.hidden = true;
            if (p->animation_bank == 15)
                p->wolf.palette = 0x810d;
            if (p->bat.signal == 1)
                return;
            if (p->bat.signal == 2)
            {
                p->animation_bank = 13;
                p->form_pose = 6;
                return;
            }
        }
        Animate(p, 0xc3);
        p->animation.remaining = 1;
        p->animation.pose = 2;
        p->bat.hidden = false;
        SotnPlayer_FormDirection(p);
        ++p->substep;
        break;
    case 1:
        if (!directions || jump)
        {
            Rotate(p, 0);
            p->vx = Decelerate(p->vx, 4608);
            p->vy = Decelerate(p->vy, 4608);
            break;
        }
        if (p->animation.id == 0xc3)
            p->animation.pose /= 3;
        ++p->substep;
        SDL_FALLTHROUGH;
    case 2: {
        const Uint16 direction = jump ? 0 : directions;
        const bool left = (direction & SOTN_BUTTON_LEFT) != 0,
                   right = (direction & SOTN_BUTTON_RIGHT) != 0,
                   up = (direction & SOTN_BUTTON_UP) != 0,
                   down = (direction & SOTN_BUTTON_DOWN) != 0;
        if ((!left && !right && !up && !down) || (left && right) || (up && down))
        {
            Animate(p, 0xc3);
            p->substep = 1;
            break;
        }
        const bool diagonal = (left || right) && (up || down);
        const Sint32 deceleration = diagonal ? 3072 : 4608;
        if (up)
        {
            p->animation.id = 0xc2;
            const Sint32 speed = diagonal ? -57344 : -81920;
            p->vy = p->vy < speed ? Decelerate(p->vy, deceleration) : speed;
            Rotate(p, diagonal ? 128 : -128);
        }
        else if (down)
        {
            p->animation.id = p->contact & 1 ? 0xc4 : 0xc5;
            const Sint32 speed = diagonal ? 114688 : 81920;
            p->vy = p->vy > speed ? Decelerate(p->vy, deceleration) : speed;
            Rotate(p, 0);
        }
        else
        {
            p->animation.id = 0xc2;
            p->vy = Decelerate(p->vy, deceleration);
            Rotate(p, 384);
            if (!p->bat_screech)
                p->sound = 0x64e;
            screech = true;
        }
        if (left || right)
        {
            p->facing_left = left;
            const Sint32 speed = (left ? -1 : 1) * (diagonal ? 57344 : 81920);
            p->vx =
                (left ? p->vx < speed : p->vx > speed) ? Decelerate(p->vx, deceleration) : speed;
        }
        else
            p->vx = Decelerate(p->vx, deceleration);
        break;
    }
    default:
        p->failed = true;
        SDL_SetError("Selected Bat flight has an unintegrated command state");
        return;
    }
settling:
    if (p->bat.settle_y)
    {
        if (p->vx > 0)
            p->vx = 0;
        const Sint16 shift = p->bat.settle_y > 0 ? 1 : -1;
        p->bat.settle_y = (Sint16)(p->bat.settle_y - shift);
        p->y.parts.high = Word(p->y.parts.high + shift);
    }
    p->mist_motion.settle_y = p->bat.settle_y;
    p->bat_screech = screech;
}
void SotnPlayer_UnmorphBatTick(SotnPlayer *p)
{
    p->form_rotated = true;
    p->vx = Decelerate(p->vx, 8192);
    if (p->contact & 3)
        p->vy = 0;
    p->vy = Decelerate(p->vy, 8192);
    Rotate(p, 0);
    if (!p->substep)
    {
        const Sint16 defaults[4] = {29, 25, 25, 25};
        Uint16 ready = 0;
        for (Uint16 i = 0; i < 4; ++i)
        {
            if (p->floor_sensors[i].y < defaults[i])
                ++p->floor_sensors[i].y;
            else
                ++ready;
            if (p->ceiling_sensors[i].y > -22)
                --p->ceiling_sensors[i].y;
            else
                ++ready;
            if (!i)
            {
                if (p->bat.cramped)
                {
                    if (p->bat.settle_y < 12)
                        ++p->bat.settle_y;
                }
                else if (p->contact & 0x8000)
                    p->y.parts.high = Word(p->y.parts.high - 1);
            }
        }
        if ((p->contact & 3) == 3)
        {
            p->bat.cramped = true;
            p->vx = p->vy = 0;
        }
        if (ready == 8)
        {
            p->animation_bank = 1;
            p->bat.rotation = 0;
            p->form_rotated = false;
            p->bat.signal = 1;
            p->substep = p->bat.cramped ? 2 : 1;
            p->form_pose = p->substep == 2 ? 0xc7 : 0x5f;
        }
    }
    else if (p->bat.signal == 3)
    {
        p->bat.active = p->bat.hidden = false;
        SotnPlayer_FormReturn(p, p->substep == 2);
    }
    p->mist_motion.settle_y = p->bat.settle_y;
    SotnFormEdge edge = {0};
    if (!SotnFormCollision_FloorEdge(&p->map, p->x.parts.high, p->y.parts.high,
                                     p->floor_sensors[1].y, false, &edge))
    {
        p->failed = true;
        return;
    }
    if (edge.displaced)
    {
        p->x.parts.high = Word(p->x.parts.high + edge.shift_x);
        p->vx = 0;
    }
}
