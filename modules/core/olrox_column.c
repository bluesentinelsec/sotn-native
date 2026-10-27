// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO0 func_801AE858 at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Original tables stay private.
#include "olrox_internal.h"

static void Quad(SotnActorPrimitive *p, Sint32 left_top, Sint32 right_top, Sint32 left_bottom,
                 Sint32 right_bottom, Sint32 top, Sint32 bottom)
{
    p->offset_x[0] = (Sint16)Wrap16(left_top);
    p->offset_x[1] = (Sint16)Wrap16(right_top);
    p->offset_x[2] = (Sint16)Wrap16(left_bottom);
    p->offset_x[3] = (Sint16)Wrap16(right_bottom);
    p->offset_y[0] = p->offset_y[1] = (Sint16)Wrap16(top);
    p->offset_y[2] = p->offset_y[3] = (Sint16)Wrap16(bottom);
}
static void UV(SotnActorPrimitive *p, Sint32 left, Sint32 right, Sint32 top, Sint32 bottom)
{
    p->u[0] = p->u[2] = (Uint8)left;
    p->u[1] = p->u[3] = (Uint8)right;
    p->v[0] = p->v[1] = (Uint8)top;
    p->v[2] = p->v[3] = (Uint8)bottom;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    SotnDataView delay = {0};
    if (a->state.parameters > 3 || !Slice(s->nz0, 0xbf8 + a->state.parameters * 2U, 2, &delay) ||
        !ActorInitialize(s, a, 0x6f0))
        return SDL_SetError("Olrox skull column requires its original variant and definition");
    a->state.hitbox_width = 16;
    a->state.hitbox_height = 0;
    a->state.scale_set = true;
    a->state.transform_flags_set = true;
    a->state.transform_flags |= SOTN_ACTOR_SCALE_X | SOTN_ACTOR_SCALE_Y;
    a->state.scale_x = 256;
    a->state.scale_y = 0;
    a->state.priority = (Uint16)(a->state.priority + 12U);
    if (!ActorPrimitives(s, a, 24))
    {
        SDL_ClearError();
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    for (Uint16 i = 0; i < 24; ++i)
    {
        ActorPrimitive *primitive = ActorPrimitiveGet(s, a, i);
        if (!primitive)
            return false;
        SotnActorPrimitive *p = &primitive->state;
        *p = (SotnActorPrimitive){.active = i < 12,
                                  .textured = true,
                                  .quad = true,
                                  .screen_space = true,
                                  .priority_set = true,
                                  .priority = (Uint16)(a->state.priority + (i < 12 ? 10U : 8U))};
        if (i < 12)
        {
            p->palette = 0x214;
            p->blend = true;
            p->page = i < 2 || i >= 10 ? 0x33 : 0x32;
            if (i < 2 || i >= 10)
                UV(p, (i & 1U) * 24, (i & 1U) * 24 + 23, i < 2 ? 64 : 94, i < 2 ? 94 : 64);
            else
            {
                const Uint16 row = (Uint16)((i - 2U) / 2U);
                const Sint32 top = 128 + row * 31 + (row == 0 ? 1 : 0);
                UV(p, (i & 1U) * 24 + 208, (i & 1U) * 24 + 231, top, top + 31);
            }
            if (i & 1U)
            {
                const Uint8 left = p->u[0];
                p->u[0] = p->u[2] = p->u[1];
                p->u[1] = p->u[3] = left;
            }
        }
        else
            p->framebuffer = true;
    }
    a->olrox.column_width = 24;
    a->olrox.column_spread = 0;
    a->olrox.timer = Read16(delay.bytes);
    return true;
}
static bool Squeeze(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(a->state.x) - s->camera_x,
                 y = SDL_min(Pixel(a->state.y) - s->camera_y, 240);
    if (a->olrox.column_width)
    {
        --a->olrox.column_width;
        a->olrox.column_spread = (Sint16)Wrap16((Sint32)a->olrox.column_spread + 2);
    }
    for (Uint16 side = 0; side < 2; ++side)
    {
        ActorPrimitive *primitive = ActorPrimitiveGet(s, a, (Uint16)(12U + side));
        if (!primitive)
            return false;
        SotnActorPrimitive *p = &primitive->state;
        p->active = true;
        p->blend = false;
        p->page = 0x100;
        UV(p, side ? x : x - 24, side ? x + 24 : x, 0, y);
        const Sint32 left = side ? x : x - a->olrox.column_width,
                     right = side ? x + a->olrox.column_width : x;
        Quad(p, left, right, left, right, -a->olrox.column_spread, y);
    }
    if (!a->olrox.column_width)
    {
        if (!ActorSound(s, 0x7ca))
            return false;
        for (Uint16 i = 0; i < 7; ++i)
            a->olrox.column_radii[i] = (Sint16)(-(Sint32)i * 32);
        ++a->state.step;
    }
    return true;
}
static bool Wave(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, 0xd50, &animated))
        return false;
    for (Uint16 i = 0; i < 7; ++i)
        a->olrox.column_radii[i] = (Sint16)SDL_min(640, a->olrox.column_radii[i] + 8 - i);
    const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
    for (Uint16 segment = 0; segment < 6; ++segment)
    {
        const Sint32 first = a->olrox.column_radii[segment],
                     last = a->olrox.column_radii[segment + 1U],
                     top = SDL_min(y, Wrap16(y - first)), bottom = SDL_min(y, Wrap16(y - last));
        for (Uint16 side = 0; side < 2; ++side)
        {
            const Uint16 index = (Uint16)(segment * 2U + side);
            ActorPrimitive *beam = ActorPrimitiveGet(s, a, index),
                           *warp = ActorPrimitiveGet(s, a, (Uint16)(12U + index));
            if (!beam || !warp)
                return false;
            SotnMathTrig upper = {0}, lower = {0};
            if (!SotnMath_Trig(s->executable, Wrap16(first * 24 + side * 2048), &upper) ||
                !SotnMath_Trig(s->executable, Wrap16(last * 24 + side * 2048), &lower))
                return false;
            const Sint32 edge = x - 24 + side * 48;
            Quad(&beam->state, edge + upper.cosine / 1024, x, edge + lower.cosine / 1024, x, top,
                 bottom);
            SotnActorPrimitive *p = &warp->state;
            *p = (SotnActorPrimitive){.active = segment != 5,
                                      .textured = true,
                                      .quad = true,
                                      .framebuffer = true,
                                      .screen_space = true,
                                      .priority_set = true,
                                      .priority = (Uint16)(a->state.priority + 8U),
                                      .page = 0x100};
            UV(p, edge, x, top, bottom);
            p->offset_x[0] = beam->state.offset_x[0];
            p->offset_x[1] = beam->state.offset_x[1];
            p->offset_x[2] = beam->state.offset_x[2];
            p->offset_x[3] = beam->state.offset_x[3];
            p->offset_y[0] = p->offset_y[1] = (Sint16)Wrap16(top);
            p->offset_y[2] = p->offset_y[3] = (Sint16)Wrap16(bottom);
            if (top < 0)
            {
                p->v[0] = p->v[1] = 0;
                p->offset_y[0] = p->offset_y[1] = 0;
            }
            if (bottom >= 241)
            {
                p->v[2] = p->v[3] = 240;
                p->offset_y[2] = p->offset_y[3] = 240;
            }
            for (Uint16 vertex = 0; vertex < 2; ++vertex)
            {
                const Sint32 coordinate = p->offset_x[vertex];
                if (coordinate >= 257 || coordinate < 0)
                {
                    p->offset_x[vertex] = p->offset_x[vertex + 2U] = coordinate < 0 ? 0 : 256;
                    p->u[vertex] = p->u[vertex + 2U] = coordinate < 0 ? 0 : 255;
                }
            }
        }
    }
    const Sint32 lead = a->olrox.column_radii[1], penultimate = a->olrox.column_radii[5],
                 tail = a->olrox.column_radii[6];
    if (lead > 0)
    {
        const Sint32 extent =
            penultimate > 0 ? lead - penultimate + (tail > 0 ? (penultimate - tail) / 2 : 0) : lead;
        const Sint32 half = extent / 2;
        a->state.hitbox_height = (Uint8)(half >= 257 ? 255 : half);
        a->state.hitbox_y = (Sint16)Wrap16((Sint32)a->state.hitbox_height - lead);
    }
    if (tail > 0 && a->state.scale_x)
        a->state.scale_x = (Uint16)(a->state.scale_x - 2U);
    if (tail >= 513)
        a->remove = true;
    return true;
}
bool ActorOlroxColumn(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || a->state.id != 0x31 || !a->state.generation)
        return SDL_SetError("Olrox skull column requires an active original BO0 identity");
    if (a->state.step < 3)
    {
        const Uint16 slot = a->enemy_state.owner_slot;
        if (slot >= SOTN_STAGE_ACTOR_SLOTS || !s->slots[slot].state.generation ||
            s->slots[slot].state.id != 0x2b ||
            s->slots[slot].state.generation != a->enemy_state.owner_generation || a->slot <= slot ||
            a->slot - slot > 13 || a->slot - slot < 11 ||
            s->slots[slot].olrox.children[a->slot - slot] != a->state.generation)
        {
            a->remove = true;
            return true;
        }
    }
    Uint8 animated = 0;
    switch (a->state.step)
    {
    case 0:
        return Initialize(s, a);
    case 1:
        if (!ActorAnimate(s, a, 0xd48, &animated))
            return false;
        if (!animated)
        {
            a->state.step = 2;
            OlroxSubstep(a, 0);
            a->pose = (Uint16)(ActorRandom(s) & 7U);
        }
        if ((Sint16)a->state.scale_y < 256)
            a->state.scale_y = (Uint16)(a->state.scale_y + 16U);
        return true;
    case 2:
        if (!ActorAnimate(s, a, 0xd50, &animated))
            return false;
        if (s->slots[a->enemy_state.owner_slot].olrox.fire_skulls)
        {
            if (a->olrox.timer)
            {
                --a->olrox.timer;
                return true;
            }
            ++a->state.step;
        }
        if ((Sint16)a->state.scale_y < 256)
            a->state.scale_y = (Uint16)(a->state.scale_y + 16U);
        if (s->olrox_flags)
            a->remove = true;
        return true;
    case 3:
        return Squeeze(s, a);
    case 4:
        return Wave(s, a);
    default:
        return SDL_SetError("Olrox skull column entered an invalid phase");
    }
}
