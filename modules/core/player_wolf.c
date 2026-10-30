// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c and wolfForm.c ownership/resource/control integration,
// QCF from dra/704D0.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "player_internal.h"
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint16 Whole(Sint32 value)
{
    return Word(value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536)));
}
static bool Relic(const SotnPlayer *p, Uint16 id)
{
    return (p->relics[id] & 3U) == 3;
}
static bool Generation(SotnPlayer *p)
{
    if (p->bat_counter == SDL_MAX_UINT32)
    {
        p->failed = true;
        return SDL_SetError("Wolf transformation generation exhausted; restart the native session");
    }
    p->wolf.generation = ++p->bat_counter;
    return true;
}
static void Animate(SotnPlayer *p, Uint16 id)
{
    if (!SotnAnimation_Set(&p->animation, id))
        p->failed = true;
}
static void Command(SotnPlayer *p)
{
    const bool complete = p->wolf_combo >= 3;
    const Uint16 directions = p->buttons & 0xf000U,
                 forward = p->wolf_combo_facing ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    const Uint16 expected = p->wolf_combo == 0   ? SOTN_BUTTON_DOWN
                            : p->wolf_combo == 1 ? (Uint16)(SOTN_BUTTON_DOWN | forward)
                                                 : forward;
    if (!p->wolf_combo)
        p->wolf_combo_facing = p->facing_left;
    if (p->wolf_combo < 3 && directions == expected)
    {
        p->wolf_combo_timer = 20;
        p->wolf_combo = p->wolf_combo == 2 ? 255 : (Uint16)(p->wolf_combo + 1U);
        return;
    }
    if (p->wolf_combo >= 3)
    {
        if (!p->wolf_combo_timer)
            return;
        if (p->tight_space)
        {
            p->wolf_combo = 0;
            return;
        }
    }
    p->wolf_combo_timer = Word((Sint32)p->wolf_combo_timer - 1);
    if (!p->wolf_combo_timer)
    {
        p->wolf_combo = 0;
        if (complete)
            p->wolf_combo_facing = p->facing_left;
    }
}
bool SotnPlayer_SetWolfContext(SotnPlayer *p, const SotnWolfRuntime *body,
                               const SotnPlayerBlink *effect, const SotnPlayerFactory *pending,
                               Uint32 generation, bool available, Sint32 camera_x, Sint32 camera_y)
{
    if (!p || !body || (effect && pending) ||
        ((effect || pending) ? !generation : generation != 0) || camera_x < 0 || camera_x > 16384 ||
        camera_y < 0 || camera_y > 16384 || (generation && generation != p->wolf.generation) ||
        (pending && (pending->removed || !pending->entity_id || pending->blueprint != 44 ||
                     (pending->parameter != 0x2300 && pending->parameter != 0x2400 &&
                      pending->parameter != 0x1b00))) ||
        (effect && (effect->removed || (effect->parameter != 0x23 && effect->parameter != 0x24 &&
                                        effect->parameter != 0x25 && effect->parameter != 0x1b))) ||
        (p && p->step == SOTN_PLAYER_WOLF && p->substep &&
         body->generation != p->wolf.body_generation) ||
        (p && p->wolf.active && p->wolf.generation && p->wolf.signal < 3 && !p->mist.active &&
         ((!effect && !pending) || generation != p->wolf.generation)))
        return SDL_SetError(
            "Wolf context requires its actual generation-owned body/effect/factory");
    p->wolf_context = true;
    p->wolf_factory_available = available;
    p->wolf_camera_x = camera_x;
    p->wolf_camera_y = camera_y;
    if (body->generation)
    {
        p->wolf_body_root_y = Whole(body->body.parts[0].y);
    }
    if (effect)
    {
        if (effect->parameter == 0x23 && effect->phase == 13)
            p->wolf.signal = 2;
        if (effect->parameter == 0x25 || effect->parameter == 0x1b)
            p->wolf.signal = 3;
    }
    return true;
}
static SotnWolfControllerFrame Snapshot(const SotnPlayer *p)
{
    SotnWolfControllerFrame f = p->wolf.controller;
    f.x = (Sint32)((Sint64)p->x.value - (Sint64)p->wolf_camera_x * 65536);
    f.y = (Sint32)((Sint64)p->y.value - (Sint64)p->wolf_camera_y * 65536);
    f.motion.animation = p->animation;
    f.motion.velocity_x = p->vx;
    f.motion.velocity_y = p->vy;
    f.motion.contacts = (Uint32)p->contact;
    f.motion.substep = p->substep;
    f.motion.facing_left = p->facing_left;
    f.motion.buttons = p->buttons;
    f.root_y = p->wolf_body_root_y;
    f.tapped = p->tapped;
    f.mp = p->mp;
    f.coyote_ticks = p->timers[5];
    f.drop_ticks = p->timers[7];
    f.priority = p->wolf.priority;
    f.game_timer = p->mist_game_timer;
    f.raw_contacts = (Uint32)p->previous_contact;
    f.direction_locked = (p->movement_flags & 2U) != 0;
    f.skill_enabled = Relic(p, SOTN_RELIC_SKILL_WOLF);
    f.power_enabled = Relic(p, SOTN_RELIC_POWER_WOLF);
    f.holy_symbol_enabled = Relic(p, 14);
    f.water_level = p->water.waist;
    f.fall_water_level = p->water.feet;
    f.quarter_circle_complete = p->wolf_combo == 255;
    return f;
}
static void Apply(SotnPlayer *p, SotnWolfControllerFrame f)
{
    p->wolf.controller = f;
    p->animation = f.motion.animation;
    p->vx = f.motion.velocity_x;
    p->vy = f.motion.velocity_y;
    p->substep = f.motion.substep;
    p->facing_left = f.motion.facing_left;
    p->mp = f.mp;
    p->timers[5] = (Uint8)f.coyote_ticks;
    p->timers[7] = (Uint8)f.drop_ticks;
}
void SotnPlayer_WolfCancel(SotnPlayer *p)
{
    p->wolf.active = p->wolf.hidden = false;
    p->wolf.rotation = 0;
    p->animation_bank = 1;
    p->status &= ~4U;
}
void SotnPlayer_WolfInput(SotnPlayer *p)
{
    p->wolf.started = p->wolf.body_started = p->wolf.finish_flash = p->wolf.return_started = false;
    if (!p->wolf_context)
        return;
    Command(p);
    if (p->mist_lockout || p->script_hold || p->mist.started || p->grab.state != SOTN_GRAB_NONE ||
        !(p->tapped & SOTN_BUTTON_WOLF))
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
    case SOTN_PLAYER_BAT:
        if (!p->substep)
            return;
        break;
    default:
        return;
    }
    if (p->mist_underwater && !Relic(p, 14))
        return;
    SotnWorldState world = {0};
    SDL_memcpy(world.relics, p->relics, sizeof(world.relics));
    SotnTransformationCharge charge = {0};
    if (!SotnTransformation_MP(&world, SOTN_FORM_WOLF, p->mp, p->mist_game_timer, false, &charge))
    {
        p->failed = true;
        return;
    }
    if (!charge.allowed)
        return;
    if (p->step == SOTN_PLAYER_HIGH_JUMP && p->gravity.upside_down)
        p->animation.sprite = 0x9d;
    p->dark.ticks = 0;
    SotnPlayer_GravityCancel(p);
    const SotnWolfControllerFrame retained = p->wolf.controller;
    p->wolf = (SotnPlayerWolf){.controller = retained,
                               .active = true,
                               .priority = 146,
                               .palette = 0x104,
                               .draw_flags = p->form_rotated ? 4 : 0};
    p->bat.active = false;
    p->step = SOTN_PLAYER_WOLF;
    p->substep = 0;
    p->sound = 0x668;
}
static bool Finished(SotnPlayer *p)
{
    if (!p->substep || p->substep == 8)
        return false;
    const bool cancel = (p->tapped & SOTN_BUTTON_WOLF) || (p->mist_underwater && !Relic(p, 14));
    SotnTransformationCharge charge = {0};
    if (!cancel)
    {
        SotnWorldState world = {0};
        SDL_memcpy(world.relics, p->relics, sizeof(world.relics));
        if (!SotnTransformation_MP(&world, SOTN_FORM_WOLF, p->mp, p->mist_game_timer, true,
                                   &charge))
        {
            p->failed = true;
            return true;
        }
        if (charge.allowed)
        {
            p->mp = (Uint16)(p->mp - charge.spent);
            p->mp_spent = (Uint16)(p->mp_spent + charge.spent);
            return false;
        }
    }
    // The original return does not retry a failed factory. Retain that valid missing-child
    // wait explicitly, instead of inventing readiness or borrowing the prior entry generation.
    p->wolf.return_factory_missing = !p->wolf_factory_available;
    if (p->wolf_factory_available)
    {
        if (!Generation(p))
            return true;
    }
    else
        p->wolf.generation = 0;
    p->step = SOTN_PLAYER_UNMORPH_WOLF;
    p->substep = 0;
    Animate(p, 0xca);
    p->form_pose = 1;
    p->wolf.palette = 0x810d;
    p->wolf.hidden = true;
    p->wolf.signal = 0;
    p->wolf.cramped = false;
    p->wolf.return_started = !p->wolf.return_factory_missing;
    p->vy = p->vy >= 0 ? p->vy / 2 : (Sint32)(-((-(Sint64)p->vy + 1) / 2));
    return true;
}
static void Transition(SotnPlayer *p)
{
    SotnWolfTransition t = {.player = Snapshot(p),
                            .bank = p->animation_bank,
                            .palette = p->wolf.palette,
                            .texture = p->wolf.texture,
                            .saved_bank = p->wolf.capture_bank,
                            .saved_sprite = p->wolf.capture_sprite,
                            .form_pose = p->form_pose,
                            .rotation = p->wolf.rotation,
                            .signal = p->wolf.signal,
                            .draw_flags = p->wolf.draw_flags,
                            .timer14 = Word(p->invincibility),
                            .returning = p->step == SOTN_PLAYER_UNMORPH_WOLF,
                            .cramped = p->wolf.cramped,
                            .movement = {(Uint16)p->movement_flags, 0, 0}};
    for (Uint16 i = 0; i < 4; ++i)
    {
        t.floor_y[i] = p->floor_sensors[i].y;
        t.ceiling_y[i] = p->ceiling_sensors[i].y;
    }
    const bool entry = !t.returning && !p->substep;
    if (entry)
    {
        if (!p->mist.active && !p->wolf_factory_available)
            return;
        if (!Generation(p))
            return;
        p->wolf.body_generation = p->wolf.generation;
    }
    SotnWolfTransitionEvents e = {0};
    if (!SotnWolfTransition_Tick(
            &t,
            (SotnWolfTransitionInput){.center_priority = 148,
                                      .mist_child = p->mist.active,
                                      .mist_ready = p->mist.active && p->mist_visual_phase == 5,
                                      .entry_factory_created = entry && !p->mist.active},
            &e))
    {
        p->failed = true;
        return;
    }
    Apply(p, t.player);
    p->animation_bank = t.bank;
    p->wolf.palette = t.palette;
    p->wolf.texture = t.texture;
    p->wolf.capture_bank = t.saved_bank;
    p->wolf.capture_sprite = t.saved_sprite;
    p->form_pose = t.form_pose;
    p->wolf.rotation = t.rotation;
    p->wolf.signal = (Uint8)t.signal;
    p->wolf.draw_flags = t.draw_flags;
    p->wolf.priority = t.player.priority;
    p->wolf.cramped = t.cramped;
    p->invincibility = (Uint16)t.timer14;
    p->movement_flags = t.movement[0];
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->floor_sensors[i].y = t.floor_y[i];
        p->ceiling_sensors[i].y = t.ceiling_y[i];
    }
    p->wolf.started = e.entry_flash;
    p->wolf.body_started = e.replace_body;
    p->wolf.finish_flash = e.finish_flash;
    if (e.clear_weapon)
    {
        p->attack_mode = 0;
        p->attack = p->shield = (SotnPlayerAttack){0};
        p->shield_cursor = (SotnShield){0};
        p->subweapon_cast = false;
    }
    if (p->mist.active && p->substep == 8 && p->mist_visual_phase > 0 && p->mist_visual_phase <= 2)
        p->mist.request_restore = true;
    if (e.finish_flash)
    {
        p->mist.active = false;
        p->mist.retire = true;
    }
    if (e.normal_return || e.stuck_return)
    {
        p->wolf.active = p->wolf.hidden = false;
        p->wolf.palette = 0x8100;
        p->wolf.priority = 148;
        SotnPlayer_WolfReturn(p, e.stuck_return);
    }
}
void SotnPlayer_WolfTick(SotnPlayer *p)
{
    p->wolf.palette = 0x104;
    p->wolf.blend = 0;
    p->wolf.priority = 146;
    if (Finished(p))
        return;
    p->wolf.controller.joint_distance = 14;
    if (!p->substep || p->substep == 8)
    {
        if (p->wolf.controller.run_window)
            --p->wolf.controller.run_window;
        if (p->wolf.controller.swim_lock)
            --p->wolf.controller.swim_lock;
        Transition(p);
    }
    else
    {
        SotnWolfControllerFrame f = Snapshot(p);
        SotnWolfControllerEvents e = {0};
        if (!SotnWolfController_Tick(&f, p->dra, &e))
        {
            p->failed = true;
            return;
        }
        // The component reproduces original screen-space requests. Export owned
        // room-space sources before physics/camera follow can change their origin.
        for (Uint16 i = 0; i < e.count; ++i)
        {
            const Sint64 x = (Sint64)e.requests[i].source.x + (Sint64)p->wolf_camera_x * 65536,
                         y = (Sint64)e.requests[i].source.y + (Sint64)p->wolf_camera_y * 65536;
            if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 ||
                y > SDL_MAX_SINT32)
            {
                p->failed = true;
                SDL_SetError("Wolf command source exceeds the owned room coordinate range");
                return;
            }
            e.requests[i].source.x = (Sint32)x;
            e.requests[i].source.y = (Sint32)y;
        }
        Apply(p, f);
        p->wolf.commands = e;
        p->mp_spent = (Uint16)(p->mp_spent + e.mp_spent);
        p->bat_input_hold = (Uint8)e.input_hold;
        if (e.return_requested)
        {
            p->tapped = SOTN_BUTTON_WOLF;
            (void)Finished(p);
        }
    }
    if (p->failed)
        return;
    if (!SotnWolf_Cadence(p->vx, p->wolf.controller.motion.body_angle, &p->wolf.controller.cadence))
    {
        p->failed = true;
        return;
    }
    p->wolf.controller.motion.body_angle = p->wolf.controller.cadence.body_angle;
    p->wolf.hidden = p->wolf.palette == 0x810d;
}
void SotnPlayer_UnmorphWolfTick(SotnPlayer *p)
{
    Transition(p);
}
bool SotnPlayer_CommitWolfBody(SotnPlayer *p, const SotnWolfRuntime *r,
                               const SotnWolfControllerFrame *f, Sint32 camera_x, Sint32 camera_y)
{
    if (!p || !r || !f || p->step != SOTN_PLAYER_WOLF || !p->wolf.active || !r->generation ||
        r->generation != p->wolf.body_generation || f->game_timer != p->mist_game_timer ||
        p->wolf_body_commit_frame == p->frame_number || camera_x < 0 || camera_x > 16384 ||
        camera_y < 0 || camera_y > 16384 || f->x != (Sint64)p->x.value - (Sint64)camera_x * 65536 ||
        f->y != (Sint64)p->y.value - (Sint64)camera_y * 65536 || f->mp > p->mp ||
        f->motion.substep > 9 || f->motion.mode > 4 || f->motion.animation.pose >= 8 ||
        f->motion.animation.remaining < -1 || f->motion.animation.remaining > 32767 ||
        f->motion.body_angle < 0 || f->motion.body_angle > 4095 || p->mp - f->mp > 255 ||
        f->coyote_ticks > 255 || f->drop_ticks > 255 ||
        ((f->motion.animation.id < 0xde || f->motion.animation.id > 0xed) &&
         !(f->motion.animation.id == 0xca && (!p->substep || p->substep == 8) &&
           !r->root_reserved && !r->body.parts[0].step)) ||
        f->motion.velocity_x < -16 * 65536 || f->motion.velocity_x > 16 * 65536 ||
        f->motion.velocity_y < -16 * 65536 || f->motion.velocity_y > 16 * 65536)
        return SDL_SetError("Wolf body feedback lost its actual player/generation/update origin");
    p->wolf_camera_x = camera_x;
    p->wolf_camera_y = camera_y;
    p->wolf_body_commit_frame = p->frame_number;
    Apply(p, *f);
    p->wolf.palette = r->root.player_palette;
    p->wolf.draw_flags = r->root.player_draw_flags;
    p->wolf.blend = r->root.player_blend;
    p->wolf.opacity = r->root.player_opacity;
    p->wolf.hidden = p->wolf.palette == 0x810d;
    p->wolf_body_root_y = Whole(r->body.parts[0].y);
    return true;
}
