// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US boss/bo5/e_richter_prologue.c:func_us_801A66B0 and 801A666C,
// sotn-decomp 7bc54d8157b6. Picture coordinates, UVs, palettes and artwork stay private.
#include "actors_internal.h"

static bool Initialize(SotnActors *s, Actor *a)
{
    if (a->state.primitive_count)
        return SDL_SetError("BO5 memory picture cannot initialize over an existing reservation");
    const Uint16 kind = a->state.id == 0x2e;
    SotnDataView positions = {0}, uv = {0}, palettes = {0};
    if (!Slice(s->nz0, 0xcf0U + kind * 16U, 16, &positions) ||
        !Slice(s->nz0, 0xd10U + kind * 16U, 16, &uv) ||
        !Slice(s->nz0, 0xd30U + kind * 8U, 8, &palettes))
        return false;
    for (Uint16 i = 0; i < 4; ++i)
        if (Signed16(positions.bytes + i * 4U) < -1024 ||
            Signed16(positions.bytes + i * 4U) > 1023 ||
            Signed16(positions.bytes + i * 4U + 2) < -1024 ||
            Signed16(positions.bytes + i * 4U + 2) > 1023 ||
            Read16(palettes.bytes + i * 2U) >= 0x300)
            return SDL_SetError("BO5 memory picture has invalid private coordinates or palette");
    // The original retries allocation before initialization; it does not discard this actor.
    if (!ActorPrimitives(s, a, 4))
        return true;
    if (!ActorInitialize(s, a, 0x478))
        return false;
    a->state.animation_set = 0x8000;
    a->state.flags |= 0x800000U;
    a->timer = 0;
    for (Uint16 i = 0; i < 4; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.active = true,
                                        .textured = true,
                                        .screen_space = true,
                                        .quad = true,
                                        .priority_set = true,
                                        .priority = a->state.priority,
                                        .page = 20,
                                        .palette = Read16(palettes.bytes + i * 2U)};
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
        {
            p->state.offset_x[vertex] = (Sint16)Signed16(positions.bytes + i * 4U);
            p->state.offset_y[vertex] = (Sint16)Signed16(positions.bytes + i * 4U + 2);
            p->state.u[vertex] = uv.bytes[i * 4U + (vertex & 1U) * 2U];
            p->state.v[vertex] = uv.bytes[i * 4U + (vertex >> 1) * 2U + 1];
        }
    }
    return true;
}
static void Expand(SotnActorPrimitive *p, Sint16 amount)
{
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->offset_x[i] = (Sint16)Wrap16(p->offset_x[i] + (i & 1U ? amount : -amount));
        p->offset_y[i] = (Sint16)Wrap16(p->offset_y[i] + (i & 2U ? amount : -amount));
    }
}
bool ActorHippogryphMemory(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 25 || !a->state.generation ||
        (a->state.id != 0x2d && a->state.id != 0x2e))
        return SDL_SetError("BO5 memory pictures require their original stage and active owner");
    if (s->hippogryph_skip)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
        return Initialize(s, a);
    if (a->state.step > 9 || a->state.primitive_count != 4)
        return SDL_SetError("BO5 memory picture entered an invalid state or reservation");
    const bool odd = (a->timer & 1U) != 0;
    const Uint16 step = a->state.step;
    if (step != 5)
    {
        const Uint16 index = step < 5 ? step - 1U : 9U - step;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
        if (!p)
            return false;
        const Sint16 amount = step <= 3 ? 1 : step == 4 ? 2 : step == 6 ? -2 : -1;
        if (odd || step == 3 || step == 4 || step == 6 || step == 7)
            Expand(&p->state, amount);
        if (odd && (step == 4 || step == 6))
            for (Uint16 vertex = 0; vertex < 4; ++vertex)
                p->state.offset_y[vertex] =
                    (Sint16)Wrap16(p->state.offset_y[vertex] +
                                   (vertex & 2U ? (step == 4 ? 1 : -1) : (step == 4 ? -1 : 1)));
    }
    a->timer = (Uint16)(a->timer + 1U);
    if (a->timer >= (step == 5 ? 192U : 8U))
    {
        if (step == 9)
            a->remove = true;
        else
        {
            a->timer = 0;
            ++a->state.step;
        }
    }
    return true;
}
