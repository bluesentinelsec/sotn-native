// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c:EntityLesserDemonSpit at the pinned reconstruction.
#include "actors_internal.h"
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Lesser Demon spit exceeds bounded fixed-point motion");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Column(SotnActors *s, Actor *a)
{
    if (!ActorPrimitives(s, a, 1))
    {
        a->remove = true;
        return true;
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    if (a->state.priority > 509)
        return SDL_SetError("Lesser Demon spit priority exceeds ordering bounds");
    p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                    .y = Pixel(a->state.y) * 65536,
                                    .textured = true,
                                    .quad = true,
                                    .active = true,
                                    .blend = true,
                                    .page = 0x33,
                                    .palette = 0x24d,
                                    .priority_set = true,
                                    .priority = (Uint16)(a->state.priority + 2),
                                    .u = {0x68, 0x7f, 0x68, 0x7f},
                                    .v = {0xc8, 0xc8, 0xff, 0xff},
                                    .offset_x = {-16, 28, -16, 28},
                                    .offset_y = {-48, -48, 12, 12}};
    a->state.flags |= 0x800000U;
    a->state.frame = 37;
    a->pose = a->remaining = 0;
    a->demon.timer = 0;
    if (!ActorLibrarySound(s, a, 0x65b))
        return false;
    ++a->state.step;
    return true;
}
bool ActorLibraryDemonSpit(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x974))
            return false;
        a->state.flags |= 0x80000000U;
        a->state.blend_mode = 0x30;
        a->state.scale_x = a->state.scale_y = 384;
        a->state.scale_set = true;
        break;
    case 1:
        a->vx = a->state.facing_left ? 262144 : -262144;
        a->vy = 262144;
        ++a->state.step;
        break;
    case 2: {
        bool blocked = false;
        if (!Move(a) || !ActorAnimate(s, a, 0x1bcc, &animation) ||
            !ActorLibraryBlocked(s, a, 0x1aec, false, &blocked))
            return false;
        if (blocked)
        {
            a->pose = a->remaining = 0;
            a->state.scale_x = 320;
            a->state.scale_y = 512;
            a->state.hitbox_width = a->state.hitbox_height = 8;
            ++a->state.step;
        }
        break;
    }
    case 3:
        if (!ActorAnimate(s, a, 0x1bd8, &animation))
            return false;
        if (!animation)
            return Column(s, a);
        break;
    case 4: {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        if (a->demon.timer++ > 16)
        {
            ++a->state.step;
            a->demon.timer = 29;
            a->state.hitbox_height = 64;
            a->state.hitbox_y = (Sint16)(a->state.hitbox_y - 32);
        }
        else
        {
            p->state.offset_y[0] -= (Sint16)(16 - a->demon.timer);
            p->state.offset_y[1] = p->state.offset_y[0];
            a->state.hitbox_height =
                (Uint16)((p->state.offset_y[2] - p->state.offset_y[0]) / 2 - 16);
            a->state.hitbox_y = (Sint16) - (Sint32)a->state.hitbox_height;
        }
        break;
    }
    case 5: {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        if (!a->demon.timer)
            return SDL_SetError("Lesser Demon spit column timer underflow");
        if (--a->demon.timer == 0)
        {
            a->pose = a->remaining = 0;
            a->state.hitbox_state = 0;
            ++a->state.step;
            p->state.active = false;
        }
        else
        {
            p->state.offset_y[0] += 4;
            p->state.offset_y[1] = p->state.offset_y[0];
            p->state.v[2] = (Uint8)(p->state.v[2] - 2);
            p->state.v[3] = p->state.v[2];
            a->state.hitbox_height =
                (Uint16)((p->state.offset_y[2] - p->state.offset_y[0]) / 2 - 16);
            a->state.hitbox_y = (Sint16) - (Sint32)a->state.hitbox_height;
        }
        break;
    }
    case 6:
        if (a->state.scale_y < 32)
            return SDL_SetError("Lesser Demon spit fade scale underflow");
        a->state.scale_y -= 32;
        if (!ActorAnimate(s, a, 0x1be8, &animation))
            return false;
        if (!animation)
        {
            a->state.opacity_set = true;
            a->state.opacity = 128;
            ++a->state.step;
        }
        break;
    case 7:
        if (a->state.scale_y < 16 || a->state.opacity < 16)
            return SDL_SetError("Lesser Demon spit opacity/scale underflow");
        a->state.scale_y -= 16;
        a->state.opacity -= 16;
        if (!a->state.opacity)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Lesser Demon spit entered an invalid gameplay step");
    }
    return true;
}
