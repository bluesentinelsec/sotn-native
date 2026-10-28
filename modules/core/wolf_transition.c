// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/wolfForm.c:WolfStepS_0/WolfStepS_8/PlayerStepUnmorphWolf,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork stays private.
#include <SDL3/SDL_error.h>
#include <sotn/wolf_transition.h>
static Sint32 Decelerate(Sint32 value)
{
    return value > 0 ? SDL_max(0, value - 8192) : value < 0 ? SDL_min(0, value + 8192) : 0;
}
bool SotnWolfTransition_Tick(SotnWolfTransition *transition, SotnWolfTransitionInput input,
                             SotnWolfTransitionEvents *out)
{
    if (!transition || !out || transition->bank > 31 || input.center_priority < 2 ||
        input.center_priority > 511 || transition->player.priority > 511 ||
        transition->player.motion.mode > 4 || transition->player.motion.velocity_x < -16 * 65536 ||
        transition->player.motion.velocity_x > 16 * 65536 ||
        transition->player.motion.velocity_y < -16 * 65536 ||
        transition->player.motion.velocity_y > 16 * 65536 ||
        (transition->returning
             ? transition->player.motion.substep > 2
             : transition->player.motion.substep != 0 && transition->player.motion.substep != 8))
        return SDL_SetError(
            "Wolf transition requires bounded actual player/sensor/child snapshots");
    for (Uint16 i = 0; i < 4; ++i)
        if (transition->floor_y[i] < -64 || transition->floor_y[i] > 64 ||
            transition->ceiling_y[i] < -64 || transition->ceiling_y[i] > 64)
            return SDL_SetError("Wolf transition sensor leaves original bounded geometry");
    SotnWolfTransition next = *transition;
    SotnWolfControllerFrame *p = &next.player;
    SotnWolfJumpFrame *m = &p->motion;
    SotnWolfTransitionEvents events = {0};
    if (!next.returning)
    {
        if (!m->substep)
        {
            if (!input.mist_child)
            {
                events.entry_flash = true;
                if (!input.entry_factory_created)
                    goto done;
                events.clear_weapon = true;
                next.signal = (Uint16)(next.signal + 1U);
            }
            next.saved_sprite = m->animation.sprite;
            next.saved_bank = next.bank;
            next.timer14 = SDL_max(next.timer14, 4);
            events.replace_body = true;
            if (!SotnAnimation_Set(&m->animation, 0xe8))
                return false;
            m->mode = 0;
            p->coyote_ticks = 8;
            m->animation.pose = 4;
            m->animation.remaining = 4;
            m->substep = 8;
            m->body_angle = 2048;
            p->priority = (Uint16)(input.center_priority - 2U);
            next.rotation = 0;
            SDL_memset(next.movement, 0, sizeof(next.movement));
            p->direction_locked = false;
            p->run_window = 0;
            m->velocity_x = m->velocity_y = 0;
            next.texture = 0x7e;
            next.bank = 15;
            next.palette = 0x810d;
        }
        else
        {
            const Sint16 floor[4] = {29, 25, 25, 25};
            next.palette = 0x810d;
            for (Uint16 i = 0; i < 4; ++i)
                if (next.floor_y[i] < floor[i])
                    ++next.floor_y[i];
            next.timer14 = SDL_max(next.timer14, 4);
            m->velocity_y = 0;
            m->animation.remaining = 4;
            if (input.mist_child)
            {
                if (!input.mist_ready)
                    goto done;
                events.finish_flash = true;
            }
            else if (next.signal != 3)
                goto done;
            next.palette = 0x104;
            m->substep = 5;
            m->velocity_y = -98304;
        }
    }
    else
    {
        m->velocity_x = Decelerate(m->velocity_x);
        if (m->contacts & 3U)
            m->velocity_y = 0;
        m->velocity_y = Decelerate(m->velocity_y);
        if (!m->substep)
        {
            Uint16 ready = 0;
            for (Uint16 i = 0; i < 4; ++i)
                if (next.ceiling_y[i] > -22)
                    --next.ceiling_y[i];
                else
                    ++ready;
            if ((m->contacts & 3U) == 3)
            {
                next.cramped = true;
                m->velocity_x = m->velocity_y = 0;
            }
            if (ready == 4)
            {
                next.bank = 1;
                next.texture = 0;
                next.draw_flags = 0;
                next.rotation = 0;
                if (!SotnAnimation_Set(&m->animation, 0xca))
                    return false;
                next.signal = 1;
                m->substep = next.cramped ? 2 : 1;
                next.form_pose = next.cramped ? 0xc7 : 0x5f;
            }
        }
        else if (next.signal == 3)
        {
            // The caller now invokes the real normal/stuck helper, which owns final sensors,
            // velocity, priority and lockout. Reporting a request does not complete that helper.
            events.normal_return = m->substep == 1;
            events.stuck_return = m->substep == 2;
            if (events.stuck_return)
                p->priority = input.center_priority;
        }
    }
done:
    *transition = next;
    *out = events;
    return true;
}
