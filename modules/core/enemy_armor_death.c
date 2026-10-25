// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected FadeArmorLordDeath control flow, st/e_armor_lord.h at the pinned reference.
// The native ribbon projection and flat lighting are bounded presentation adaptations;
// per-vertex GTE lighting/depth ordering remain tracked parity work.
#include "actors_internal.h"
static bool Silhouette(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(a->state.x) - (a->state.facing_left ? 26 : 13);
    const Sint32 y = Pixel(a->state.y) - 16;
    if (!ActorSprite(s, a, 0, x * 65536, y * 65536, 216, 0, 39, 55, 0x15, 0x160))
        return false;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    p->state.width = 40;
    p->state.height = 56;
    p->state.x = x * 65536;
    p->state.y = y * 65536;
    p->state.quad = true;
    p->state.offset_x[1] = p->state.offset_x[3] = 40;
    p->state.offset_y[2] = p->state.offset_y[3] = 56;
    p->state.red = p->state.green = p->state.blue = 0;
    p->state.blend = p->state.modulate = true;
    if (a->state.facing_left)
    {
        p->state.u[0] = p->state.u[2] = 255;
        p->state.u[1] = p->state.u[3] = 216;
    }
    for (Uint16 i = 1; i < 3; ++i)
    {
        ActorPrimitive *side = ActorPrimitiveGet(s, a, i);
        if (side == NULL)
            return false;
        side->state = p->state;
        for (Uint32 edge = 0; edge < 4; ++edge)
        {
            side->state.offset_x[edge] = i == 1 ? 0 : 40;
            side->state.u[edge] = p->state.u[i == 1 ? 0 : 1];
        }
        side->state.red = side->state.green = side->state.blue = 32;
    }
    return true;
}
static bool Ribbons(SotnActors *s, Actor *a)
{
    const Sint32 anchor_x = Pixel(a->state.x) + (a->state.facing_left ? -5 : 8);
    const Sint32 anchor_y = Pixel(a->state.y) + 41;
    for (Uint16 i = 0; i < 8; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(3 + i));
        if (p == NULL)
            return false;
        SotnActorPrimitive *q = &p->state;
        if (!q->active)
        {
            *q = (SotnActorPrimitive){.width = 1,
                                      .height = 1,
                                      .page = 0x14,
                                      .palette = 0x220,
                                      .u = {226, 236, 226, 236},
                                      .v = {208, 208, 208, 208},
                                      .red = 64,
                                      .green = 64,
                                      .blue = 64,
                                      .quad = true,
                                      .active = true,
                                      .textured = true,
                                      .blend = true,
                                      .modulate = true};
        }
        if (a->armor.spread < 92)
            q->v[2] = q->v[3] = (Uint8)(q->v[2] + 1);
        if (s->ticks % 3 == 0)
        {
            const Uint8 u = q->u[0];
            q->u[0] = q->u[2] = q->u[1];
            q->u[1] = q->u[3] = u;
        }
        q->x = anchor_x * 65536;
        q->y = anchor_y * 65536;
        for (Uint16 edge = 0; edge < 2; ++edge)
        {
            const Sint32 angle = (Sint32)(i + edge) * 512 + a->armor.rotation;
            const Sint32 x = ActorShift(-(Sint64)a->armor.depth * ActorSin(angle + 1024), 12);
            const Sint32 z = 1024 + ActorShift((Sint64)a->armor.depth * ActorSin(angle), 12);
            if (z < 900 || z > 1152)
                return SDL_SetError("Armor Lord death ribbon exceeds its projection depth");
            const Sint16 px = (Sint16)((x * 1024) / z);
            const Sint16 py = (Sint16)((-a->armor.spread * 1024) / z);
            q->offset_x[edge] = q->offset_x[edge + 2U] = px;
            q->offset_y[edge] = py;
            q->offset_y[edge + 2U] = 0;
        }
    }
    a->armor.rotation = (Sint16)((a->armor.rotation + 4) & 4095);
    if (a->armor.spread < 104)
        a->armor.spread = (Sint16)(a->armor.spread + 2);
    else if (s->ticks % 4 == 0)
    {
        --a->armor.depth;
        if (a->armor.depth == 25)
            ++a->substep;
    }
    if (a->armor.phase == 2)
        a->armor.spread = (Sint16)(a->armor.spread + 8);
    if (a->armor.depth == 0)
    {
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        if (ActorPrimitives(s, a, 20))
        {
            for (Uint16 i = 0; i < 20; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                if (p == NULL)
                    return false;
                p->state = (SotnActorPrimitive){
                    .x = (anchor_x + (Sint32)(ActorRandom(s) & 3U) - 2) * 65536,
                    .y = (anchor_y - 72 + (Sint32)(ActorRandom(s) & 63U)) * 65536,
                    .width = 1,
                    .height = 1,
                    .red = 224,
                    .green = 160,
                    .blue = 136,
                    .active = true,
                    .blend = true};
                p->timer = (Uint16)((ActorRandom(s) & 7U) + 1);
            }
        }
        else
            SDL_ClearError();
        a->armor.phase = 0;
        a->armor.timer = 0;
        ++a->substep;
    }
    return true;
}
static bool Narrow(SotnActors *s, Actor *a)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    const Uint16 delta = a->state.facing_left ? 255 : 1;
    p->state.u[0] = p->state.u[2] = (Uint8)(p->state.u[0] + delta);
    p->state.u[1] = p->state.u[3] = (Uint8)(p->state.u[1] - delta);
    ++p->state.offset_x[0];
    p->state.offset_x[2] = p->state.offset_x[0];
    --p->state.offset_x[1];
    p->state.offset_x[3] = p->state.offset_x[1];
    return true;
}
bool ActorArmorDeath(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
        if (!ActorPrimitives(s, a, 14))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        if (!Silhouette(s, a))
            return false;
        a->armor.depth = 32;
        a->armor.spread = a->armor.rotation = 0;
        a->armor.timer = a->armor.phase = 0;
        a->state.opacity_set = true;
        a->state.opacity = 128;
        ++a->substep;
        break;
    case 1: {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (p == NULL)
            return false;
        p->state.red = (Uint8)(p->state.red + 2);
        p->state.green = p->state.blue = p->state.red;
        a->state.opacity = (Uint8)(a->state.opacity - 2);
        if (!a->state.opacity)
        {
            a->state.frame = 0;
            p->state.blend = false;
            a->armor.phase = 1;
            if (!ActorSound(s, 0x660))
                return false;
            ++a->substep;
        }
        break;
    }
    case 2:
        ++a->substep;
        break;
    case 3:
        if (s->ticks % 8 == 0)
        {
            if (!Narrow(s, a))
                return false;
            for (Uint16 i = 1; i < 3; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                if (p == NULL)
                    return false;
                const Uint16 edge = i == 1 ? 1 : 0;
                p->state.u[edge] =
                    (Uint8)(p->state.u[edge] + (a->state.facing_left ? -1 : 1) * (i == 1 ? 1 : -1));
                p->state.u[edge + 2U] = p->state.u[edge];
                p->state.offset_x[edge] = (Sint16)(p->state.offset_x[edge] + (i == 1 ? 1 : -1));
                p->state.offset_x[edge + 2U] = p->state.offset_x[edge];
                p->state.offset_y[i == 1 ? 0 : 1] -= 3;
                --p->state.offset_y[i == 1 ? 2 : 3];
            }
            if (++a->armor.timer > 5)
                ++a->substep;
        }
        break;
    case 4:
        break;
    case 5:
        if (s->ticks % 4 == 0)
        {
            if (!Narrow(s, a))
                return false;
            for (Uint16 i = 1; i < 3; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                if (p == NULL)
                    return false;
                p->state.x += i == 1 ? 65536 : -65536;
                for (Uint16 k = 0; k < 4; ++k)
                    p->state.u[k] = (Uint8)(p->state.u[k] +
                                            (a->state.facing_left ? -1 : 1) * (i == 1 ? 1 : -1));
            }
            if (++a->armor.timer > 20)
            {
                a->armor.phase = 2;
                ++a->substep;
            }
        }
        break;
    case 6:
        if (s->ticks % 2 == 0)
        {
            for (Uint16 i = 1; i < 3; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                if (p == NULL)
                    return false;
                const Uint16 edge = i == 1 ? 0 : 1;
                p->state.u[edge] =
                    (Uint8)(p->state.u[edge] + (a->state.facing_left ? -1 : 1) * (i == 1 ? 1 : -1));
                p->state.u[edge + 2U] = p->state.u[edge];
                p->state.offset_x[edge] = (Sint16)(p->state.offset_x[edge] + (i == 1 ? 1 : -1));
                p->state.offset_x[edge + 2U] = p->state.offset_x[edge];
            }
            if (++a->armor.timer > 24)
                ++a->substep;
        }
        break;
    case 7:
        break;
    case 8:
        for (Uint16 i = 0; i < a->state.primitive_count; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL || !p->timer)
                return SDL_SetError("Armor Lord death particle lost its cadence");
            if (s->ticks % p->timer == 0)
                p->state.y -= 65536;
            p->state.red = (Uint8)(p->state.red - 2);
            p->state.green = (Uint8)(p->state.green - 2);
            p->state.blue = (Uint8)(p->state.blue - 2);
        }
        if (a->armor.timer++ > 64)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Armor Lord death entered an invalid phase");
    }
    return a->armor.phase ? Ribbons(s, a) : true;
}
