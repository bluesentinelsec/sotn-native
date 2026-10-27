// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed BO0 313A8.c:func_us_801B551C at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime tables stay private.
#include "olrox_monster_internal.h"
static Sint16 Word(Sint32 n)
{
    return (Sint16)Wrap16(n);
}
static void Whole(Sint32 *p, Sint32 n)
{
    *p = (Sint32)((Sint64)Word(n) * 65536 + ((Uint32)*p & 65535U));
}
static bool Approach(Actor *a, Sint16 target, Sint16 speed, bool *arrived)
{
    return OlroxMonster_Approach(&a->state.rotation, target, speed, arrived);
}
static bool Capture(SotnActors *s, Actor *a, Actor *body, bool alternate)
{
    if (!OlroxMonster_FreezeSprite(s, a, body, 0))
        return false;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    p->state.page = alternate ? 0x15 : 0x14;
    p->state.palette = alternate ? 0x219 : (Uint16)(p->state.palette + 0x20bU);
    p->state.active = true;
    p->state.priority_set = true;
    p->state.priority = a->state.priority;
    return true;
}
static bool Spawn(SotnActors *s, Actor *a, Uint16 id, Uint16 parameters, Uint32 begin, Uint32 end,
                  Actor **out)
{
    if (!ActorSpawn(s, a, id, parameters, begin, end, out))
        return false;
    if (*out)
    {
        (*out)->state.x = Pixel(a->state.x) * 65536;
        (*out)->state.y = Pixel(a->state.y) * 65536;
        (*out)->state.facing_left = a->state.facing_left;
        (*out)->enemy_state.owner_slot = a->slot;
        (*out)->enemy_state.owner_generation = a->state.generation;
    }
    return true;
}
static bool Laser(SotnActors *s, Actor *a, Actor *body)
{
    switch (a->substep)
    {
    case 0: {
        Actor *laser = NULL;
        if (!Spawn(s, a, 0x39, a->olrox.laser_pattern & 1U, 0, 192, &laser))
            return false;
        if (!laser)
            return SDL_SetError(
                "Olrox laser actor pool exhausted; replacing its boss body is unsafe");
        body->olrox.laser_busy = 1;
        a->state.frame = 3;
        ++a->substep;
        break;
    }
    case 1:
        if (!a->olrox.laser_busy)
            body->olrox.laser_busy = 0;
        break;
    default:
        return SDL_SetError("Olrox head laser has an invalid original subphase");
    }
    return true;
}
static bool Aim(SotnActors *s, Actor *a, Actor *body)
{
    if (!a->substep)
    {
        a->state.frame = 4;
        Sint16 angle = 0;
        const Sint16 dx = Word(Pixel(a->state.x) - Pixel(s->player.x)),
                     dy = Word(Pixel(a->state.y) - Pixel(s->player.y));
        if (!SotnMath_Atan2(s->executable, -dy, a->state.facing_left ? -dx : dx, &angle))
            return false;
        Sint16 target = Word(256 - angle);
        if (target >= 897)
            target = 896;
        if (target < -384)
            target = 384;
        a->olrox.head_target = target;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        bool head = false, torso = false;
        if (!Approach(a, a->olrox.head_target, 32, &head) ||
            !OlroxMonster_Smooth(&body->olrox.body_angle, &body->olrox.body_velocity,
                                 (Sint16)(-((Sint32)a->olrox.head_target + 384) / 2), 32, &torso))
            return false;
        if (head && torso)
        {
            body->olrox.fire_busy = 1;
            a->substep = 0;
            a->state.step = 9;
        }
    }
    return true;
}
static bool Fire(SotnActors *s, Actor *a, Actor *body)
{
    if (a->olrox.fire_rank > 3)
        return SDL_SetError("Olrox fireball rank exceeds its original four private profiles");
    SotnDataView profile = {0};
    if (!Slice(s->nz0, 0x11ec + a->olrox.fire_rank * 8U, 8, &profile))
        return false;
    if (!a->substep)
    {
        a->state.frame = 4;
        a->olrox.timer = Read16(profile.bytes);
        a->olrox.head_sweep = false;
        a->olrox.head_delay = Read16(profile.bytes + 6);
        ++a->substep;
    }
    if (a->substep == 1)
    {
        if (!--a->olrox.head_delay)
        {
            a->olrox.head_delay = Read16(profile.bytes + 6);
            Actor *fire = NULL;
            if (!Spawn(s, a, 0x3a, 0, 96, 128, &fire))
                return false;
            if (fire)
                fire->olrox.fire_rank = a->olrox.fire_rank;
        }
        const Sint32 range = Signed16(profile.bytes + 2), delta = Signed16(profile.bytes + 4);
        a->state.rotation = Word(a->state.rotation + (a->olrox.head_sweep ? delta : -delta));
        if (a->state.rotation < a->olrox.head_target - range)
            a->olrox.head_sweep = true;
        if (a->olrox.head_target + range < a->state.rotation)
            a->olrox.head_sweep = false;
        if (a->state.rotation < -384)
            a->olrox.head_sweep = true;
        if (a->state.rotation >= 897)
            a->olrox.head_sweep = false;
        body->olrox.body_angle = (Sint16)(-((Sint32)a->state.rotation + 384) / 2);
        if (!--a->olrox.timer)
        {
            a->substep = 0;
            a->state.step = 1;
            body->olrox.fire_busy = 0;
        }
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a, Actor *body)
{
    switch (a->substep)
    {
    case 0:
        if (!ActorPrimitives(s, a, 1))
        {
            SDL_ClearError();
            ++a->substep;
            break;
        }
        a->state.flags |= 0x800000U;
        if (!Capture(s, a, body, false))
            return false;
        ++a->substep;
        break;
    case 1:
        a->state.palette = 0x219;
        a->state.texture_offset = 0x54;
        if (body->olrox.death_phase)
        {
            if (a->state.primitive_count)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
                if (!p)
                    return false;
                p->state.active = false;
            }
            ++a->substep;
        }
        break;
    case 2:
        a->state.palette = 0x8219;
        a->state.texture_offset = 0x54;
        if (body->olrox.death_phase == 3 && a->state.primitive_count)
        {
            a->state.frame = 3;
            if (!Capture(s, a, body, true))
                return false;
            ++a->substep;
        }
        break;
    case 3:
        break;
    default:
        return SDL_SetError("Olrox head defeat has an invalid original subphase");
    }
    return true;
}
bool ActorOlroxHead(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || a->state.id != 0x35 || !a->state.generation)
        return SDL_SetError("Olrox head requires its active original BO0 identity");
    if (!a->enemy_state.owner_generation || a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
    {
        a->remove = true;
        return true;
    }
    Actor *body = &s->slots[a->enemy_state.owner_slot];
    if (body->state.id != 0x33 || body->state.generation != a->enemy_state.owner_generation ||
        !OlroxOwnsChild(s, body, a, 3))
    {
        a->remove = true;
        return true;
    }
    if (!OlroxMonster_Bind(s, body))
        return false;
    a->state.facing_left = body->state.facing_left;
    a->state.palette = body->state.palette;
    if (a->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    bool arrived = false;
    Uint8 animated = 0;
    SotnDataView target = {0};
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x714))
            return false;
        a->state.hitbox_width = a->state.hitbox_height = 10;
        a->state.frame = 1;
        a->state.transform_flags_set = true;
        a->state.transform_flags |= SOTN_ACTOR_ROTATE;
        a->state.rotation = -256;
        a->state.priority = (Uint16)((Sint32)s->olrox_monster.priority - 2);
        break;
    case 1:
        if (!ActorAnimate(s, a, 0x120c, &animated) || !Approach(a, -128, 8, &arrived))
            return false;
        a->substep = 0;
        break;
    case 7:
        if (!ActorAnimate(s, a, 0x1220, &animated) || !Approach(a, 192, 8, &arrived))
            return false;
        if (!a->remaining && !(a->pose & 3U) && !ActorSound(s, 0x819))
            return false;
        a->substep = 0;
        break;
    case 6:
        a->state.frame = 1;
        if (!Approach(a, -256, 32, &arrived))
            return false;
        break;
    case 5:
        if (!Approach(a, 384, 16, &arrived))
            return false;
        if (arrived)
        {
            if (!ActorAnimate(s, a, 0x1234, &animated))
                return false;
            if (!animated)
            {
                body->olrox.roar_busy = 0;
                a->state.step = 1;
            }
            if (a->pose == 2 && !a->remaining && !ActorSound(s, 0x819))
                return false;
            if (a->pose >= 3)
                Whole(&a->state.x, Pixel(a->state.x) + (a->remaining & 2U ? 1 : -1));
        }
        else
            a->pose = a->remaining = 0;
        break;
    case 2:
        a->state.frame = 2;
        if (a->olrox.laser_pattern >= 4 ||
            !Slice(s->nz0, 0x11d0 + a->olrox.laser_pattern * 8U, 2, &target))
            return SDL_SetError("Olrox laser head pose exceeds its private pattern table");
        if (!Approach(a, (Sint16)Signed16(target.bytes), 16, &arrived))
            return false;
        if (arrived)
        {
            a->olrox.laser_busy = 1;
            a->state.step = 3;
            a->substep = 0;
        }
        break;
    case 3:
        if (!Laser(s, a, body))
            return false;
        break;
    case 4:
        if (!Aim(s, a, body))
            return false;
        break;
    case 9:
        if (!Fire(s, a, body))
            return false;
        break;
    case 10:
        a->state.frame = 1;
        break;
    case 8:
        if (!Death(s, a, body))
            return false;
        break;
    default:
        return SDL_SetError("Olrox head entered unsupported original state %u", a->state.step);
    }
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(s->executable, a->state.rotation, &t))
        return false;
    a->state.hitbox_x = Word(-ActorShift((Sint64)t.cosine * 12, 12));
    a->state.hitbox_y = Word(-ActorShift((Sint64)t.sine * 12, 12));
    if (!s->olrox_morph.transforming && a->state.step < 8 && !(ActorRandom(s) & 63U))
    {
        Actor *drool = NULL;
        if (!Spawn(s, a, 0x3d, 0, 160, 192, &drool))
            return false;
        if (drool)
        {
            drool->state.facing_left = false;
            drool->state.parameters = (Uint16)((ActorRandom(s) & 7U) + 16U);
        }
    }
    return true;
}
