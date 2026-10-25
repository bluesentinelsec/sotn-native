// SPDX-License-Identifier: AGPL-3.0-or-later
// NO1 unk_3B5E0.c selected elevator rules, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "actors_internal.h"
static bool Rider(SotnActors *s, const Actor *a, Uint16 buttons, bool position)
{
    s->motion.protect = true;
    s->control = (SotnActorControl){.buttons = buttons, .timer = 2};
    if (position)
        return ActorDisplace(s, 0, (Pixel(a->state.y) - 4 - Pixel(s->player.y)) * 65536,
                             a->state.step == 4 ? 0x41 : 0);
    return true;
}
static bool Slam(SotnActors *s, Actor *a, bool top)
{
    if (!ActorSound(s, 0x7b3))
        return false;
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + (top ? -24 : 36), &c))
        return false;
    ShiftY(a, top ? c.bottom : c.top);
    for (Uint16 i = 0; i < 3; ++i)
    {
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 6, 0x10, 160, 192, &effect))
            return false;
        if (effect != NULL)
        {
            effect->state.x = (Pixel(a->state.x) + (Sint32)i * 8 - 8) * 65536;
            effect->state.y = (Pixel(a->state.y) + (top ? -24 : 36)) * 65536;
            effect->state.rotated = true;
            effect->state.rotation = (Sint16)(1024 - (Sint32)i * 1024);
            effect->state.facing_left = false;
        }
    }
    return true;
}
static bool Travel(SotnActors *s, Actor *a, Sint32 target)
{
    const bool up = a->state.step == 4;
    const bool riding = !a->outer_lift.called;
    if (riding && a->outer_lift.wait)
    {
        if (!Rider(s, a, SOTN_BUTTON_DOWN, true))
            return false;
        a->state.y += a->vy;
        --a->outer_lift.wait;
    }
    else
    {
        a->state.y += a->vy;
        if (riding && !Rider(s, a, SOTN_BUTTON_DOWN, true))
            return false;
    }
    const Sint32 y = Pixel(a->state.y);
    switch (a->substep)
    {
    case 0:
        a->vy += up ? -8192 : 16384;
        // Spark recipes/presentation remain separate; do not invent replacement particles.
        if ((up && y < target) || (!up && y > target))
        {
            if ((up && a->outer_lift.target == 2) || (!up && a->outer_lift.target == 0))
            {
                if (!Slam(s, a, up))
                    return false;
                a->vy = up ? -a->vy : -a->vy / 2;
                a->substep = 2;
            }
            else
            {
                if (!ActorSound(s, 0x675))
                    return false;
                a->outer_lift.moving = true;
                a->vy /= 2;
                a->substep = 1;
            }
            if (!up)
                a->outer_lift.moving = true;
        }
        break;
    case 1:
        a->vy += up ? 65536 : -98304;
        if ((up && y > target + 8) || (!up && y < target))
        {
            a->vy = 0;
            a->substep = 3;
        }
        break;
    case 2:
        if (up)
        {
            a->vy -= 2 * 65536;
            if (a->vy < 0)
            {
                a->vy = 0;
                a->substep = 3;
            }
        }
        else
        {
            a->vy += 65536;
            if (a->vy > 0)
            {
                a->vy -= 49152;
                if (y > target)
                {
                    if (!Slam(s, a, false))
                        return false;
                    a->vy = 0;
                    a->substep = 3;
                }
            }
        }
        break;
    case 3:
        a->outer_lift.moving = false;
        if (y == target)
            SetStep(a, 6);
        else
        {
            // The original writes only the integer half, retaining the fractional velocity.
            const Sint32 fraction = (Sint32)((Uint32)a->vy & 65535U);
            a->vy = fraction + (y > target ? -65536 : 65536);
        }
        break;
    default:
        return SDL_SetError("Outer Wall elevator entered an invalid travel substate");
    }
    return true;
}
static bool Shadow(SotnActors *s, Actor *a)
{
    if (!a->state.primitive_count)
        return true;
    if (!ActorSprite(s, a, 0, a->state.x - 16 * 65536, a->state.y + 28 * 65536, 208, 192, 31, 15,
                     0xe, 0x4a))
        return false;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = (Pixel(a->state.y) + 36) * 65536;
    p->state.width = 32;
    p->state.height = 16;
    p->state.red = p->state.green = p->state.blue = 48;
    p->state.modulate = p->state.blend = true;
    const Sint32 bottom = Pixel(a->state.y) + 44;
    p->state.active =
        (!a->vy || (s->ticks % 2)) && bottom <= 0x810 && !(bottom > 0x610 && bottom < 0x650);
    return true;
}

static bool Cart(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 2 || a->outer_lift.current > 2 || a->outer_lift.target > 2)
        return SDL_SetError("Outer Wall elevator stop is outside its private three-stop route");
    SotnDataView stops = {0};
    if (!Slice(s->nz0, 0x1568, 8, &stops))
        return false;
    const Sint32 target = Signed16(stops.bytes + a->outer_lift.target * 2U);
    if (target < 0 || target > 4096)
        return SDL_SetError("Outer Wall elevator destination exceeds its private bounds");
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0: {
        if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS || s->slots[a->slot + 1].state.generation)
            return SDL_SetError("Outer Wall elevator requires its free adjacent rope actor");
        if (!ActorInitialize(s, a, 0x9f8))
            return false;
        a->outer_lift.current = a->outer_lift.target = (Uint8)a->state.parameters;
        a->state.priority = 0xa4;
        // Keep the original forty-slot visual reservation; complete spark effects are pending.
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        if (available >= 40 && !ActorPrimitives(s, a, 40))
            return false;
        Actor *rope = NULL;
        if (!ActorSpawnAt(s, a, 0x2a, 0, a->slot + 1U, &rope))
            return false;
        break;
    }
    case 1:
        ++a->state.step;
        break;
    case 2: {
        a->vy = 0;
        const bool ready =
            (s->player.step == SOTN_PLAYER_STAND || s->player.step == SOTN_PLAYER_CROUCH) &&
            !s->player.attack.active && !s->player.shield.active && !s->player.subweapon_starting;
        if (Distance(a, s->player) < 6 &&
            SDL_abs((Sint64)Pixel(a->state.y) - Pixel(s->player.y)) < 16 && ready)
        {
            Sint32 direction = 0;
            if (s->buttons & SOTN_BUTTON_UP)
                direction = 1;
            if (s->buttons & SOTN_BUTTON_DOWN)
                direction = -1;
            const Sint32 next = a->outer_lift.current + direction;
            if (direction && next >= 0 && next <= 2)
            {
                a->outer_lift.target = (Uint8)next;
                s->motion.stop_velocity = true;
                if (!Rider(s, a, 0, false))
                    return false;
                SetStep(a, 3);
            }
        }
        if (a->outer_lift.called)
        {
            SetStep(a, 3);
            a->substep = 1;
        }
        break;
    }
    case 3:
        if (!a->outer_lift.called)
        {
            s->motion.protect = true;
            s->control = (SotnActorControl){.timer = 64};
        }
        if (!ActorAnimate(s, a, 0x1550, &animation))
            return false;
        if (!animation)
        {
            if (!ActorSound(s, 0x7b5) ||
                (!a->outer_lift.called && a->outer_lift.target < a->outer_lift.current &&
                 !ActorSound(s, 0x6ed)))
                return false;
            a->outer_lift.wait = 24;
            SetStep(a, a->outer_lift.target > a->outer_lift.current ? 4 : 5);
        }
        break;
    case 4:
    case 5:
        if (!Travel(s, a, target))
            return false;
        break;
    case 6:
        switch (a->substep)
        {
        case 0:
            if (!a->outer_lift.called &&
                !ActorDisplace(s, 0, (Pixel(a->state.y) - 8 - Pixel(s->player.y)) * 65536, 0))
                return false;
            a->vy = 0;
            ++a->substep;
            break;
        case 1:
            if (!a->outer_lift.called && !Rider(s, a, 0, false))
                return false;
            if (!ActorAnimate(s, a, 0x155c, &animation))
                return false;
            if (!animation)
            {
                a->pose = a->remaining = 0;
                ++a->substep;
            }
            break;
        case 2:
            a->outer_lift.current = a->outer_lift.target;
            a->outer_lift.called = false;
            SetStep(a, 2);
            break;
        default:
            return SDL_SetError("Outer Wall elevator arrival substate is invalid");
        }
        break;
    default:
        return SDL_SetError("Outer Wall elevator entered an invalid original state");
    }
    return Shadow(s, a);
}
bool ActorOuterElevator(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x29)
        return Cart(s, a);
    if (a->state.id == 0x2a)
        return ActorOuterLiftRope(s, a);
    if (a->state.id == 0x2b)
        return ActorOuterLiftDoors(s, a);
    return ActorOuterLiftSwitch(s, a);
}
