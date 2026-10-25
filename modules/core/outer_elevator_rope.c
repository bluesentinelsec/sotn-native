// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected NO1 rope geometry from unk_3B5E0.c; native texture cache replaces draw environments.
#include "actors_internal.h"
bool ActorOuterLiftRope(SotnActors *s, Actor *a)
{
    const Uint16 owner = a->enemy_state.owner_slot;
    if (owner >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Outer Wall rope owner exceeds the actor pool");
    const Actor *cart = &s->slots[owner];
    if (cart->state.id != 0x29 || cart->state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    a->state.x = Pixel(cart->state.x) * 65536;
    a->state.y = (Pixel(cart->state.y) - 34) * 65536;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x9f8))
            return false;
        a->state.frame = 0;
        return true;
    case 1: {
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        if (available < 32)
        {
            a->remove = true;
            return true;
        }
        if (!ActorPrimitives(s, a, 32))
            return false;
        ++a->state.step;
        return true;
    }
    case 2:
        ++a->state.step;
        return true;
    case 3:
        break;
    default:
        return SDL_SetError("Outer Wall rope entered an invalid state");
    }
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        p->state.active = false;
    }
    const Sint32 screen_x = Pixel(a->state.x) - s->camera_x;
    const Sint32 screen_y = Pixel(a->state.y) - s->camera_y;
    if ((screen_x & 256) || screen_y < 0)
        return true;
    Sint32 sway = a->outer_lift.sway;
    if (cart->outer_lift.moving)
        sway += cart->vy < 0 ? 96 : sway > 0 ? -48 : 48;
    else if (sway)
        sway += sway > 0 ? -48 : 48;
    if (sway < SDL_MIN_SINT16 || sway > SDL_MAX_SINT16)
        return SDL_SetError("Outer Wall rope sway exceeds its original signed field");
    a->outer_lift.sway = (Sint16)sway;
    Sint32 x = Pixel(a->state.x) - 4;
    Sint32 y = Pixel(a->state.y);
    if (screen_y > 240)
        y = s->camera_y + (screen_y & 15) + 240;
    if (!sway)
    {
        const Sint32 top = SDL_max(y - 240, 768);
        if (top >= y)
            return true;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 2);
        if (p == NULL)
            return false;
        p->state = (SotnActorPrimitive){
            .x = x * 65536,
            .y = y * 65536,
            .page = 0x10,
            .palette = 0x2f,
            .active = true,
            .textured = true,
            .quad = true,
            .offset_x = {0, 16, 0, 16},
            .offset_y = {(Sint16)(top - y), (Sint16)(top - y), 0, 0},
            .u = {0, 16, 0, 16},
            .v = {(Uint8)(255 - (y - top)), (Uint8)(255 - (y - top)), 255, 255}};
        return true;
    }
    Sint32 angle = sway & 4095, delta = 0;
    Uint16 run = 0;
    for (Uint16 i = 3; i < 32; ++i)
    {
        const Sint32 dx = ActorShift(-(Sint64)8 * ActorSin(angle), 12);
        const Sint32 dy = ActorShift(-(Sint64)8 * ActorSin(angle + 1024), 12);
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        const Sint32 end = SDL_max(y + dy, 768);
        p->state = (SotnActorPrimitive){.x = x * 65536,
                                        .y = y * 65536,
                                        .page = 0xe,
                                        .palette = 0x2f,
                                        .active = true,
                                        .textured = true,
                                        .quad = true,
                                        .offset_x = {(Sint16)dx, (Sint16)(dx + 4), 0, 4},
                                        .offset_y = {(Sint16)(end - y), (Sint16)(end - y), 0, 0},
                                        .u = {250, 254, 250, 254},
                                        .v = {192, 192, 200, 200}};
        if (!run)
        {
            run = 4;
            Sint16 target = 0;
            const Sint32 difference = Pixel(a->state.x) - 4 - (x + dx);
            if (!SotnMath_Atan2(s->executable, -difference, 16, &target))
                return false;
            delta = target - angle;
            if (delta > 2048)
                delta = 4096 - delta;
            if (delta < -2048)
                delta += 4096;
            delta /= 4;
        }
        angle = (angle + delta) & 4095;
        if (y - s->camera_y < -16 || y + dy < 768)
        {
            // Original helper hides the current primitive too when the chain terminates.
            p->state.active = false;
            break;
        }
        x += dx;
        y += dy;
        --run;
    }
    return true;
}
