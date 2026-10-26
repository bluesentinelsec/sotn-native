// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted NO2 st/e_prisoner.h at sotn-decomp 7bc54d8157b6.
// Original initialization, animation and artwork tables remain private.
#include "actors_internal.h"

static void Opacity(Actor *shadow, bool enabled)
{
    shadow->state.opacity_set = enabled;
    shadow->state.opacity = enabled ? shadow->prisoner.opacity : 0;
}

/* Original high-half assignments retain each entity's independent fraction. */
static Sint32 Whole(Sint32 position, Sint32 whole)
{
    return (Sint32)((Sint64)Wrap16(whole) * 65536 + ((Uint32)position & 65535U));
}

static Actor *Parent(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || (Uint32)slot + 1U != a->slot)
        return NULL;
    Actor *parent = &s->slots[slot];
    return parent->state.id == 0x28 && !(parent->state.parameters & 0x10U) &&
                   parent->state.generation == a->enemy_state.owner_generation &&
                   parent->state.generation &&
                   parent->prisoner.child_generation == a->state.generation
               ? parent
               : NULL;
}

static void Substep(Actor *a, Uint16 step)
{
    a->substep = step;
    a->pose = a->remaining = 0;
}

bool ActorOlroxPrisoner(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4 || a->state.id != 0x28 || !a->state.generation)
        return SDL_SetError("NO2 prisoner requires its original active stage/actor owner");
    const bool shadow = (a->state.parameters & 0x10U) != 0;
    if (shadow && !Parent(s, a))
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x8b0))
            return false;
        a->state.frame = 0;
        a->state.priority = 30;
        if (a->state.parameters & 1U)
            ++a->state.palette;
        if (shadow)
        {
            a->state.blend_mode = 0x50;
            a->prisoner.opacity = 128;
            Opacity(a, true);
            a->state.priority = 31;
            a->state.step = 8;
        }
        else
        {
            Actor *child = NULL;
            if (!ActorOwnedSpawnAt(s, a, 0x28, (Uint16)(a->state.parameters + 0x10U),
                                   (Uint32)a->slot + 1U, &child))
                return false;
            child->state.x = Pixel(a->state.x) * 65536;
            child->state.y = Pixel(a->state.y) * 65536;
            child->state.facing_left = false;
            a->prisoner.child_generation = child->state.generation;
            a->state.step = 1;
        }
        return true;
    }
    if (shadow)
    {
        if (a->state.step != 8)
            return SDL_SetError("NO2 prisoner shadow entered an invalid state");
        const Actor *parent = Parent(s, a);
        a->state.x = Whole(a->state.x, Pixel(parent->state.x));
        a->state.y = Whole(a->state.y, Pixel(parent->state.y));
        return true;
    }
    if ((Uint32)a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("NO2 prisoner shadow reservation exceeds the actor pool");
    Actor *child = &s->slots[a->slot + 1U];
    if (child->state.id != 0x28 || !(child->state.parameters & 0x10U) ||
        child->state.generation != a->prisoner.child_generation || !Parent(s, child) ||
        child->enemy_state.owner_slot != a->slot ||
        child->enemy_state.owner_generation != a->state.generation)
        return SDL_SetError("NO2 prisoner lost its generation-owned shadow");
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 1:
        if (Distance(a, s->player) < 64)
        {
            if (!(ActorRandom(s) & (a->prisoner.appeared ? 15U : 1U)))
            {
                a->prisoner.appeared = true;
                a->prisoner.shift = (Sint16)((ActorRandom(s) & 3U) * 7U);
                if (ActorRandom(s) & 1U)
                    a->prisoner.shift = (Sint16)-a->prisoner.shift;
                a->state.x = Whole(a->state.x, Pixel(a->state.x) + a->prisoner.shift);
                a->state.step = 2;
            }
            else
                a->state.step = 6;
        }
        a->state.frame = child->state.frame = 0;
        break;
    case 2:
        if (!ActorAnimate(s, a, 0xe44, &animation))
            return false;
        child->state.frame = a->state.frame;
        --child->prisoner.opacity;
        Opacity(child, child->state.opacity_set);
        if (!child->prisoner.opacity)
            SetStep(a, 3);
        break;
    case 3:
        if (!ActorAnimate(s, a, 0xe58, &animation) ||
            (!animation && !ActorPositionalSound(s, a, 0x7c0)))
            return false;
        child->state.frame = (Uint16)(a->state.frame + 2U);
        child->state.priority = 106;
        child->state.blend_mode = 0;
        Opacity(child, false);
        child->state.rotated = child->state.scale_set = child->state.tinted = false;
        if (Distance(a, s->player) > 64)
        {
            child->state.blend_mode = 0x50;
            Opacity(child, true);
            child->state.priority = (Uint16)(a->state.priority + 1U);
            SetStep(a, 4);
        }
        break;
    case 4:
        switch (a->substep)
        {
        case 0:
            if (!ActorAnimate(s, a, 0xe68, &animation))
                return false;
            if (!animation)
            {
                if (ActorRandom(s) & 1U)
                    SetStep(a, 5);
                else
                    Substep(a, 1);
            }
            break;
        case 1:
            if (!ActorAnimate(s, a, 0xe7c, &animation))
                return false;
            if (!animation)
                Substep(a, Distance(a, s->player) < 64 ? 2 : 3);
            break;
        case 2:
            if (!ActorAnimate(s, a, 0xe8c, &animation))
                return false;
            if (!animation)
                SetStep(a, 3);
            break;
        case 3:
            if (!ActorAnimate(s, a, 0xe84, &animation))
                return false;
            if (!animation)
                SetStep(a, 5);
            break;
        default:
            return SDL_SetError("NO2 prisoner entered an invalid retreat substate");
        }
        child->state.frame = 0;
        break;
    case 5:
        if (!ActorAnimate(s, a, 0xe94, &animation))
            return false;
        child->state.frame = a->state.frame;
        ++child->prisoner.opacity;
        Opacity(child, child->state.opacity_set);
        if (child->prisoner.opacity > 128)
        {
            a->state.x = Whole(a->state.x, Pixel(a->state.x) - a->prisoner.shift);
            SetStep(a, 1);
        }
        break;
    case 6:
        a->state.frame = child->state.frame = 0;
        if (Distance(a, s->player) > 64)
            a->state.step = 1;
        break;
    default:
        return SDL_SetError("NO2 prisoner entered an invalid state");
    }
    return true;
}
