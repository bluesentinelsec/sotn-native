// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_49A6C.c:CreateEntFactoryFromEntity, func_us_801CA2AC.
#include "actors_internal.h"

static bool Vacant(SotnActors *s, Uint32 slot, bool *vacant)
{
    SotnActorPlacement placement = {0};
    Uint32 generation = 0;
    if (!SotnRoomActors_Get(s->spawns, slot, &placement, &generation))
        return false;
    *vacant = generation == 0 && s->slots[slot].state.generation == 0;
    return true;
}
static bool Find(SotnActors *s, Uint32 begin, Uint32 end, bool reverse, Actor **out)
{
    if (begin > end || end > SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("BO4 child pool exceeds its stage-relative slots");
    *out = NULL;
    for (Uint32 i = 0; i < end - begin; ++i)
    {
        const Uint32 slot = reverse ? end - i - 1 : begin + i;
        bool vacant = false;
        if (!Vacant(s, slot, &vacant))
            return false;
        if (vacant)
        {
            *out = &s->slots[slot];
            return true;
        }
    }
    return true;
}
static bool Spawn(SotnActors *s, Actor *target, const Actor *source, Uint16 id, Uint16 parameter)
{
    if (s->effect_serial >= 0x7fffffff || target < s->slots ||
        target >= s->slots + SOTN_STAGE_ACTOR_SLOTS || id >= 56)
        return SDL_SetError("BO4 child identity, generation or destination is invalid");
    const Uint16 slot = (Uint16)(target - s->slots);
    ActorPrimitivesClear(s, target);
    if (id == 0)
    {
        *target = (Actor){0};
        return true;
    }
    *target = (Actor){.state = {.generation = 0x80000000U | ++s->effect_serial,
                                .id = (Uint16)(0x100U | id),
                                .parameters = parameter,
                                .x = source->state.x,
                                .y = source->state.y,
                                .priority = source->state.priority,
                                .facing_left = source->state.facing_left},
                      .slot = slot,
                      .dynamic = true};
    target->enemy_state.owner_slot = source->slot;
    target->enemy_state.owner_generation = source->state.generation;
    return true;
}

bool ActorDoppleFactoryRequest(SotnActors *s, const Actor *source, const SotnDoppleRequest *request)
{
    if (s == NULL || source == NULL || request == NULL || s->stage_id != 26 ||
        request->kind != SOTN_DOPPLE_FACTORY || request->id >= 115 || request->parameter > 255 ||
        source->state.generation == 0)
        return SDL_SetError("BO4 factory requires an original blueprint and active source");
    Actor *factory = NULL;
    if (!Find(s, 4, 16, false, &factory))
        return false;
    if (factory == NULL)
        return true;
    Actor origin = *source;
    origin.state.x = request->x;
    origin.state.y = request->y;
    origin.state.facing_left = request->facing_left;
    if (!Spawn(s, factory, &origin, 1, request->id))
        return false;
    factory->dopple_factory.params = (Uint16)(request->parameter << 8);
    return true;
}

bool ActorDoppleFactory(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x101)
        return SDL_SetError("BO4 factory dispatch received a foreign entity");
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS ||
        a->enemy_state.owner_generation == 0 ||
        s->slots[a->enemy_state.owner_slot].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    if (a->state.step == 0)
    {
        SotnDataView profile = {0};
        if (a->state.parameters >= 115 ||
            !Slice(s->nz0, 0x14b0 + (size_t)a->state.parameters * 6, 6, &profile))
            return SDL_SetError("BO4 factory blueprint is missing or out of range");
        const Uint8 *p = profile.bytes;
        if (p[0] >= 56 || p[1] == 0 || (p[2] & 63U) == 0 || (p[4] & 15U) >= 10)
            return SDL_SetError("BO4 factory blueprint has invalid child, count or pool");
        a->dopple_factory.blueprint = p[0];
        a->dopple_factory.amount = p[1];
        a->dopple_factory.per_cycle = p[2] & 63U;
        a->dopple_factory.noncritical = (p[2] & 128U) != 0;
        a->dopple_factory.increment = (p[2] & 64U) != 0;
        a->dopple_factory.cycle = p[3];
        a->dopple_factory.kind = p[4] & 15U;
        a->dopple_factory.origin = p[4] >> 4;
        a->dopple_factory.delay = p[5];
        a->state.flags |= 0x10000000U;
        a->state.step = 1;
        switch (a->dopple_factory.origin)
        {
        case 0:
        case 6:
            a->state.flags |= 0x08000000U;
            break;
        case 2:
        case 4:
        case 5:
        case 7:
            a->state.x = s->dopple.body.motion.x;
            a->state.y = s->dopple.body.motion.y;
            break;
        }
    }
    else
    {
        switch (a->dopple_factory.origin)
        {
        case 2:
        case 4:
        case 7:
            a->state.x = s->dopple.body.motion.x;
            a->state.y = s->dopple.body.motion.y;
            if ((a->dopple_factory.origin == 4 && s->dopple.body.step != 2) ||
                (a->dopple_factory.origin == 7 && s->dopple.body.step != 11))
            {
                a->remove = true;
                return true;
            }
            break;
        }
    }
    if (a->dopple_factory.delay)
    {
        if (--a->dopple_factory.delay)
            return true;
        a->dopple_factory.delay = a->dopple_factory.cycle;
    }
    SotnDataView range = {0};
    if (a->dopple_factory.kind >= 10 ||
        !Slice(s->nz0, 0x1764 + a->dopple_factory.kind * 2U, 2, &range) || range.bytes[0] < 80 ||
        range.bytes[1] < range.bytes[0] || range.bytes[1] >= 144)
        return SDL_SetError("BO4 factory child range is truncated or invalid");
    for (Uint16 i = 0; i < a->dopple_factory.per_cycle; ++i)
    {
        Actor *child = NULL;
        const Uint16 kind = a->dopple_factory.kind;
        if (kind == 3)
        {
            const Uint32 slot = range.bytes[0] - 64U;
            if (!SotnRoomActors_Release(s->spawns, slot, false))
                return false;
            child = &s->slots[slot];
            s->dopple.body.busy = false;
        }
        else
        {
            const Uint32 pool = a->dopple_factory.spawned % 3U;
            const Uint32 begin =
                kind == 8 ? (pool == 0 ? 17U : 16U + pool * 16U) : range.bytes[0] - 64U;
            const Uint32 end = kind == 8 ? 32U + pool * 16U : range.bytes[1] - 63U;
            if (!Find(s, begin, end, kind == 0, &child))
                return false;
        }
        if (child == NULL)
        {
            if (a->dopple_factory.noncritical)
                a->remove = true;
            else
                a->dopple_factory.delay = a->dopple_factory.cycle;
            return true;
        }
        const Uint16 index = a->dopple_factory.increment ? a->dopple_factory.spawned : i;
        const Uint16 parameter = (Uint16)(a->dopple_factory.params + index);
        if (!Spawn(s, child, a, a->dopple_factory.blueprint, parameter))
            return false;
        // The reconstructed parent follows the factory's original source, not the controller.
        child->enemy_state.owner_slot = a->enemy_state.owner_slot;
        child->enemy_state.owner_generation = a->enemy_state.owner_generation;
        if (++a->dopple_factory.spawned == a->dopple_factory.amount)
        {
            a->remove = true;
            return true;
        }
    }
    a->dopple_factory.delay = a->dopple_factory.cycle;
    return true;
}
