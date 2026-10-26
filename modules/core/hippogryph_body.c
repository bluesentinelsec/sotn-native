// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO5 func_us_801A1C14 / func_us_801A19FC and shared collision services,
// pinned sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original timelines, sensor offsets and attack-choice arrays remain private runtime inputs.
#include "hippogryph_internal.h"

static void Step(Actor *a, Uint16 step)
{
    SetStep(a, step);
    a->hippogryph.phase = 0;
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Hippogryph motion exceeds bounded fixed-point position");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Velocity(Actor *a, Sint64 x, Sint64 y)
{
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Hippogryph acceleration exceeds bounded fixed-point velocity");
    a->vx = (Sint32)x;
    a->vy = (Sint32)y;
    return true;
}
static void Face(SotnActors *s, Actor *a)
{
    a->state.facing_left = Side(a, s->player) == 0;
}
static bool PackedPose(const Actor *a, Uint16 pose)
{
    return a->pose == pose && !a->remaining;
}
static bool Offsets(SotnActors *s, Actor *a, size_t offset, bool facing, Uint8 *result)
{
    *result = 0;
    for (Uint16 i = 0; i <= 8; ++i)
    {
        SotnDataView sensor = {0};
        if (!Slice(s->nz0, offset + i * 4U, 2, &sensor))
            return false;
        const Sint32 dx = Signed16(sensor.bytes);
        if (dx == 255)
            return true;
        if (i == 8 || !Slice(s->nz0, offset + i * 4U, 4, &sensor))
            return SDL_SetError("Hippogryph collision sensors have no bounded terminator");
        SotnCollider c = {0};
        if (!ActorQuery(s, Wrap16((Sint64)Pixel(a->state.x) + (facing ? dx : -dx)),
                        Wrap16((Sint64)Pixel(a->state.y) + Signed16(sensor.bytes + 2)), &c))
            return false;
        *result = (Uint8)((*result << 1U) | ((c.effects & SOTN_EFFECT_SOLID) ? 1U : 0U));
    }
    return false;
}
bool HippogryphFloor(SotnActors *s, Actor *a, bool *landed)
{
    Uint16 collision = 0;
    if (!ActorGround(s, a, 0xaac, &collision))
        return false;
    *landed = (collision & 1U) != 0;
    if (!*landed && Pixel(a->state.y) > 183)
    {
        ShiftY(a, 183 - Pixel(a->state.y));
        a->vx = a->vy = 0;
        *landed = true;
    }
    return true;
}
bool HippogryphParticles(SotnActors *s, Actor *a, Uint8 count, Sint16 offset_y)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        Actor *part = NULL;
        if (!ActorSpawn(s, a, 0x22, 0, 160, 192, &part))
            return false;
        if (!part)
            continue;
        part->state.animation_set = a->state.animation_set;
        part->state.texture_offset = a->state.texture_offset;
        part->state.palette = (Uint16)(a->state.palette & 0x7fffU);
        part->state.priority = a->state.priority;
        part->state.facing_left = false;
        part->state.flags = 0xc8202000U;
        part->state.x = Wrap16((Sint64)Pixel(a->state.x) - 32 + (ActorRandom(s) & 63U)) * 65536;
        part->state.y =
            Wrap16((Sint64)Pixel(a->state.y) + offset_y - 32 + (ActorRandom(s) & 63U)) * 65536;
        part->vy = -16384;
        if (ActorRandom(s) & 1U)
            ++part->state.priority;
        part->state.step = 48;
        part->state.parameters = (Uint16)(ActorRandom(s) & 1U);
        if (part->state.parameters)
        {
            part->state.step = 50;
            const bool left = (ActorRandom(s) & 1U) != 0;
            part->vx = left ? -53248 : 53248;
            part->hippogryph.rotation_velocity = left ? 3328 : -3328;
        }
        if (a->state.step == 22 &&
            !Velocity(a, (Sint64)a->vx + (a->state.facing_left ? 16384 : -16384), a->vy))
            return false;
    }
    return true;
}
static bool Spawn(SotnActors *s, Actor *a, Uint16 id, Uint16 param, Uint32 begin, Uint32 end,
                  Sint16 dx, Sint16 dy, Actor **out)
{
    if (!ActorSpawn(s, a, id, param, begin, end, out))
        return false;
    if (*out)
    {
        (*out)->state.x = Wrap16((Sint64)Pixel(a->state.x) + dx) * 65536;
        (*out)->state.y = Wrap16((Sint64)Pixel(a->state.y) + dy) * 65536;
    }
    return true;
}
void HippogryphGrab(SotnActors *s, Actor *a, SotnPlayerGrabState state)
{
    s->player.grab.state = state;
    if (state != SOTN_GRAB_NONE)
    {
        s->player.grab.damage = 16;
        s->player.grab.source_x = Pixel(a->state.x);
        s->player.grab.source_velocity = a->vx;
    }
    s->motion.grab_changed = true;
    s->motion.grab = s->player.grab;
}
static void Pin(SotnActors *s, const Actor *a)
{
    const Sint32 x = Pixel(a->state.x), y = Wrap16((Sint64)Pixel(a->state.y) + 32);
    s->motion.align_horizontal = s->motion.align_vertical = true;
    s->motion.alignment_x = x;
    s->motion.alignment_y = y;
    s->player.x = (Sint32)((Sint64)x * 65536 + ((Uint32)s->player.x & 65535U));
    s->player.y = (Sint32)((Sint64)y * 65536 + ((Uint32)s->player.y & 65535U));
}
static bool Ascend(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    switch (a->substep)
    {
    case 0:
        if (!ActorAnimate(s, a, 0x8bc, &animated))
            return false;
        if (!animated)
        {
            a->state.frame = 49;
            a->vy = -327680;
            a->hippogryph.timer = 4;
            a->hippogryph.phase = 0;
            ++a->substep;
        }
        if (a->state.frame == 41 && !a->hippogryph.phase)
        {
            if (!HippogryphParticles(s, a, 4, 0))
                return false;
            a->hippogryph.phase = 1;
            a->vx = a->state.facing_left ? 24576 : -24576;
            ShiftY(a, -4);
        }
        if (a->hippogryph.phase)
        {
            if (!Move(a))
                return false;
            if ((a->state.frame == 42 || a->state.frame == 43) &&
                !Velocity(a, a->vx, (Sint64)a->vy - 24576))
                return false;
        }
        break;
    case 1:
        if (!Move(a) || !Velocity(a, a->vx, (Sint64)a->vy + 7680))
            return false;
        if (!--a->hippogryph.timer)
        {
            a->pose = 0;
            ++a->substep;
        }
        break;
    case 2:
        if (!Move(a) || !Velocity(a, a->vx, (Sint64)a->vy + 8192) ||
            !ActorAnimate(s, a, 0x8d4, &animated))
            return false;
        if (!animated)
            Step(a, 6);
        return true;
    default:
        return true;
    }
    if (Pixel(a->state.y) < 114)
    {
        a->state.frame = 49;
        a->vy = -294912;
        a->hippogryph.phase = 0;
        a->substep = 2;
    }
    return true;
}
static bool Hover(SotnActors *s, Actor *a)
{
    if (!a->substep)
    {
        a->hippogryph.timer = 80;
        a->vx = a->state.facing_left ? 114688 : -114688;
        a->vy = 0;
        ++a->substep;
    }
    Uint8 animated = 0;
    if (!Move(a) || !ActorAnimate(s, a, 0x830, &animated) ||
        (PackedPose(a, 1) && !ActorPositionalSound(s, a, 0x768)) ||
        !Velocity(a, a->vx,
                  (Sint64)a->vy + (a->state.frame == 6 || a->state.frame == 7 ? -10240 : 3584)))
        return false;
    if (!--a->hippogryph.timer)
    {
        const Sint32 progress = a->vx >= 0 ? Pixel(a->state.x) : 512 - Pixel(a->state.x);
        if (Distance(a, s->player) > 80 && progress < 144)
        {
            a->vx = a->vy = 0;
            Step(a, 22);
        }
        else if (progress < 384)
            Step(a, 8);
        else
        {
            Face(s, a);
            Step(a, 6);
        }
    }
    return true;
}
static bool Land(SotnActors *s, Actor *a)
{
    Uint8 animated = 0, collision = 0;
    bool landed = false;
    switch (a->substep)
    {
    case 0:
        a->vx = a->state.facing_left ? 32768 : -32768;
        a->vy = 229376;
        ++a->substep;
        SDL_FALLTHROUGH;
    case 1:
        if (!ActorAnimate(s, a, 0x938, &animated))
            return false;
        if (!animated)
        {
            a->pose = a->remaining = 0;
            ++a->substep;
        }
        if (!Move(a))
            return false;
        if (Pixel(a->state.y) > 136)
        {
            a->pose = a->remaining = 0;
            a->substep = 2;
        }
        return true;
    case 2:
        if (!ActorAnimate(s, a, 0x944, &animated) || !Move(a) ||
            !Offsets(s, a, 0xadc, false, &collision))
            return false;
        if (collision)
        {
            a->pose = a->remaining = 0;
            a->hippogryph.phase = 0;
            ++a->substep;
        }
        return true;
    case 3:
        if (!HippogryphFloor(s, a, &landed))
            return false;
        if (landed)
            a->hippogryph.phase = 1;
        if (!Velocity(a, a->vx, SDL_max((Sint64)a->vy - 31744, 32768)) ||
            !ActorAnimate(s, a, 0x950, &animated))
            return false;
        if (!animated && a->hippogryph.phase)
            Step(a, 10);
        return true;
    default:
        return true;
    }
}
static bool Breath(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    if (!a->substep)
    {
        if (!ActorAnimate(s, a, 0x858, &animated))
            return false;
        if (!animated)
        {
            a->hippogryph.timer = 64;
            ++a->substep;
            if (!ActorPositionalSound(s, a, 0x6c3))
                return false;
        }
        return !PackedPose(a, 4) || ActorPositionalSound(s, a, 0x614);
    }
    if (a->substep != 1)
        return true;
    if (a->hippogryph.timer & 1U)
    {
        Actor *flame = NULL;
        const Sint16 dx = a->state.facing_left ? 29 : -29;
        const Uint8 phase = a->hippogryph.phase;
        if (!Spawn(s, a, 0x27, phase, 160, 192, dx, -2, &flame) ||
            !Spawn(s, a, 0x27, (Uint16)(phase + 2U), phase ? 96 : 160, phase ? 128 : 192, dx, -2,
                   &flame))
            return false;
        a->hippogryph.phase ^= 1;
    }
    if (!--a->hippogryph.timer)
        Step(a, 10);
    return true;
}
static bool Run(SotnActors *s, Actor *a)
{
    if (!a->hippogryph.phase)
    {
        a->hippogryph.phase = 1;
        a->hippogryph.timer = 128;
        a->vx = a->state.facing_left ? 81920 : -81920;
        a->vy = 0;
    }
    Uint8 animated = 0, collision = 0;
    if (!Move(a) || !ActorAnimate(s, a, 0x824, &animated))
        return false;
    if (a->state.frame == 5)
    {
        if (!a->remaining && !ActorPositionalSound(s, a, 0x646))
            return false;
        if (!Velocity(a, (Sint64)a->vx + (a->state.facing_left ? -7168 : 7168), a->vy))
            return false;
    }
    if (a->state.frame == 1 &&
        !Velocity(a, (Sint64)a->vx + (a->state.facing_left ? 7168 : -7168), a->vy))
        return false;
    const Sint32 x = Pixel(a->state.x);
    if ((a->state.facing_left && x > 384) || (!a->state.facing_left && x < 128))
    {
        a->state.facing_left = !a->state.facing_left;
        a->vx = a->state.facing_left ? 81920 : -81920;
        a->pose = a->remaining = 0;
    }
    if (!Offsets(s, a, 0xae4, a->state.facing_left, &collision))
        return false;
    if (!collision)
    {
        Face(s, a);
        Step(a, 4);
    }
    if (!--a->hippogryph.timer)
        Step(a, 10);
    return true;
}
static bool Lunge(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    switch (a->substep)
    {
    case 0:
        if (!ActorAnimate(s, a, 0x8fc, &animated))
            return false;
        if (!animated)
        {
            if (!HippogryphParticles(s, a, 4, 0))
                return false;
            a->hippogryph.timer = 64;
            a->vx = a->state.facing_left ? 294912 : -294912;
            a->vy = 0;
            a->pose = a->remaining = 0;
            a->state.frame = 33;
            ++a->substep;
        }
        return true;
    case 1:
        // The original lunge does not decrement this timer in this state.
        if (a->hippogryph.timer < 32 && !ActorAnimate(s, a, 0x884, &animated))
            return false;
        if (!Move(a) || !Velocity(a, a->vx, (Sint64)a->vy - 4096))
            return false;
        if (Pixel(a->state.y) < 72)
            ++a->substep;
        return true;
    case 2:
        Face(s, a);
        Step(a, 6);
        return true;
    default:
        return true;
    }
}
static bool Objects(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, 0x98c, &animated))
        return false;
    if (!animated)
    {
        Face(s, a);
        Step(a, 4);
    }
    if (a->state.frame == 77 && !a->hippogryph.phase)
    {
        a->hippogryph.phase = 1;
        const Uint16 count = (Uint16)((ActorRandom(s) & 1U) | 2U);
        for (Uint16 i = 0; i < count; ++i)
        {
            Actor *object = NULL;
            if (!Spawn(s, a, 0x25, 0, 96, 128, a->state.facing_left ? -16 : 16, 0, &object))
                return false;
            if (object)
            {
                object->state.facing_left = false;
                object->hippogryph.timer = (Uint8)((ActorRandom(s) & 31U) - 64U);
                object->vx = (a->state.facing_left ? 32768 : -32768) +
                             (a->state.facing_left ? -(Sint32)i : (Sint32)i) * 32768;
            }
        }
    }
    return true;
}
static bool GrabFlight(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    switch (a->substep)
    {
    case 0:
        if (!ActorAnimate(s, a, 0x88c, &animated) || !Move(a))
            return false;
        if (SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 80)
            ++a->substep;
        break;
    case 1:
        if (!Move(a) || !ActorAnimate(s, a, 0x894, &animated) ||
            !Velocity(a, a->vx, (Sint64)a->vy - 2048))
            return false;
        if (!animated)
            ++a->substep;
        if (s->player.grab.state == SOTN_GRAB_NONE)
        {
            const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y);
            const Sint32 progress = a->state.facing_left ? 512 - x : x;
            const Sint32 dx = Pixel(s->player.x) - x, dy = Pixel(s->player.y) - y;
            if (progress >= 128 && SDL_abs(dx) < 12 && dy >= 0 && dy < 56 &&
                !(s->player.status & 0x40c401d2U))
            {
                if (!ActorPositionalSound(s, a, 0x769))
                    return false;
                HippogryphGrab(s, a, SOTN_GRAB_START);
                a->state.x =
                    (Sint32)((Sint64)Pixel(s->player.x) * 65536 + ((Uint32)a->state.x & 65535U));
                ShiftY(a, (Sint64)Pixel(s->player.y) - 32 - Pixel(a->state.y));
            }
        }
        break;
    case 2:
        if (!Move(a) || !Velocity(a, a->vx, (Sint64)a->vy - 24576) ||
            !ActorAnimate(s, a, 0x89c, &animated))
            return false;
        if (!animated)
        {
            a->hippogryph.timer = 64;
            ++a->substep;
            if (s->player.grab.state == SOTN_GRAB_NONE &&
                !Velocity(a, a->vx, (Sint64)a->vy - 24576))
                return false;
        }
        break;
    case 3:
        if (s->player.grab.state != SOTN_GRAB_NONE)
        {
            if (!ActorAnimate(s, a, 0x8ac, &animated))
                return false;
            --a->hippogryph.timer;
            if (a->hippogryph.timer % 47U == 0)
            {
                if (!HippogryphParticles(s, a, 8, -8))
                    return false;
                HippogryphGrab(s, a, SOTN_GRAB_DAMAGE);
            }
        }
        else if (!ActorAnimate(s, a, 0x830, &animated) ||
                 (PackedPose(a, 1) && !ActorPositionalSound(s, a, 0x768)))
            return false;
        if (!Move(a))
            return false;
        if (Pixel(a->state.y) < 72)
        {
            HippogryphGrab(s, a, SOTN_GRAB_NONE);
            Face(s, a);
            Step(a, 6);
        }
        break;
    default:
        break;
    }
    if (s->player.grab.state != SOTN_GRAB_NONE)
        Pin(s, a);
    return true;
}
static bool Swoop(SotnActors *s, Actor *a)
{
    Uint8 animated = 0, collision = 0;
    bool landed = false;
    switch (a->substep)
    {
    case 0:
        Face(s, a);
        if (!ActorAnimate(s, a, 0x844, &animated))
            return false;
        if (!animated)
            ++a->hippogryph.phase;
        if ((PackedPose(a, 1) && !ActorPositionalSound(s, a, 0x768)) || !Move(a) ||
            !Velocity(a, a->vx,
                      (Sint64)a->vy + (a->state.frame == 6 || a->state.frame == 7 ? -10240 : 4096)))
            return false;
        if (a->hippogryph.phase >= 2)
        {
            a->hippogryph.phase = 0;
            a->pose = a->remaining = 0;
            ++a->substep;
            a->vx = a->state.facing_left ? 262144 : -262144;
            a->vy = 294912;
        }
        return true;
    case 1:
        if (!Move(a) || !ActorAnimate(s, a, 0x884, &animated) ||
            !Velocity(a, a->vx, SDL_max((Sint64)a->vy - 6144, 32768)) ||
            !Offsets(s, a, 0xadc, false, &collision))
            return false;
        if (collision)
        {
            a->pose = a->remaining = 0;
            a->hippogryph.phase = 0;
            ++a->substep;
        }
        return true;
    case 2:
        if (!Velocity(a, a->vx, (Sint64)a->vy - 14336) || !HippogryphFloor(s, a, &landed))
            return false;
        if (landed)
            a->hippogryph.phase = 1;
        if (!Velocity(a, (Sint64)a->vx + (a->state.facing_left ? -4096 : 4096), a->vy))
            return false;
        if (a->state.frame == 94 && !HippogryphParticles(s, a, 2, -2))
            return false;
        if (!ActorAnimate(s, a, 0x96c, &animated))
            return false;
        if (!animated && a->hippogryph.phase)
            Step(a, 10);
        return true;
    default:
        return true;
    }
}
bool ActorHippogryphBody(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 25 || a->state.id != 0x22)
        return SDL_SetError("Hippogryph body requires original BO5 raw22 ownership");
    if (a->hit_params && (a->state.step & 1U))
    {
        a->vx = a->state.facing_left ? -98304 : 98304;
        a->vy = -262144;
        Step(a, 32);
    }
    if ((a->state.flags & 0x100U) && a->state.step < 34)
    {
        a->state.hitbox_state = 0;
        a->vx = (Sint32)((Uint32)a->vx & 65535U);
        a->vy = (Sint32)((Uint32)a->vy & 65535U);
        a->hippogryph.timer = 0;
        a->state.frame = 110;
        if (a->hippogryph.linked_generation)
        {
            const Uint16 slot = a->hippogryph.linked_slot;
            if (slot >= SOTN_STAGE_ACTOR_SLOTS || slot == a->slot)
                return SDL_SetError("Hippogryph defeat has an invalid linked child slot");
            Actor *child = &s->slots[slot];
            if (child->state.generation == a->hippogryph.linked_generation)
            {
                ActorPrimitivesClear(s, child);
                if (!SotnRoomActors_Release(s->spawns, slot, false))
                    return false;
                *child = (Actor){0};
            }
            a->hippogryph.linked_generation = 0;
        }
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        a->state.flags &= ~0x800000U;
        Step(a, 36);
    }
    Uint8 animated = 0;
    bool landed = false;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x4d8))
            return false;
        a->hippogryph.saved_priority = (Uint8)a->state.priority;
        a->state.priority = (Uint16)(s->player.priority + 1U);
        Face(s, a);
        a->hippogryph.phase = 1;
        break;
    case 1:
        a->state.hitbox_state = 0;
        if (!ActorAnimate(s, a, 0x91c, &animated))
            return false;
        if (a->hippogryph.phase && Distance(a, s->player) < 128)
        {
            Step(a, 3);
            if (!s->hippogryph_encounter && !SotnWorld_VisitEvent(s->world, SOTN_TIME_HIPPOGRYPH))
                return false;
            s->hippogryph_encounter |= 1U;
            a->state.hitbox_state = 3;
            Actor *weapon = NULL;
            if (!ActorOwnedSpawnAt(s, a, 0x23, 0, (Uint32)a->slot + 1U, &weapon))
                return false;
            weapon->state.x = Pixel(a->state.x) * 65536;
            weapon->state.y = Pixel(a->state.y) * 65536;
            weapon->state.facing_left = false;
        }
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x804, &animated))
            return false;
        if (!animated)
            Step(a, 4);
        break;
    case 4:
        if (!Ascend(s, a))
            return false;
        break;
    case 6:
        if (!Hover(s, a))
            return false;
        break;
    case 8:
        if (!Land(s, a))
            return false;
        break;
    case 10: {
        Face(s, a);
        const Uint32 choice = (ActorRandom(s) & 7U) + (Distance(a, s->player) > 80 ? 4U : 0U) +
                              (Distance(a, s->player) > 128 ? 4U : 0U);
        SotnDataView next = {0};
        if (!Slice(s->nz0, 0xaec + choice, 1, &next))
            return false;
        Step(a, next.bytes[0]);
        break;
    }
    case 12:
        if (!Breath(s, a))
            return false;
        break;
    case 13:
        if (!Run(s, a))
            return false;
        break;
    case 16:
        if (!Lunge(s, a))
            return false;
        break;
    case 18:
        if (!GrabFlight(s, a))
            return false;
        break;
    case 20:
        if (!Objects(s, a))
            return false;
        break;
    case 22:
        if (!Swoop(s, a))
            return false;
        break;
    case 32:
        if (!ActorAnimate(s, a, 0x928, &animated) || !HippogryphFloor(s, a, &landed))
            return false;
        if (landed)
        {
            a->state.frame = 80;
            Step(a, 10);
        }
        break;
    case 48:
    case 50: {
        if (!ActorAnimate(s, a, a->state.step == 48 ? 0xa2c : 0xa48, &animated))
            return false;
        if (a->state.step == 50)
        {
            if (!Velocity(a, (Sint64)a->vx + a->hippogryph.rotation_velocity, a->vy))
                return false;
            if ((Uint32)a->vx + 53248U > 106496U)
            {
                if (a->hippogryph.rotation_velocity == SDL_MIN_SINT32)
                    return SDL_SetError("Hippogryph particle turn exceeds fixed-point bounds");
                a->hippogryph.rotation_velocity = -a->hippogryph.rotation_velocity;
            }
        }
        Uint16 collision = 0;
        if (!Velocity(a, a->vx, (Sint64)a->vy - 14336) || !ActorGround(s, a, 0xacc, &collision))
            return false;
        a->remove = (collision & 1U) != 0;
        break;
    }
    case 36:
        if (!HippogryphDeath(s, a))
            return false;
        break;
    case 130:
        if (s->buttons & SOTN_BUTTON_LEFT_HAND)
        {
            if (!a->state.parameters)
            {
                ++a->state.frame;
                a->state.parameters = 1;
            }
        }
        else
            a->state.parameters = 0;
        break;
    default:
        break; // Original unused jump-table states do not advance the body.
    }
    const Uint8 frame = (Uint8)a->state.frame;
    const bool vertical = (frame >= 14 && frame <= 24) || (frame >= 58 && frame <= 63);
    a->state.hitbox_width = vertical ? 6 : 20;
    a->state.hitbox_height = vertical ? 20 : 8;
    a->state.hitbox_x = frame >= 14 && frame <= 24 ? 8 : 0;
    a->state.hitbox_y = vertical ? -8 : -2;
    return true;
}
