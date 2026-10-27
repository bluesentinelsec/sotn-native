// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted BO0 3053C.c at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Private point, index and duration arrays remain runtime inputs.
#include "olrox_internal.h"

typedef struct MorphTables
{
    SotnDataView from, to, from_indices, to_indices, durations;
} MorphTables;
static bool Tables(SotnActors *s, MorphTables *t)
{
    if (!Slice(s->nz0, 0xdc4, 66U * 4U, &t->from) || !Slice(s->nz0, 0xee0, 66U * 4U, &t->to) ||
        !Slice(s->nz0, 0xffc, 26U * 4U, &t->from_indices) ||
        !Slice(s->nz0, 0x10bc, 39U * 4U, &t->to_indices) ||
        !Slice(s->nz0, 0x117c, 20, &t->durations))
        return false;
    for (Uint16 i = 0; i < 132; ++i)
        if (SDL_abs(Signed16(t->from.bytes + i * 2U)) > 512 ||
            SDL_abs(Signed16(t->to.bytes + i * 2U)) > 512)
            return SDL_SetError("Olrox morph point exceeds its bounded private mesh");
    for (size_t i = 0; i < t->from_indices.size; ++i)
        if (t->from_indices.bytes[i] >= 66)
            return SDL_SetError("Olrox humanoid morph index exceeds its private points");
    for (size_t i = 0; i < t->to_indices.size; ++i)
        if (t->to_indices.bytes[i] >= 66)
            return SDL_SetError("Olrox monster morph index exceeds its private points");
    for (Uint16 i = 0; i < 10; ++i)
    {
        const Sint32 duration = Signed16(t->durations.bytes + i * 2U);
        if (duration < 0 || duration > 4096 || (i < 9 && !duration) || (i == 9 && duration))
            return SDL_SetError(
                "Olrox morph duration requires bounded beats and its final sentinel");
    }
    return true;
}
static void Whole(Sint32 *position, Sint32 pixel)
{
    *position = (Sint32)((Sint64)Wrap16(pixel) * 65536 + ((Uint32)*position & 65535U));
}
static bool Primitive(SotnActors *s, Actor *a, Uint16 index, SotnActorPrimitive **out)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
    if (!p)
        return false;
    *out = &p->state;
    return true;
}
static bool Delay(const MorphTables *t, const Actor *a, Sint16 *out)
{
    if (a->pose >= 10)
        return SDL_SetError("Olrox morph duration cursor exceeds its original private sequence");
    *out = (Sint16)Signed16(t->durations.bytes + a->pose * 2U);
    return true;
}
static void Environment(SotnActorPrimitive *begin, SotnActorPrimitive *restore, bool mesh)
{
    *begin = (SotnActorPrimitive){.active = true,
                                  .priority_set = true,
                                  .priority = mesh ? 0xcf : 0xcb,
                                  .target = 1,
                                  .clip_left = 0,
                                  .clip_right = mesh ? 128 : 127,
                                  .clip_top = 256,
                                  .clip_bottom = mesh ? 384 : 512,
                                  .target_clear = true,
                                  .target_offset_y = mesh ? 256 : 0};
    *restore = (SotnActorPrimitive){
        .active = true, .priority_set = true, .priority = mesh ? 0xd1 : 0xd4, .target = 2};
}
static bool Child(SotnActors *s, Actor *a, Actor **out)
{
    const Uint32 slot = (Uint32)a->slot + 1U;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || s->effect_serial >= 0x7fffffffU)
        return SDL_SetError("Olrox morph monster slot or generation exceeds its owner");
    Actor *old = &s->slots[slot];
    if (old->state.generation)
    {
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, slot, &placement, &placed))
            return false;
        const bool sibling = old->enemy_state.owner_slot == a->enemy_state.owner_slot &&
                             old->enemy_state.owner_generation == a->enemy_state.owner_generation &&
                             a->enemy_state.owner_generation && old->state.id >= 0x2d &&
                             old->state.id <= 0x31;
        if (placed || (!sibling && !OlroxChild(s, a, 1)))
            return SDL_SetError("Olrox morph would replace an unrelated monster slot");
        ActorPrimitivesClear(s, old);
        *old = (Actor){0};
    }
    if (!ActorSpawnAt(s, a, 0x33, a->state.facing_left ? 1 : 0, slot, out))
        return false;
    (*out)->state.x = (s->camera_x + 64) * 65536;
    (*out)->state.y = (s->camera_y + 492) * 65536;
    (*out)->state.facing_left = false;
    a->olrox.children[1] = (*out)->state.generation;
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!ActorInitialize(s, a, 0x6cc))
        return false;
    a->state.hitbox_state = 0;
    a->state.frame = 0;
    a->state.blend_mode |= 0x30U;
    a->state.flags &= ~0x08000000U;
    s->olrox_morph.transforming = true;
    if (!ActorPrimitives(s, a, 68))
    {
        SDL_ClearError();
        a->state.step = 0;
        return true;
    }
    a->state.flags |= 0x800000U;
    for (Uint16 i = 0; i < 68; ++i)
    {
        SotnActorPrimitive *p = NULL;
        if (!Primitive(s, a, i, &p))
            return false;
        *p = (SotnActorPrimitive){.priority_set = true,
                                  .priority = 0xd0,
                                  .screen_space = true,
                                  .textured = true,
                                  .quad = true};
    }
    SotnActorPrimitive *output = NULL;
    if (!Primitive(s, a, 0, &output))
        return false;
    output->page = 0x110;
    output->priority = 0xe0;
    const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
    for (Uint16 i = 0; i < 4; ++i)
    {
        output->u[i] = a->state.facing_left ? (i & 1U ? 0 : 127) : (i & 1U ? 128 : 0);
        output->v[i] = i < 2 ? 0 : 128;
        output->offset_x[i] = (Sint16)Wrap16(
            x + (i & 1U ? (a->state.facing_left ? 60 : 68) : (a->state.facing_left ? -68 : -60)));
        output->offset_y[i] = (Sint16)Wrap16(y + (i < 2 ? -112 : 16));
    }
    return true;
}
static bool Mesh(SotnActors *s, Actor *a, const MorphTables *t)
{
    for (Uint16 i = 0; i < 65; ++i)
    {
        SotnActorPrimitive *p = NULL;
        if (!Primitive(s, a, (Uint16)(3U + i), &p))
            return false;
        const bool old = i < 26;
        const SotnDataView indices = old ? t->from_indices : t->to_indices,
                           points = old ? t->from : t->to;
        const Uint16 face = old ? i : (Uint16)(i - 26U);
        p->active = true;
        p->blend = p->modulate = true;
        p->page = old ? 0x33 : 0x130;
        p->palette = old ? 0x209 : 0x20b;
        p->red = p->green = p->blue = old ? 128 : 0;
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
        {
            const Uint8 point = indices.bytes[face * 4U + vertex];
            p->u[vertex] = (Uint8)((old ? 96 : 0) + Signed16(points.bytes + point * 4U));
            p->v[vertex] = (Uint8)((old ? 64 : 128) + Signed16(points.bytes + point * 4U + 2));
        }
    }
    for (Uint16 i = 0; i < 66; ++i)
    {
        s->olrox_morph.points[i][0] = (Signed16(t->from.bytes + i * 4U) - 16) * 65536;
        s->olrox_morph.points[i][1] = (Signed16(t->from.bytes + i * 4U + 2) - 39) * 65536;
    }
    return true;
}
static bool Vertices(SotnActors *s, Actor *a, const MorphTables *t)
{
    if (a->olrox.morph_timer > 0)
        for (Uint16 i = 0; i < 66; ++i)
            for (Uint16 axis = 0; axis < 2; ++axis)
            {
                const Sint32 from = Signed16(t->from.bytes + i * 4U + axis * 2U),
                             to = Signed16(t->to.bytes + i * 4U + axis * 2U),
                             offset = axis ? -73 : -48;
                s->olrox_morph.points[i][axis] += (to - from + offset) * 512;
            }
    const Uint8 brightness = (Uint8)ActorShift(a->olrox.morph_brightness, 8);
    const Uint8 old_color = (Uint8)SDL_min(brightness, 128);
    for (Uint16 i = 0; i < 65; ++i)
    {
        SotnActorPrimitive *p = NULL;
        if (!Primitive(s, a, (Uint16)(3U + i), &p))
            return false;
        const bool old = i < 26;
        const SotnDataView indices = old ? t->from_indices : t->to_indices;
        const Uint16 face = old ? i : (Uint16)(i - 26U);
        p->red = p->green = p->blue = old ? old_color : (Uint8)(128U - old_color);
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
        {
            const Uint8 point = indices.bytes[face * 4U + vertex];
            p->offset_x[vertex] =
                (Sint16)Wrap16(ActorShift(s->olrox_morph.points[point][0], 16) + 64);
            p->offset_y[vertex] =
                (Sint16)Wrap16(ActorShift(s->olrox_morph.points[point][1], 16) + 112);
        }
    }
    --a->olrox.morph_timer;
    a->olrox.morph_brightness = (Sint16)Wrap16(((Sint32)a->olrox.morph_timer + 16) * 256);
    if (a->olrox.morph_timer < 0 && a->olrox.morph_brightness < 0)
    {
        a->olrox.morph_timer = 32;
        a->state.step = 5;
    }
    return true;
}
static bool Pulse(SotnActors *s, Actor *a, const MorphTables *t, SotnActorPrimitive *p)
{
    if (!(s->ticks & 7U) && !ActorSound(s, 0x817))
        return false;
    Sint16 duration = 0;
    if (!Delay(t, a, &duration))
        return false;
    if (a->substep == 0)
    {
        a->olrox.morph_delay = (Sint16)Wrap16((Sint32)a->olrox.morph_delay - 1);
        if (!a->olrox.morph_delay)
        {
            ++a->pose;
            if (!Delay(t, a, &a->olrox.morph_delay))
                return false;
            a->state.step = 4;
            ++a->substep;
        }
        return true;
    }
    if (a->substep != 1)
        return SDL_SetError("Olrox morph pulse entered an invalid subphase");
    if (a->olrox.morph_delay % 8 == 0)
    {
        const Sint32 direction = a->olrox.morph_delay > duration / 2 ? 1 : -1;
        p->offset_y[0] = p->offset_y[1] = (Sint16)Wrap16(p->offset_y[1] - direction * 2);
        p->offset_x[1] = p->offset_x[3] = (Sint16)Wrap16(p->offset_x[3] + direction);
        p->offset_x[0] = p->offset_x[2] = (Sint16)Wrap16(p->offset_x[2] - direction);
    }
    a->olrox.morph_delay = (Sint16)Wrap16((Sint32)a->olrox.morph_delay - 1);
    if (!a->olrox.morph_delay)
    {
        const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
        p->offset_x[1] = p->offset_x[3] = (Sint16)Wrap16(x + 64);
        p->offset_x[0] = p->offset_x[2] = (Sint16)Wrap16(x - 64);
        p->offset_y[0] = p->offset_y[1] = (Sint16)Wrap16(y - 112);
        ++a->pose;
        if (!Delay(t, a, &a->olrox.morph_delay))
            return false;
        a->state.step = 3;
        --a->substep;
    }
    return true;
}
bool ActorOlroxMorph(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || a->state.id != 0x32 || !a->state.generation)
        return SDL_SetError("Olrox morph requires an active original BO0 owner");
    MorphTables tables = {0};
    if (!Tables(s, &tables))
        return false;
    if (s->olrox_morph.generation && s->olrox_morph.generation != a->state.generation)
        return SDL_SetError("Olrox morph mesh is owned by another actor generation");
    s->olrox_morph.generation = a->state.generation;
    if (!a->state.step)
    {
        if (!Initialize(s, a))
            return false;
        if (!a->state.step)
            return true;
    }
    SotnActorPrimitive *output = NULL, *begin = NULL, *restore = NULL;
    if (!Primitive(s, a, 0, &output) || !Primitive(s, a, 1, &begin) ||
        !Primitive(s, a, 2, &restore))
        return false;
    switch (a->state.step)
    {
    case 1: {
        Environment(begin, restore, false);
        Actor *child = NULL;
        if (!Child(s, a, &child))
            return false;
        ++a->state.step;
        return true;
    }
    case 2:
        if (!Mesh(s, a, &tables))
            return false;
        Environment(begin, restore, true);
        a->state.frame = 0;
        a->state.priority = 0xa0;
        Whole(&a->state.x, output->offset_x[0] + 64 + s->camera_x);
        Whole(&a->state.y, output->offset_y[0] + 112 + s->camera_y);
        a->state.flags |= 0x08000000U;
        a->olrox.morph_timer = 128;
        a->olrox.morph_brightness = -32768;
        a->pose = a->substep = 0;
        if (!Delay(&tables, a, &a->olrox.morph_delay) || !ActorSound(s, 0x819))
            return false;
        ++a->state.step;
        return true;
    case 3:
        output->active = true;
        if (!Vertices(s, a, &tables))
            return false;
        return Pulse(s, a, &tables, output);
    case 4:
        return Pulse(s, a, &tables, output);
    case 5:
        if (--a->olrox.morph_timer == 0)
            ++a->state.step;
        return true;
    case 6: {
        Actor *child = OlroxChild(s, a, 1);
        if (!child || child->state.id != 0x33)
            return SDL_SetError("Olrox morph completion lost its owned monster body");
        s->olrox_morph.transforming = false;
        s->olrox_morph.generation = 0;
        Whole(&child->state.x, Pixel(a->state.x));
        Whole(&child->state.y, 460);
        a->remove = true;
        return true;
    }
    default:
        return SDL_SetError("Olrox morph entered an invalid phase");
    }
}
