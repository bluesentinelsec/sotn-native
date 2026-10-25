// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected NO1 elevator door states and trapezoid geometry at the pinned reference.
#include "actors_internal.h"
static bool Geometry(SotnActors *s, Actor *a)
{
    if (a->state.primitive_count != 2)
        return true;
    const bool close = a->state.frame != 0;
    if ((close && a->outer_lift.upper[0] < 10) || (!close && a->outer_lift.upper[0] > -10))
    {
        if (!a->outer_lift.sounded)
        {
            if (!ActorSound(s, 0x7b1))
                return false;
            a->outer_lift.sounded = true;
        }
    }
    else
        a->outer_lift.sounded = false;
    a->outer_lift.upper[0] = (Sint16)(close ? SDL_min(10, a->outer_lift.upper[0] + 1)
                                            : SDL_max(-10, a->outer_lift.upper[0] - 1));
    a->outer_lift.upper[1] = (Sint16)(close ? SDL_min(-4, a->outer_lift.upper[1] + 1)
                                            : SDL_max(-24, a->outer_lift.upper[1] - 1));
    a->outer_lift.lower[0] = (Sint16)(close ? SDL_max(10, a->outer_lift.lower[0] - 1)
                                            : SDL_min(24, a->outer_lift.lower[0] + 1));
    a->outer_lift.lower[1] = (Sint16)(close ? SDL_max(-4, a->outer_lift.lower[1] - 1)
                                            : SDL_min(10, a->outer_lift.lower[1] + 1));
    for (Uint16 i = 0; i < 2; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        p->state = (SotnActorPrimitive){.x = a->state.x,
                                        .y = a->state.y,
                                        .page = 0x12,
                                        .palette = 0x209,
                                        .active = true,
                                        .textured = true,
                                        .quad = true,
                                        .offset_x = {-12, 1, -12, 1},
                                        .u = {0x70, 0x7d, 0x70, 0x7d}};
        for (Uint32 side = 0; side < 2; ++side)
        {
            if (i == 0)
            {
                p->state.offset_y[side] = -24;
                p->state.offset_y[2 + side] = a->outer_lift.upper[side];
                p->state.v[2 + side] = (Uint8)(side ? 0xe0 : 0xee);
                // Each slanted edge retains its own original texture crop.
                p->state.v[side] = (Uint8)(p->state.v[2 + side] - (a->outer_lift.upper[side] + 24));
            }
            else
            {
                p->state.offset_y[side] = a->outer_lift.lower[side];
                p->state.offset_y[2 + side] = 24;
                p->state.v[side] = (Uint8)(side ? 0xe0 : 0xee);
                p->state.v[2 + side] = (Uint8)(p->state.v[side] + (24 - a->outer_lift.lower[side]));
            }
        }
    }
    return true;
}
bool ActorOuterLiftDoors(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 2)
        return SDL_SetError("Outer Wall door stop exceeds its private three-stop route");
    Actor *cart = &s->slots[88];
    const bool owned = cart->state.id == 0x29 && cart->state.generation != 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0xa1c))
            return false;
        a->state.frame = 31;
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        if (available >= 2)
        {
            if (!ActorPrimitives(s, a, 2))
                return false;
            a->state.animation_set = 0;
        }
        a->outer_lift.upper[0] = 10;
        a->outer_lift.upper[1] = -4;
        a->outer_lift.lower[0] = 10;
        a->outer_lift.lower[1] = -4;
        break;
    }
    case 1:
        if (s->world->flags[SOTN_OUTER_ELEVATOR] && owned)
        {
            if (cart->outer_lift.current == a->state.parameters &&
                cart->outer_lift.target == a->state.parameters)
                SetStep(a, 2);
            const Sint32 dx = Pixel(a->state.x) - Pixel(s->player.x);
            if (dx < 0)
                a->outer_lift.wait = 32;
            else if (dx < 32 && SDL_abs((Sint64)Pixel(a->state.y) - Pixel(s->player.y)) < 16)
            {
                a->outer_lift.wait = (Uint8)(a->outer_lift.wait - 1);
                if (!a->outer_lift.wait)
                    a->state.step = 4;
            }
            else
                a->outer_lift.wait = 32;
        }
        break;
    case 2:
        a->state.frame = 0;
        if (owned && cart->outer_lift.target != a->state.parameters)
            SetStep(a, 3);
        break;
    case 3:
        a->state.frame = 31;
        SetStep(a, 1);
        break;
    case 4:
        if (!owned)
            return SDL_SetError("Outer Wall door lost its active elevator during a call");
        if (!a->substep)
        {
            cart->outer_lift.called = true;
            cart->outer_lift.target = (Uint8)a->state.parameters;
            ++a->substep;
        }
        if (cart->outer_lift.current == a->state.parameters)
            SetStep(a, 2);
        break;
    default:
        return SDL_SetError("Outer Wall elevator door entered an invalid state");
    }
    if (!Geometry(s, a))
        return false;
    SotnDataView indices = {0}, tiles = {0};
    const size_t index = a->state.parameters * 6U;
    const size_t state = (a->state.parameters + (a->state.frame ? 0U : 3U)) * 6U;
    if (!Slice(s->nz0, 0x1508 + index, 6, &indices) || !Slice(s->nz0, 0x151c + state, 6, &tiles))
        return false;
    for (Uint32 i = 0; i < 3; ++i)
        if (Read16(indices.bytes + i * 2) >= (Uint64)s->map.width * s->map.height ||
            Read16(tiles.bytes + i * 2) >= s->map.types.size)
            return SDL_SetError("Outer Wall door tile patch exceeds the room or its definitions");
    for (Uint32 i = 0; i < 3; ++i)
        if (!ActorTile(s, Read16(indices.bytes + i * 2), Read16(tiles.bytes + i * 2)))
            return false;
    return true;
}
