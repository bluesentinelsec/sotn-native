// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_thornweed_corpseweed.h projectile at the pinned reconstruction.
// Original animation and hitbox data remain in private LIB files.
#include "actors_internal.h"
bool ActorLibraryPlantProjectile(SotnActors *s, Actor *a)
{
    if (a->hit_flags && !(a->hit_flags & 0x80U))
        a->state.flags |= 0x100U;
    if (a->state.flags & 0x100U)
    {
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 2, 0, 160, 192, &effect))
            return false;
        if (effect)
        {
            effect->state.x = Pixel(a->state.x) * 65536;
            effect->state.y = Pixel(a->state.y) * 65536;
            effect->state.facing_left = false;
        }
        a->remove = true;
        return true;
    }
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x8f0))
            return false;
        const Sint32 speed = (Sint32)SDL_min(96, Distance(a, s->player)) * 2048;
        a->vx = a->state.facing_left ? -speed : speed;
        if (!ActorPrimitives(s, a, 2))
        {
            a->remove = true;
            return true;
        }
        if (!a->state.priority)
            return SDL_SetError("Corpseweed projectile primitive priority underflow");
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .quad = true,
                                        .textured = true,
                                        .page = 0x14,
                                        .palette = 0x255,
                                        .u = {64, 128, 64, 128},
                                        .v = {64, 64, 128, 128},
                                        .red = 64,
                                        .green = 64,
                                        .blue = 64,
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority - 1)};
        break;
    }
    case 1: {
        if (!ActorAnimate(s, a, 0x2bc8, &animation))
            return false;
        const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
            a->vy > SDL_MAX_SINT32 - 8192)
            return SDL_SetError("Corpseweed projectile exceeds bounded fixed-point motion");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
        a->vy += 8192;
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Wrap16((Sint64)Pixel(a->state.y) + 1), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            if (!ActorSound(s, 0x672))
                return false;
            ShiftY(a, c.tile_top);
            SetStep(a, 2);
        }
        break;
    }
    case 2: {
        if (!ActorAnimate(s, a, 0x2bd0, &animation))
            return false;
        if (!animation)
        {
            a->remove = true;
            return true;
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        if (!a->substep)
        {
            p->state.x = Pixel(a->state.x) * 65536;
            p->state.y = Pixel(a->state.y) * 65536;
            p->state.offset_y[0] = -4;
            p->state.offset_y[1] = -32;
            p->state.offset_y[2] = p->state.offset_y[3] = 0;
            p->state.active = p->state.blend = p->state.modulate = true;
            p->state.page = 0x34;
            p->state.offset_x[0] = p->state.offset_x[2] = 0;
            p->state.offset_x[1] = p->state.offset_x[3] = a->state.facing_left ? -32 : 32;
            a->substep = a->state.facing_left ? 2 : 1;
        }
        else if (a->substep == 1 || a->substep == 2)
        {
            const Sint16 direction = a->substep == 1 ? 1 : -1;
            p->state.offset_x[0] += direction;
            p->state.offset_x[1] += (Sint16)(direction * 8);
            p->state.offset_x[2] -= direction;
            p->state.offset_x[3] += (Sint16)(direction * 6);
            p->state.offset_y[0] -= 3;
            p->state.offset_y[1] -= 10;
            ++p->state.offset_y[2];
            --p->state.offset_y[3];
            p->state.red = (Uint8)SDL_max(0, (Sint32)p->state.red - 7);
            p->state.green = p->state.blue = p->state.red;
        }
        else
            return SDL_SetError("Corpseweed splat substep is invalid");
        break;
    }
    default:
        return SDL_SetError("Corpseweed projectile entered an invalid step");
    }
    return ActorLibraryPlantBox(s, a);
}
