// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected active Wolf substates and helpers: dra/wolfForm.c, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Original spell tables remain private.
#include <sotn/collision.h>
#include <sotn/player.h>
#include <sotn/wolf_controller.h>

typedef struct Context
{
    SotnWolfControllerFrame frame;
    SotnWolfControllerEvents events;
    SotnDataView dra;
    bool failed;
} Context;
static Sint32 Word(Sint32 value, Uint32 amount)
{
    const Uint32 bits = (Uint32)value + amount;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}
static void Animate(Context *c, Uint16 id)
{
    if (!SotnAnimation_Set(&c->frame.motion.animation, id))
        c->failed = true;
}
static void Request(Context *c, Uint8 kind, Uint32 parameter, Sint16 shift_x)
{
    if (c->events.count >= SDL_arraysize(c->events.requests))
    {
        c->failed = true;
        SDL_SetError("Wolf controller exceeded its bounded ordered requests");
        return;
    }
    const SotnWolfControllerFrame *f = &c->frame;
    c->events.requests[c->events.count++] =
        (SotnWolfRequest){.source = {.x = Word(f->x, (Uint32)(Sint32)shift_x * 65536U),
                                     .y = f->y,
                                     .flags = f->entity_flags,
                                     .priority = f->priority,
                                     .facing_left = f->motion.facing_left},
                          .kind = kind,
                          .parameter = parameter};
}
static void Speed(Context *c, Sint32 speed)
{
    c->frame.motion.velocity_x = c->frame.motion.facing_left ? -speed : speed;
}
static void Decelerate(Context *c, Sint32 amount)
{
    Sint32 *speed = &c->frame.motion.velocity_x;
    *speed = *speed > 0   ? SDL_max(0, *speed - amount)
             : *speed < 0 ? SDL_min(0, *speed + amount)
                          : 0;
}
static void Direction(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    if (f->direction_locked)
        return;
    bool facing = f->motion.facing_left;
    if (f->motion.buttons & SOTN_BUTTON_RIGHT)
        facing = false;
    else if (f->motion.buttons & SOTN_BUTTON_LEFT)
        facing = true;
    c->events.turned |= facing != f->motion.facing_left;
    f->motion.facing_left = facing;
}
static bool Backward(const Context *c)
{
    const SotnWolfJumpFrame *f = &c->frame.motion;
    return (f->buttons & (f->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT)) != 0;
}
static bool Forward(const Context *c)
{
    const SotnWolfJumpFrame *f = &c->frame.motion;
    return (f->buttons & (f->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT)) != 0;
}
static void Jump(Context *c)
{
    if (!SotnWolf_Jump(&c->frame.motion))
        c->failed = true;
}
static void Stand(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    f->motion.substep = 1;
    f->motion.mode = 0;
    Animate(c, (Uint16)(0xde + ((f->motion.contacts & 0x20U) ? 1U : 0U)));
    f->motion.velocity_x /= 2;
    f->motion.velocity_y = 0;
    f->idle_timer = 512;
    if (f->motion.contacts & 0x40U)
    {
        f->motion.mode = 1;
        Animate(c, 0xe9);
    }
}
static void Walk(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    f->motion.substep = 2;
    if (Backward(c))
    {
        Animate(c, 0xe1);
        f->motion.mode = 0;
        f->run_window = 0;
    }
    else if (f->run_window)
    {
        Animate(c, 0xe2);
        f->motion.mode = 2;
        Speed(c, 131072);
    }
    else
    {
        Animate(c, 0xe0);
        f->motion.mode = 1;
        f->run_window = 12;
    }
}
static void Fall(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    if (f->motion.substep == 2 && f->motion.mode == 2)
    {
        Animate(c, 0xe7);
        f->motion.mode = 1;
    }
    else
    {
        Animate(c, 0xe8);
        Speed(c, 65536);
        f->motion.mode = 0;
        if (f->previous_contacts & 0x40U)
        {
            f->motion.velocity_x = 0;
            f->motion.animation.pose = 4;
            f->motion.animation.remaining = 1;
        }
    }
    f->motion.substep = 5;
    f->motion.body_angle = Word(f->motion.body_angle, (Uint32)-256);
    f->coyote_ticks = 8;
    f->motion.velocity_y = 0;
}
static void Recoil(Context *c)
{
    Animate(c, 0xea);
    c->frame.motion.mode = 0;
    c->frame.motion.velocity_x = 0;
    c->frame.motion.substep = 6;
    c->frame.coyote_ticks = 8;
}
static void Crouch(Context *c)
{
    c->frame.motion.substep = 3;
    Animate(c, 0xe3);
    c->frame.motion.mode = 0;
}
static void Attack(Context *c, bool airborne)
{
    SotnWolfControllerFrame *f = &c->frame;
    c->events.attack_requested = true;
    if (airborne || !f->quarter_circle_complete || !f->skill_enabled)
        return;
    const size_t at = 0x8410U + 4U * 28U;
    if (!c->dra.bytes || at > c->dra.size || c->dra.size - at < 28)
    {
        c->failed = true;
        SDL_SetError("Wolf charge requires its complete private spell-four definition");
        return;
    }
    const Uint16 cost = c->dra.bytes[at + 12];
    if (f->mp < cost)
        return;
    f->mp = (Uint16)(f->mp - cost);
    c->events.mp_spent = cost;
    Direction(c);
    f->motion.substep = 2;
    f->motion.mode = 4;
    Speed(c, 5 * 65536);
    f->motion.velocity_y = 0;
    Animate(c, 0xed);
    c->events.learn_charge = true;
}
static void Wall(Context *c, bool return_form)
{
    Request(c, SOTN_WOLF_SHAKE, 2, 0);
    Request(c, SOTN_WOLF_SOUND, 0x644, 0);
    c->frame.motion.velocity_x = 0;
    c->events.input_hold = 32;
    if (return_form)
    {
        c->events.return_requested = true;
        return;
    }
    bool side = false;
    for (Uint16 i = 3; i < 14; ++i)
        if ((i < 7 || i >= 10) && (c->frame.wall_effects[i] & SOTN_EFFECT_SIDE))
            side = true;
    Speed(c, 65536);
    Request(c, SOTN_WOLF_FACTORY, 0x90004, 0);
    c->frame.motion.mode = 0;
    if (!side)
    {
        Fall(c);
        Speed(c, 65536);
    }
    else
    {
        Recoil(c);
        Speed(c, -65536);
    }
    c->frame.motion.velocity_y = -229376;
}
static bool WallImpact(Context *c)
{
    const SotnWolfJumpFrame *f = &c->frame.motion;
    if (((f->contacts & 4U) && f->velocity_x > 360448) ||
        ((f->contacts & 8U) && f->velocity_x < -360448))
    {
        Wall(c, true);
        return true;
    }
    if (((f->contacts & 4U) && f->velocity_x > 262144) ||
        ((f->contacts & 8U) && f->velocity_x < -360448))
    {
        Wall(c, false);
        return true;
    }
    return false;
}
static void Idle(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    Decelerate(c, 8192);
    if (f->tapped & SOTN_BUTTON_JUMP)
        Jump(c);
    else if (!(f->motion.contacts & 1U))
        Fall(c);
    else if (f->tapped & SOTN_BUTTON_LEFT_HAND)
        Attack(c, false);
    else if (f->motion.buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT))
        Walk(c);
    else if (f->motion.buttons & SOTN_BUTTON_DOWN)
        Crouch(c);
    else if (f->motion.mode == 0)
    {
        const bool settled = SDL_abs(Pixel(f->y) - f->root_y) < 4;
        if ((settled && --f->idle_timer == 0) || (f->motion.contacts & 0x40U))
        {
            f->motion.mode = 1;
            Animate(c, 0xe9);
        }
    }
}
static void Slide(Context *c)
{
    if (c->frame.tapped & SOTN_BUTTON_JUMP)
        Jump(c);
    else if (!(c->frame.motion.contacts & 1U))
        Recoil(c);
    else if (Forward(c))
        Walk(c);
    else if (c->frame.raw_contacts & 0x40U)
        Stand(c);
    else if (c->frame.game_timer % 6U == 0)
        Request(c, SOTN_WOLF_FACTORY, 0x10045, 0);
}
static void Running(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    SotnWolfJumpFrame *p = &f->motion;
    const Uint16 directions = p->buttons & 0xf000U;
    if ((f->tapped & SOTN_BUTTON_JUMP) && p->mode != 4)
    {
        Jump(c);
        return;
    }
    if (!(p->contacts & 1U) && p->mode != 4)
    {
        Fall(c);
        return;
    }
    if ((p->buttons & SOTN_BUTTON_DOWN) && p->mode != 4)
    {
        Crouch(c);
        return;
    }
    switch (p->mode)
    {
    case 0:
        if (f->tapped & SOTN_BUTTON_LEFT_HAND)
        {
            Attack(c, false);
            break;
        }
        if (p->animation.pose >= 3)
        {
            if (p->animation.remaining < 0)
            {
                Walk(c);
                if (!(directions & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)))
                {
                    Stand(c);
                    break;
                }
            }
            Speed(c, 65536);
            break;
        }
        if (p->animation.remaining == 1 && p->animation.pose == 2)
            p->facing_left = !p->facing_left;
        Speed(c, -65536);
        break;
    case 1:
        if (f->tapped & SOTN_BUTTON_LEFT_HAND)
        {
            Attack(c, false);
            break;
        }
        Speed(c, 65536);
        if (Backward(c))
        {
            p->mode = 0;
            Animate(c, 0xe1);
        }
        if (!(directions & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)))
            Stand(c);
        break;
    case 2:
        if ((f->tapped & SOTN_BUTTON_LEFT_HAND) && SDL_abs(p->velocity_x) < 3 * 65536)
        {
            Attack(c, false);
            break;
        }
        if (f->game_timer % 6U == 0)
            Request(c, SOTN_WOLF_FACTORY, 0x10045, 0);
        if (p->velocity_x > 0)
        {
            p->velocity_x = SDL_min(p->velocity_x + 1536, 9 * 65536);
            if (!f->power_enabled && p->velocity_x > 3 * 65536)
                p->velocity_x = 3 * 65536;
        }
        if (p->velocity_x < 0)
        {
            p->velocity_x = SDL_max(p->velocity_x - 1536, -9 * 65536);
            // Original US code compares negative speed against positive three here.
            if (!f->power_enabled && p->velocity_x < 3 * 65536)
                p->velocity_x = -3 * 65536;
        }
        if (WallImpact(c))
            break;
        if (((p->contacts & 4U) && p->velocity_x > 0) ||
            ((p->contacts & 8U) && p->velocity_x < 0) ||
            !(directions & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)))
        {
            p->animation.id = 0xe0;
            p->mode = 3;
            f->turn_timer = 24;
            break;
        }
        if (p->facing_left)
        {
            if ((f->raw_contacts & 0xf001U) == 1 && (f->previous_contacts & 0xf001U) == 0xc001U)
            {
                Jump(c);
                p->velocity_y /= 4;
            }
            if ((f->raw_contacts & 0xf001U) == 0x8001U && (f->previous_contacts & 0xf001U) == 1)
            {
                Jump(c);
                p->velocity_y /= 2;
            }
        }
        else
        {
            if ((f->raw_contacts & 0xf001U) == 1 && (f->previous_contacts & 0xf001U) == 0x8001U)
            {
                Jump(c);
                p->velocity_y /= 4;
            }
            if ((f->raw_contacts & 0xf001U) == 0xc001U && (f->previous_contacts & 0xf001U) == 1)
            {
                Jump(c);
                p->velocity_y /= 2;
            }
        }
        break;
    case 3:
        if ((f->tapped & SOTN_BUTTON_LEFT_HAND) && SDL_abs(p->velocity_x) < 3 * 65536)
        {
            Attack(c, false);
            break;
        }
        if (SDL_abs(p->velocity_x) > 65536)
            Decelerate(c, 8192);
        if (Backward(c))
        {
            p->mode = 0;
            Animate(c, 0xe1);
        }
        if (--f->turn_timer == 0)
        {
            p->mode = 1;
            f->run_window = 12;
        }
        break;
    case 4:
        Decelerate(c, 1024);
        if (f->game_timer % 2U == 0)
            Request(c, SOTN_WOLF_FACTORY, 0x10045, 0);
        if (p->animation.remaining >= 0)
            break;
        if (Forward(c))
        {
            Animate(c, 0xe2);
            p->mode = 2;
            if (SDL_abs(p->velocity_x) < 2 * 65536)
            {
                Speed(c, 2 * 65536);
                break;
            }
        }
        else
        {
            Stand(c);
            p->velocity_x = 0;
        }
        break;
    }
}
static void Crouching(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    SotnWolfJumpFrame *p = &f->motion;
    const Uint16 pose = p->animation.pose;
    const bool down = (p->buttons & SOTN_BUTTON_DOWN) != 0;
    Decelerate(c, 8192);
    if (f->tapped & SOTN_BUTTON_JUMP)
    {
        if (down)
            for (Uint16 i = 0; i < 4; ++i)
                if (f->floor_effects[i] & SOTN_EFFECT_SOLID_FROM_ABOVE)
                {
                    f->drop_ticks = 8;
                    Fall(c);
                    p->velocity_x = 0;
                    p->animation.pose = 4;
                    p->animation.remaining = 1;
                    p->velocity_y = 2 * 65536;
                    return;
                }
        Jump(c);
        return;
    }
    if (!(p->contacts & 1U))
    {
        Fall(c);
        return;
    }
    if ((f->tapped & SOTN_BUTTON_LEFT_HAND) && p->mode != 3)
    {
        Attack(c, false);
        return;
    }
    switch (p->mode)
    {
    case 0:
        if (!down)
        {
            Animate(c, 0xe4);
            p->mode = 2;
            if (!pose)
                p->animation.pose = 1;
        }
        else if (p->animation.remaining < 0)
            ++p->mode;
        break;
    case 1:
        if (!down)
        {
            Animate(c, 0xe4);
            p->mode = 2;
        }
        break;
    case 2:
        if (down)
        {
            Animate(c, 0xe3);
            p->mode = 0;
            if (!pose)
                p->animation.pose = 1;
        }
        else if (p->animation.remaining < 0)
            Stand(c);
        break;
    case 3:
        if (p->animation.remaining < 0)
            Stand(c);
        break;
    }
}
static void Gravity(Context *c)
{
    const Sint32 amount = c->frame.fall_water_level > 12 ? 2560 : 10240;
    c->frame.motion.velocity_y = SDL_min(c->frame.motion.velocity_y + amount, 7 * 65536);
}
static void HardLanding(Context *c)
{
    c->frame.motion.substep = 3;
    c->frame.motion.mode = 3;
    Animate(c, 0xe5);
    Request(c, SOTN_WOLF_FACTORY, 0, 0);
}
static void Falling(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    SotnWolfJumpFrame *p = &f->motion;
    if (f->coyote_ticks && (f->tapped & SOTN_BUTTON_JUMP))
    {
        Jump(c);
        return;
    }
    if (!Forward(c))
        Decelerate(c, 2048);
    if (p->contacts & 1U)
    {
        if (p->mode == 1)
        {
            p->substep = 2;
            p->mode = 2;
            Animate(c, 0xe2);
        }
        else if (p->velocity_y > 450560)
            HardLanding(c);
        else
            Stand(c);
        p->velocity_y = 0;
        Request(c, SOTN_WOLF_SOUND, 0x64c, 0);
        return;
    }
    if (f->tapped & SOTN_BUTTON_LEFT_HAND)
        Attack(c, true);
    switch (p->mode)
    {
    case 0:
        Direction(c);
        break;
    case 1:
        if (WallImpact(c))
            return;
        if (((p->contacts & 4U) && p->velocity_x > 163840) ||
            ((p->contacts & 8U) && p->velocity_x < -163840))
            Decelerate(c, 8192);
        if (p->animation.pose == 3)
            p->animation.remaining = 6;
        break;
    }
    Gravity(c);
}
static void Recoiling(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    if (f->coyote_ticks && (f->tapped & SOTN_BUTTON_JUMP))
    {
        Jump(c);
        return;
    }
    Gravity(c);
    if (f->tapped & SOTN_BUTTON_LEFT_HAND)
        Attack(c, true);
    if (f->motion.contacts & 1U)
    {
        Request(c, SOTN_WOLF_SOUND, 0x64c, 0);
        if (f->motion.velocity_y > 450560)
            HardLanding(c);
        else
            Stand(c);
        f->motion.velocity_y = 0;
    }
}
static void Jumping(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    SotnWolfJumpFrame *p = &f->motion;
    const bool forward = Forward(c);
    if (!forward)
        Decelerate(c, 8192);
    if (p->contacts & 1U)
    {
        if (p->mode == 2)
        {
            p->substep = 2;
            p->mode = 2;
            Animate(c, 0xe2);
        }
        else if (p->velocity_y > 450560 || p->mode == 3)
            HardLanding(c);
        else
        {
            // These are separate original tests: Stand can itself select mode one.
            if (p->mode == 0)
                Stand(c);
            if (p->mode == 1)
                Walk(c);
        }
        Request(c, SOTN_WOLF_SOUND, 0x64c, 0);
        p->velocity_y = 0;
        return;
    }
    if (p->velocity_y < -65536 && (p->contacts & 2U))
    {
        if (p->velocity_y < -5 * 65536)
        {
            Request(c, SOTN_WOLF_FACTORY, 0x10004, p->facing_left ? -3 : 3);
            Request(c, SOTN_WOLF_SHAKE, 3, 0);
            Request(c, SOTN_WOLF_SOUND, 0x644, 0);
            p->velocity_x = p->velocity_y = 0;
            Fall(c);
            return;
        }
        p->velocity_y = 0;
    }
    if (f->tapped & SOTN_BUTTON_LEFT_HAND)
        Attack(c, true);
    switch (p->mode)
    {
    case 0:
        Direction(c);
        if (forward && SDL_abs(p->velocity_x) < 65536)
            Speed(c, 65536);
        break;
    case 1:
        Direction(c);
        if (forward)
        {
            if (SDL_abs(p->velocity_x) < 65536)
                Speed(c, 65536);
            else
                Decelerate(c, 8192);
        }
        break;
    case 2:
        if (WallImpact(c))
            return;
        // The original checks the left wall for both velocity signs.
        if ((p->contacts & 8U) && p->velocity_x != 0)
            p->mode = 1;
        if (SDL_abs(p->velocity_x) < 65536)
            p->mode = 1;
        if (Backward(c))
        {
            Direction(c);
            p->mode = 1;
            Speed(c, 65536);
            break;
        }
        if (p->animation.pose == 3)
            p->animation.remaining = 6;
        break;
    case 3:
        if (p->animation.pose == 3)
            p->animation.remaining = 6;
        break;
    }
    Gravity(c);
}
static void Swimming(Context *c)
{
    SotnWolfControllerFrame *f = &c->frame;
    SotnWolfJumpFrame *p = &f->motion;
    if (f->tapped & SOTN_BUTTON_JUMP)
    {
        Jump(c);
        f->swim_lock = 16;
        return;
    }
    if (p->contacts & 1U)
    {
        Stand(c);
        return;
    }
    if (!f->skill_enabled || !(p->buttons & SOTN_BUTTON_CANCEL) || f->water_level == 0)
    {
        Fall(c);
        return;
    }
    Speed(c, 32768);
    p->velocity_y = f->water_level > 12 ? -32768 : 0;
    if (p->buttons & SOTN_BUTTON_RIGHT)
    {
        p->facing_left = false;
        p->velocity_x = 32768;
    }
    if (p->buttons & SOTN_BUTTON_LEFT)
    {
        p->facing_left = true;
        p->velocity_x = -32768;
    }
    if (!(p->buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT | SOTN_BUTTON_UP)))
    {
        Decelerate(c, 1024);
        p->velocity_y = 32768;
    }
    const Sint32 target = p->velocity_y <= 0 ? 2112 : 1824;
    if (p->body_angle > target)
        p->body_angle -= 8;
    if (p->body_angle < target)
        p->body_angle += 8;
}
bool SotnWolfController_Tick(SotnWolfControllerFrame *frame, SotnDataView dra,
                             SotnWolfControllerEvents *out)
{
    if (!frame || !out || frame->motion.substep == 0 || frame->motion.substep == 8 ||
        frame->motion.substep > 9 || frame->motion.mode > 4 ||
        frame->motion.velocity_x < -16 * 65536 || frame->motion.velocity_x > 16 * 65536 ||
        frame->motion.velocity_y < -32 * 65536 || frame->motion.velocity_y > 32 * 65536 ||
        frame->motion.animation.id >= 247 || frame->motion.animation.pose >= 4096 ||
        frame->motion.animation.remaining < SDL_MIN_SINT16 ||
        frame->motion.animation.remaining > SDL_MAX_SINT16 || frame->mp > 9999 ||
        frame->priority > 511)
        return SDL_SetError(
            "Wolf locomotion requires bounded active controller and owner snapshots");
    Context c = {.frame = *frame, .dra = dra};
    SotnWolfControllerFrame *f = &c.frame;
    if (f->run_window)
        --f->run_window;
    if (f->swim_lock)
        --f->swim_lock;
    if (f->motion.substep != 9 && f->water_level > 12 && f->holy_symbol_enabled &&
        f->skill_enabled && !(f->motion.contacts & 1U) &&
        (f->motion.buttons & SOTN_BUTTON_CANCEL) && !f->swim_lock)
    {
        f->motion.substep = 9;
        f->motion.mode = 0;
        Animate(&c, 0xec);
        f->motion.velocity_y = 0;
    }
    f->joint_distance = 14;
    switch (f->motion.substep)
    {
    case 1:
        Idle(&c);
        break;
    case 2:
        Running(&c);
        break;
    case 3:
        Crouching(&c);
        break;
    case 4:
        Jumping(&c);
        break;
    case 5:
        Falling(&c);
        break;
    case 6:
        Recoiling(&c);
        break;
    case 7:
        Slide(&c);
        break;
    case 9:
        Swimming(&c);
        break;
    }
    if (c.failed || !SotnWolf_Cadence(f->motion.velocity_x, f->motion.body_angle, &f->cadence))
        return false;
    f->motion.body_angle = f->cadence.body_angle;
    f->previous_contacts = f->raw_contacts;
    *frame = *f;
    *out = c.events;
    return true;
}
