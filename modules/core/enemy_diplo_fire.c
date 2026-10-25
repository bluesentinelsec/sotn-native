// SPDX-License-Identifier: AGPL-3.0-or-later
// Diplocephalus fireball from st/no0/e_diplocephalus_tail.c, pinned reference.
// Animation/UV data remain private; the twenty trail primitives belong to the actor.
#include "actors_internal.h"
static bool Trail(SotnActors *s, Actor *a)
{
    if ((s->ticks & 1) && !(a->state.flags & 0x100))
        for (Uint16 i = 0; i < a->state.primitive_count; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            if (p->state.active)
                continue;
            if (!ActorSprite(s, a, i, a->state.x - 12 * 65536, a->state.y - 12 * 65536, 0, 0, 28,
                             28, 26, 0x170))
                return false;
            p->state.blend = p->state.modulate = true;
            p->state.red = p->state.green = p->state.blue = 64;
            p->timer = p->phase = 0;
            break;
        }
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        if (!p->state.active)
            continue;
        if (!p->timer)
        {
            SotnDataView frame = {0};
            if (p->phase >= 8 || !Slice(s->nz0, 0x1e70 + p->phase * 5U, 5, &frame))
                return SDL_SetError("Diplocephalus fire trail cursor exceeds its private stream");
            const Uint8 duration = frame.bytes[0], u = frame.bytes[1], v = frame.bytes[2],
                        width = frame.bytes[3], height = frame.bytes[4];
            if (duration == 0 || duration == 255)
            {
                p->state.active = false;
                continue;
            }
            if ((Uint32)u + width > 255 || (Uint32)v + height > 255)
                return SDL_SetError("Diplocephalus trail UV exceeds its texture page");
            p->timer = duration;
            ++p->phase;
            p->state.u[0] = p->state.u[2] = u;
            p->state.u[1] = p->state.u[3] = (Uint8)(u + width);
            p->state.v[0] = p->state.v[1] = v;
            p->state.v[2] = p->state.v[3] = (Uint8)(v + height);
        }
        --p->timer;
    }
    return true;
}
bool ActorDiploFireball(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100) && a->state.step < 4)
    {
        if (!ActorSound(s, 0x652))
            return false;
        a->state.hitbox_state = 0;
        SetStep(a, 4);
    }
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xb54))
            return false;
        if (!ActorPrimitives(s, a, 20))
        {
            a->remove = true;
            return true;
        }
        a->state.rotated = true;
        a->state.scale_x = a->state.scale_y = 96;
        a->vx = a->state.facing_left ? 6 * 65536 : -6 * 65536;
        a->vy = 0;
        SDL_FALLTHROUGH;
    case 1:
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (!ActorAnimate(s, a, 0x1e3c, &animation))
            return false;
        a->state.rotation = (Sint16)((a->state.rotation + 64) & 4095);
        a->state.scale_x = a->state.scale_y = (Uint16)SDL_min(256, a->state.scale_x + 16);
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vx -= a->vx / 8;
        if (SDL_abs(a->vx) < 16384)
        {
            a->diplo.angle = a->state.facing_left ? 0 : 2048;
            a->diplo.acceleration = 0;
            ++a->state.step;
        }
        break;
    case 2: {
        Sint16 goal = 0;
        if (!SotnMath_Atan2(s->executable, Pixel(s->player.y) - Pixel(a->state.y),
                            Pixel(s->player.x) - Pixel(a->state.x), &goal))
            return false;
        const Uint16 delta = (Uint16)((goal - a->diplo.angle) & 4095);
        const Uint16 distance = delta & 2048 ? (Uint16)((2048 - delta) & 2047) : delta;
        a->diplo.angle =
            (Sint16)(distance > 16 ? (a->diplo.angle + ((delta & 2048) ? -16 : 16)) & 4095 : goal);
        a->diplo.acceleration += 3072;
        if (a->diplo.acceleration > 48 * 4096)
        {
            a->diplo.acceleration = 48 * 4096;
            ++a->state.step;
        }
        a->vx = ActorShift((Sint64)a->diplo.acceleration * ActorSin(a->diplo.angle + 1024), 12);
        a->vy = ActorShift((Sint64)a->diplo.acceleration * ActorSin(a->diplo.angle), 12);
        SDL_FALLTHROUGH;
    }
    case 3:
        if (!ActorAnimate(s, a, 0x1e3c, &animation))
            return false;
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    case 4:
        if (!a->substep)
        {
            for (Uint16 i = 0; i < a->state.primitive_count; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                if (p == NULL)
                    return false;
                p->state.active = false;
            }
            a->vx = ActorShift(a->vx, 1);
            a->vy = ActorShift(a->vy, 1);
            a->state.rotated = false;
            a->state.scale_x = a->state.scale_y = 0;
            a->state.flags = 0xfffffff0U;
            a->state.palette = 0x2c0;
            ++a->substep;
        }
        a->state.x += a->vx;
        a->state.y += a->vy - 65536;
        if (!ActorAnimate(s, a, 0x1e54, &animation))
            return false;
        if (!animation)
        {
            a->remove = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("Diplocephalus fireball entered invalid state %u", a->state.step);
    }
    return Trail(s, a);
}
