// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/71830.c:ControlMistForm/MistFormFinished and 6DF70.c:CheckMoveDirection,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Private art stays private.
#include <sotn/mist_motion.h>
#include <sotn/player.h>

static Sint32 Decelerate(Sint32 velocity, Sint32 amount)
{
    return velocity > 0   ? SDL_max(0, velocity - amount)
           : velocity < 0 ? SDL_min(0, velocity + amount)
                          : 0;
}
static bool Direction(SotnMistMotion *motion, SotnMistInput input)
{
    const bool previous = motion->facing_left;
    if (!input.direction_locked)
    {
        if (input.directions & SOTN_BUTTON_RIGHT)
            motion->facing_left = false;
        else if (input.directions & SOTN_BUTTON_LEFT)
            motion->facing_left = true;
    }
    return previous != motion->facing_left;
}
static Sint32 Axis(Sint32 velocity, Sint32 target, Sint32 deceleration)
{
    return target > 0 ? (velocity > target ? Decelerate(velocity, deceleration) : target)
                      : (velocity < target ? Decelerate(velocity, deceleration) : target);
}
bool SotnMistMotion_Tick(SotnMistMotion *motion, SotnMistInput input, SotnMistMotionEvents *out)
{
    const Uint16 mask = SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT;
    if (!motion || !out || !input.world || motion->phase > 1 || motion->ended ||
        (input.directions & (Uint16)~mask))
        return SDL_SetError("Mist movement requires a live bounded cursor and direction snapshot");
    if (!SotnWorld_Validate(input.world))
        return false;
    SotnMistMotion next = *motion;
    SotnMistMotionEvents events = {0};
    const bool power = (input.world->relics[SOTN_RELIC_POWER_MIST] & 2U) != 0;
    const Sint32 ortho = power ? 65536 : 32768, diagonal = power ? 40960 : 20480;
    if (next.phase)
    {
        bool ended = input.underwater || input.cancel;
        if (!ended)
        {
            SotnTransformationCharge charge = {0};
            if (!SotnTransformation_MP(input.world, SOTN_FORM_MIST, input.mp, input.game_timer,
                                       true, &charge))
                return false;
            events.spent = charge.spent;
            ended = !charge.allowed;
        }
        if (!ended && !power)
        {
            if (next.remaining)
                --next.remaining;
            ended = next.remaining == 0;
        }
        if (ended)
        {
            events.turned = Direction(&next, input);
            events.ended = next.ended = true;
            next.phase = 0;
            *motion = next;
            *out = events;
            return true;
        }
    }
    if (!next.phase)
    {
        events.turned = Direction(&next, input);
        if (next.vx)
            next.vx = next.vx > 0 ? ortho : -ortho;
        if (next.vy)
            next.vy = next.vy > 0 ? ortho : -ortho;
        next.remaining = power ? 100000 : 16;
        next.phase = 1;
        events.started = true;
    }
    else
    {
        Sint32 x = 0, y = 0;
        bool moving = true;
        switch (input.directions)
        {
        case SOTN_BUTTON_UP:
            y = -ortho;
            break;
        case SOTN_BUTTON_DOWN:
            y = ortho;
            break;
        case SOTN_BUTTON_RIGHT:
            x = ortho;
            next.facing_left = false;
            break;
        case SOTN_BUTTON_LEFT:
            x = -ortho;
            next.facing_left = true;
            break;
        case SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT:
            x = diagonal;
            y = -diagonal;
            next.facing_left = false;
            break;
        case SOTN_BUTTON_UP | SOTN_BUTTON_LEFT:
            x = -diagonal;
            y = -diagonal;
            next.facing_left = true;
            break;
        case SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT:
            x = diagonal;
            y = diagonal;
            next.facing_left = false;
            break;
        case SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT:
            x = -diagonal;
            y = diagonal;
            next.facing_left = true;
            break;
        default:
            moving = false;
            break;
        }
        const Sint32 deceleration = x && y ? 512 : 768;
        next.vx = moving && x ? Axis(next.vx, x, deceleration) : Decelerate(next.vx, 768);
        next.vy = moving && y ? Axis(next.vy, y, deceleration) : Decelerate(next.vy, 768);
    }
    if (next.settle_y)
    {
        events.shift_y = (Sint16)(next.settle_y > 0 ? 1 : -1);
        next.settle_y = (Sint16)(next.settle_y - events.shift_y);
    }
    *motion = next;
    *out = events;
    return true;
}
