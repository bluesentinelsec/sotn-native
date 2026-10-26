// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted st/e_hammer.h at sotn-decomp 7bc54d8157b6; runtime arrays remain private.
#include "giant_knights_internal.h"

static bool Decide(SotnActors *s, Actor *a, const Actor *reference)
{
    if (a->giant.decision_wait)
        --a->giant.decision_wait;
    Sint32 dx = Pixel(reference->state.x) - Pixel(s->player.x);
    if (a->state.facing_left)
        dx = -dx;
    if (dx < -16)
    {
        GiantStep(a, 10);
        return true;
    }
    Actor *foot = NULL;
    Uint16 obstacle = 0;
    if (!GiantPart(s, a, a->giant.support_leg == 1 ? 10 : 13, &foot) ||
        !GiantAhead(s, foot, a->state.facing_left, &obstacle))
        return false;
    if (obstacle)
    {
        GiantStep(a, 7);
        return true;
    }
    if (!GiantAhead(s, foot, !a->state.facing_left, &obstacle))
        return false;
    if (obstacle)
    {
        GiantStep(a, 5);
        return true;
    }
    Uint16 next = a->state.step == 8 ? (dx < 80 ? 5 : 8) : dx < 80 ? 7 : dx > 160 ? 8 : 5;
    if (!a->giant.decision_wait && dx < 96)
    {
        a->giant.decision_wait = 3;
        next = 6;
    }
    if (next != a->state.step)
        GiantStep(a, next);
    return true;
}
static bool Complete(const Actor *a)
{
    return !a->giant.pose[0] && !a->giant.remaining[0];
}
static bool AttackPose(SotnActors *s, Actor *a, size_t list, size_t pose, Sint16 duration)
{
    Actor *hammer = NULL, *upper = NULL, *lower = NULL;
    if (!GiantMotion(s, a, list, pose, 2) || !GiantPart(s, a, 15, &hammer) ||
        !GiantPart(s, a, 7, &upper) || !GiantPart(s, a, 8, &lower))
        return false;
    lower->state.x = hammer->state.x;
    lower->state.y = hammer->state.y;
    return GiantSolve(s, a, upper, lower) && GiantBend(s, a, upper, duration) &&
           GiantBend(s, a, lower, duration) && GiantParts(s, a, 0x2244, false);
}
bool GiantHammer(SotnActors *s, Actor *a)
{
    if ((a->state.step & 1U) && (a->hit_flags & 3U))
        GiantStep(a, 12);
    if ((a->state.flags & 0x100U) && a->state.step < 24)
    {
        if (!ActorPositionalSound(s, a, 0x745))
            return false;
        GiantStep(a, 24);
    }
    size_t list = a->giant.support_leg == 1 ? 0x22dc : 0x22f4;
    Actor *foot = NULL;
    SotnCollider collider = {.top = 9};
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x928))
            return false;
        a->state.frame = 3;
        a->state.hitbox_width = a->state.hitbox_height = 6;
        SDL_FALLTHROUGH;
    case 1:
        if ((Sint64)a->state.x + a->vx < SDL_MIN_SINT32 ||
            (Sint64)a->state.x + a->vx > SDL_MAX_SINT32 ||
            (Sint64)a->state.y + a->vy < SDL_MIN_SINT32 ||
            (Sint64)a->state.y + a->vy > SDL_MAX_SINT32)
            return SDL_SetError("Hammer floor motion exceeds fixed-point bounds");
        if (!ActorGround(s, a, 0x219c, &collision))
            return false;
        if (collision & 1U)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            ++a->state.step;
        }
        // Original pre-construction epilogue touches empty reserved storage only.
        return true;
    case 2:
        if (!GiantCreate(s, a, false))
            return false;
        ++a->state.step;
        SDL_FALLTHROUGH;
    case 3: {
        Actor *upper = NULL, *lower = NULL, *hammer = NULL;
        GiantClearPose(a);
        if (!GiantPart(s, a, 4, &upper) || !GiantPart(s, a, 5, &lower) ||
            !GiantPart(s, a, 15, &hammer))
            return false;
        upper->giant.rotation = 0x100;
        lower->giant.rotation = 0x680;
        hammer->giant.rotation = 0x2c0;
        GiantStep(a, 5);
    }
        SDL_FALLTHROUGH;
    case 5:
    case 7:
        if (!GiantMotion(s, a, list, a->state.step == 5 ? 0x23d8 : 0x2410, 2) ||
            !GiantParts(s, a, 0x2244, false))
            return false;
        if (a->giant.pose[0] > 1)
        {
            if (!GiantListed(s, a, list, 3, &foot) || !GiantGround(s, a, foot, &collider))
                return false;
            if (foot->giant.terrain)
            {
                const bool retreat = a->state.step == 7;
                ShiftY(foot, collider.top);
                a->giant.support_leg ^= 1U;
                GiantClearPose(a);
                if (retreat)
                {
                    if (!GiantAhead(s, foot, !a->state.facing_left, &collision))
                        return false;
                    if (collision)
                        GiantStep(a, 5);
                    else if (!Decide(s, a, a))
                        return false;
                }
                else if (!Decide(s, a, foot))
                    return false;
            }
            else if (a->state.step == 5 && a->giant.pose[0] == 2 && a->giant.remaining[0] == 1)
                GiantStep(a, 8);
        }
        break;
    case 8:
        switch (a->substep)
        {
        case 0:
            list = a->giant.support_leg == 1 ? 0x2264 : 0x2280;
            if (!GiantMotion(s, a, list, 0x2378, 2))
                return false;
            if (Complete(a))
                ++a->substep;
            break;
        case 1:
            if (!GiantMotion(s, a, list, 0x24c0, 2))
                return false;
            if (Complete(a))
                ++a->substep;
            break;
        case 2:
            a->vx = a->state.facing_left ? 2 * 65536 : -2 * 65536;
            a->vy = -49152;
            GiantClearPose(a);
            ++a->substep;
            SDL_FALLTHROUGH;
        case 3:
            collider.top = 10;
            if (!GiantMove(a, 24576) || !GiantMotion(s, a, list, 0x24e8, 3) ||
                !GiantListed(s, a, list, 3, &foot) || !GiantGround(s, a, foot, &collider))
                return false;
            if (foot->giant.terrain)
            {
                if (!ActorPositionalSound(s, a, 0x648))
                    return false;
                ShiftY(foot, collider.top + 1);
                a->giant.support_leg ^= 1U;
                GiantClearPose(a);
                a->substep = 1;
                if (!Decide(s, a, foot))
                    return false;
            }
            break;
        default:
            return SDL_SetError("Hammer entered an invalid leap substate");
        }
        if (!GiantParts(s, a, 0x2244, false))
            return false;
        break;
    case 6:
        list = a->giant.support_leg == 1 ? 0x230c : 0x2324;
        switch (a->substep)
        {
        case 0:
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (!AttackPose(s, a, list, 0x265c, 16))
                return false;
            if (Complete(a))
            {
                if (!ActorPositionalSound(s, a, 0x743))
                    return false;
                ++a->substep;
            }
            break;
        case 2:
            if (!AttackPose(s, a, list, 0x266c, 2))
                return false;
            if (a->giant.pose[0] == 4 && !a->giant.remaining[0])
                a->giant.support_leg ^= 1U;
            if (a->giant.pose[0] == 6 && !a->giant.remaining[0] &&
                !ActorPositionalSound(s, a, 0x65d))
                return false;
            // The shared native camera does not yet expose the original small-Y shake.
            if (Complete(a))
            {
                a->giant.support_leg ^= 1U;
                ++a->substep;
            }
            break;
        case 3:
            if (!GiantMotion(s, a, list, 0x265c, 2) || !GiantParts(s, a, 0x2244, false))
                return false;
            if (Complete(a))
                ++a->substep;
            break;
        case 4:
            if (!Decide(s, a, a))
                return false;
            break;
        default:
            return SDL_SetError("Hammer entered an invalid attack substate");
        }
        break;
    case 10:
        if (!GiantMotion(s, a, list, 0x2520, 2) || !GiantParts(s, a, 0x2244, false))
            return false;
        if (!a->giant.remaining[0])
        {
            a->state.facing_left = !a->state.facing_left;
            if (!Decide(s, a, a))
                return false;
        }
        break;
    case 12:
        if (!a->substep)
        {
            if (!ActorPositionalSound(s, a, 0x744))
                return false;
            ++a->substep;
        }
        list = a->giant.support_leg == 1 ? 0x2264 : 0x2280;
        if (!GiantMotion(s, a, list, 0x2700, 2) || !GiantParts(s, a, 0x2244, false) ||
            (Complete(a) && !Decide(s, a, a)))
            return false;
        break;
    case 24:
        if (!a->substep)
        {
            if (!GiantDisable(s, a, 0x2244, true))
                return false;
            a->vy = -2 * 65536;
            a->state.hitbox_state = 0;
            a->state.flags |= 0x80000000U;
            ++a->substep;
        }
        if (a->substep != 1 || !GiantMove(a, 12288))
            return SDL_SetError("Hammer entered an invalid death motion");
        return (s->service_ticks & 7U) ||
               ActorPositionalSound(s, a, (ActorRandom(s) & 1U) ? 0x65b : 0x657);
    default:
        return SDL_SetError("Hammer entered an invalid body state");
    }
    return GiantParts(s, a, 0x2244, true);
}
