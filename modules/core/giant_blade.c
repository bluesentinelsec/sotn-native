// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted st/e_blade.h at sotn-decomp 7bc54d8157b6. Original arrays stay private.
#include "giant_knights_internal.h"

// The shared header names data in another overlay; NO2 stores it 0x7dc bytes earlier.
static size_t Table(size_t original)
{
    return original - 0x7dcU;
}
static bool Complete(const Actor *a)
{
    return !a->giant.pose[0] && !a->giant.remaining[0];
}
static bool Swords(SotnActors *s, Actor *a)
{
    for (Uint16 i = 15; i <= 16; ++i)
    {
        Actor *sword = NULL, *parent = NULL;
        if (!GiantPart(s, a, i, &sword) || !GiantPart(s, a, sword->giant.parent, &parent))
            return false;
        sword->giant.rotation = (Sint16)Wrap16((Sint32)parent->giant.rotation - 0x600);
    }
    return true;
}
static Uint16 Decision(const SotnActors *s, const Actor *a)
{
    const Sint64 distance = Distance(a, s->player);
    Uint16 next = 0;
    switch (a->state.step)
    {
    case 8:
        if (distance < 64)
            next = 10;
        if (distance > 80)
            next = 6;
        if (a->giant.terrain)
            next = 7;
        break;
    case 10:
        if (distance < 88)
            next = 8;
        if (distance < 56)
            next = 7;
        if (distance > 80)
            next = 6;
        if (a->giant.terrain)
            next = 7;
        break;
    case 12:
        if (distance < 64)
            next = 10;
        break;
    case 6:
        if (a->giant.terrain)
            next = 7;
        if (distance < 48)
            next = 10;
        if (distance < 80)
            next = 8;
        break;
    default:
        if (distance > 80)
            next = 6;
        if (distance < 64)
            next = 10;
        if (a->giant.terrain)
            next = 7;
        break;
    }
    if (a->state.facing_left != (Side(a, s->player) == 0))
        next = 12;
    return next;
}
static void Decide(SotnActors *s, Actor *a, bool fallback)
{
    const Uint16 next = Decision(s, a);
    if (next || fallback)
        GiantStep(a, next ? next : 5);
}
static bool Pose(SotnActors *s, Actor *a, size_t list, size_t pose, bool swords)
{
    return GiantMotion(s, a, Table(list), Table(pose), 4) && (!swords || Swords(s, a)) &&
           GiantParts(s, a, Table(0x3494), false);
}
static bool LookAhead(SotnActors *s, Actor *a, const Actor *foot)
{
    return GiantAhead(s, foot, a->state.facing_left, &a->giant.terrain);
}
static bool SwordTrail(SotnActors *s, Actor *a, Uint8 active)
{
    for (Uint16 i = 15; i <= 16; ++i)
    {
        Actor *sword = NULL;
        if (!GiantPart(s, a, i, &sword))
            return false;
        sword->giant.attack_active = active;
    }
    return true;
}
bool GiantBlade(SotnActors *s, Actor *a)
{
    if ((a->state.step & 1U) && (a->hit_flags & 3U))
        GiantStep(a, 14);
    if ((a->state.flags & 0x100U) && a->state.step < 24)
    {
        Actor *front = NULL, *back = NULL;
        if (!GiantPart(s, a, 10, &front) || !GiantPart(s, a, 13, &back))
            return false;
        if (front->giant.terrain || back->giant.terrain)
        {
            if (!ActorPositionalSound(s, a, 0x750))
                return false;
            GiantStep(a, 24);
        }
    }
    size_t list = a->giant.support_leg == 1 ? 0x3530 : 0x3548;
    Actor *foot = NULL, *sword = NULL;
    SotnCollider collider = {.top = 9};
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x958))
            return false;
        a->state.frame = 0;
        a->state.facing_left = a->state.parameters != 0;
        a->state.hitbox_width = a->state.hitbox_height = 6;
        SDL_FALLTHROUGH;
    case 1:
        if ((Sint64)a->state.x + a->vx < SDL_MIN_SINT32 ||
            (Sint64)a->state.x + a->vx > SDL_MAX_SINT32 ||
            (Sint64)a->state.y + a->vy < SDL_MIN_SINT32 ||
            (Sint64)a->state.y + a->vy > SDL_MAX_SINT32)
            return SDL_SetError("Blade floor motion exceeds fixed-point bounds");
        if (!ActorGround(s, a, Table(0x33e4), &collision))
            return false;
        if (collision & 1U)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            ++a->state.step;
        }
        return true;
    case 2:
        if (!GiantCreate(s, a, true))
            return false;
        ++a->state.step;
        SDL_FALLTHROUGH;
    case 3:
        GiantClearPose(a);
        GiantStep(a, 5);
        SDL_FALLTHROUGH;
    case 5:
    case 7:
        if (a->state.step == 7)
        {
            if (!a->substep)
            {
                a->giant.pose[0] = 1;
                ++a->substep;
            }
            list = a->giant.support_leg == 1 ? 0x3560 : 0x3570;
        }
        if (!Pose(s, a, list, a->state.step == 7 ? 0x3638 : 0x3600, true))
            return false;
        if (a->giant.pose[0] > 1)
        {
            if (!GiantListed(s, a, Table(list), 3, &foot) || !GiantGround(s, a, foot, &collider))
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
                        GiantStep(a, 6);
                }
                if (!LookAhead(s, a, foot))
                    return false;
                Decide(s, a, false);
            }
        }
        break;
    case 6:
        switch (a->substep)
        {
        case 0:
            list = a->giant.support_leg == 1 ? 0x34b8 : 0x34d8;
            if (!GiantMotion(s, a, Table(list), Table(0x38c4), 4) || !Swords(s, a))
                return false;
            if (Complete(a))
                ++a->substep;
            // Preserve the original unconditional extra increment.
            ++a->substep;
            break;
        case 1:
            if (!GiantMotion(s, a, Table(list), Table(0x3700), 4) || !Swords(s, a))
                return false;
            if (Complete(a))
                ++a->substep;
            break;
        case 2:
            a->vx = a->state.facing_left ? 163840 : -163840;
            a->vy = -65536;
            GiantClearPose(a);
            ++a->substep;
            SDL_FALLTHROUGH;
        case 3:
            if (!GiantMove(a, 5632) || !GiantMotion(s, a, Table(list), Table(0x3720), 3) ||
                !Swords(s, a) || !GiantListed(s, a, Table(list), 3, &foot) ||
                !GiantGround(s, a, foot, &collider))
                return false;
            if (foot->giant.terrain)
            {
                if (!ActorPositionalSound(s, a, 0x648))
                    return false;
                ShiftY(foot, collider.top);
                a->giant.support_leg ^= 1U;
                GiantClearPose(a);
                if (!LookAhead(s, a, foot))
                    return false;
                const Uint16 next = Decision(s, a);
                if (next)
                    GiantStep(a, next);
                else
                    a->substep = 1;
            }
            break;
        default:
            return SDL_SetError("Blade entered an invalid leap substate");
        }
        if (!GiantParts(s, a, Table(0x3494), false))
            return false;
        break;
    case 8:
        list = a->giant.support_leg == 1 ? 0x34b8 : 0x34d8;
        switch (a->substep)
        {
        case 0:
            if (!SwordTrail(s, a, 1))
                return false;
            a->giant.attack_active = 0;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (!Pose(s, a, list, 0x386c, false) || !GiantPart(s, a, 16, &sword))
                return false;
            if (sword->hit_flags)
                a->giant.attack_active = 1;
            if (Complete(a))
                a->substep = a->giant.attack_active ? 3 : 2;
            break;
        case 2:
            if (!GiantMotion(s, a, Table(list), Table(0x389c), 4))
                return false;
            if (a->giant.pose[0] == 1 && !a->giant.remaining[0] &&
                !ActorPositionalSound(s, a, 0x66e))
                return false;
            if (!GiantParts(s, a, Table(0x3494), false))
                return false;
            if (Complete(a))
            {
                if (!SwordTrail(s, a, 0))
                    return false;
                ++a->substep;
            }
            break;
        case 3:
            if (!Pose(s, a, list, 0x35a8, false))
                return false;
            if (Complete(a))
                Decide(s, a, true);
            break;
        default:
            return SDL_SetError("Blade entered an invalid sword substate");
        }
        break;
    case 10:
        if (!a->substep)
        {
            if (!ActorPositionalSound(s, a, 0x74e))
                return false;
            ++a->substep;
        }
        list = a->giant.support_leg == 1 ? 0x34f8 : 0x3514;
        if (!Pose(s, a, list, 0x3934, true) || !GiantListed(s, a, Table(list), 5, &foot))
            return false;
        foot->giant.attack_active = 1;
        if (Complete(a))
        {
            foot->giant.attack_active = 0;
            Decide(s, a, true);
        }
        break;
    case 12:
        if (!Pose(s, a, list, 0x3760, true))
            return false;
        if (!a->giant.remaining[0])
        {
            a->state.facing_left = !a->state.facing_left;
            a->giant.terrain = 0;
            Decide(s, a, true);
        }
        break;
    case 14:
        if (!a->substep)
        {
            if (!ActorPositionalSound(s, a, 0x74f))
                return false;
            ++a->substep;
        }
        list = a->giant.support_leg == 1 ? 0x34b8 : 0x34d8;
        if (!Pose(s, a, list, 0x3990, false))
            return false;
        if (Complete(a))
            Decide(s, a, true);
        break;
    case 24:
        switch (a->substep)
        {
        case 0:
            if (!GiantDisable(s, a, Table(0x3494), false))
                return false;
            a->state.hitbox_state = 0;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            list = a->giant.support_leg == 1 ? 0x34b8 : 0x34d8;
            if (!Pose(s, a, list, 0x3a24, false))
                return false;
            if (a->giant.pose[0] == 3 && !a->giant.remaining[0] &&
                !ActorPositionalSound(s, a, 0x646))
                return false;
            if (Complete(a))
                ++a->substep;
            break;
        case 2:
            if (!GiantMotion(s, a, Table(0x3580), Table(0x3a4c), 1) ||
                !GiantParts(s, a, Table(0x3494), false))
                return false;
            if (Complete(a))
            {
                if (!ActorPositionalSound(s, a, 0x625) || !GiantPart(s, a, 15, &sword))
                    return false;
                sword->giant.released = 1;
                a->giant.released =
                    1; // Retain ownership history after the thrown slot is released.
                sword->giant.rotation = sword->state.rotation = 0x400;
                a->giant.timer = 32;
                ++a->substep;
            }
            break;
        case 3:
            if (a->giant.timer <= 0)
                return SDL_SetError("Blade death wait has an invalid timer");
            if (!--a->giant.timer)
                ++a->substep;
            break;
        case 4:
            if (!GiantDisable(s, a, Table(0x3494), true))
                return false;
            a->giant.timer = 64;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 5:
            if (!(s->service_ticks & 7U) &&
                !ActorPositionalSound(s, a, (ActorRandom(s) & 1U) ? 0x65b : 0x657))
                return false;
            if (a->giant.timer <= 0)
                return SDL_SetError("Blade death completion has an invalid timer");
            if (!--a->giant.timer)
                a->remove = true;
            return true;
        default:
            return SDL_SetError("Blade entered an invalid death substate");
        }
        break;
    default:
        return SDL_SetError("Blade entered an invalid body state");
    }
    return GiantParts(s, a, Table(0x3494), true);
}
