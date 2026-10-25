#include "actors_internal.h"
bool ActorPrimitives(SotnActors *s, Actor *a, Uint16 count)
{
    if (s == NULL || a == NULL || count > ACTOR_PRIMITIVE_LIMIT || a->state.primitive_count != 0 ||
        a->state.generation == 0)
        return SDL_SetError(
            "Actor primitive allocation requires an active owner and bounded count");
    Uint16 available = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        if (s->primitives[i].owner == 0 && available < count)
            a->primitive[available++] = (Uint16)i;
    if (available < count)
        return SDL_SetError("Laboratory actor primitive pool is full");
    for (Uint16 i = 0; i < count; ++i)
        s->primitives[a->primitive[i]] = (ActorPrimitive){.owner = a->state.generation};
    a->state.primitive_count = (Uint8)count;
    return true;
}
ActorPrimitive *ActorPrimitiveGet(SotnActors *s, const Actor *a, Uint16 index)
{
    if (s == NULL || a == NULL || index >= a->state.primitive_count ||
        index >= ACTOR_PRIMITIVE_LIMIT || a->primitive[index] >= ACTOR_PRIMITIVE_CAPACITY ||
        s->primitives[a->primitive[index]].owner != a->state.generation)
    {
        SDL_SetError("Actor primitive index or owner generation is invalid");
        return NULL;
    }
    return &s->primitives[a->primitive[index]];
}
void ActorPrimitivesClear(SotnActors *s, const Actor *a)
{
    if (s == NULL || a == NULL)
        return;
    for (Uint32 i = 0; i < a->state.primitive_count && i < ACTOR_PRIMITIVE_LIMIT; ++i)
        if (a->primitive[i] < ACTOR_PRIMITIVE_CAPACITY &&
            s->primitives[a->primitive[i]].owner == a->state.generation)
            s->primitives[a->primitive[i]] = (ActorPrimitive){0};
}
bool SotnActors_GetPrimitive(const SotnActors *s, Uint32 slot, Uint32 index,
                             SotnActorPrimitive *out)
{
    if (s == NULL || out == NULL || slot >= SOTN_STAGE_ACTOR_SLOTS ||
        index >= ACTOR_PRIMITIVE_LIMIT)
        return SDL_SetError("Actor primitive read exceeds the slot or part bound");
    const Actor *a = &s->slots[slot];
    if (index >= a->state.primitive_count)
    {
        *out = (SotnActorPrimitive){0};
        return true;
    }
    const Uint16 id = a->primitive[index];
    if (id >= ACTOR_PRIMITIVE_CAPACITY || s->primitives[id].owner != a->state.generation)
        return SDL_SetError("Actor primitive generation is stale");
    *out = s->primitives[id].state;
    return true;
}
static bool Resolve(SotnDataView image, Uint32 address, size_t bytes, Uint32 alignment,
                    SotnDataView *out)
{
    if (address < 0x80180000 || address % alignment != 0)
        return SDL_SetError("Actor sprite data address is invalid");
    return Slice(image, address - 0x80180000, bytes, out);
}
bool ActorStageParts(SotnActors *s, const Actor *a, SotnDataView *out)
{
    const Uint32 bank = a->state.animation_set & 0x7fff;
    const Uint32 bank_count = SotnStageProfile_Get(s->stage_id)->sprite_banks;
    SotnDataView banks = {0}, frames = {0}, header = {0};
    if (!(a->state.animation_set & 0x8000) || bank == 0 || bank >= bank_count || s->nz0.size < 24 ||
        !Resolve(s->nz0, Read32(s->nz0.bytes + 20), bank_count * 4, 4, &banks) ||
        !Resolve(s->nz0, Read32(banks.bytes + bank * 4), 8, 4, &frames))
        return false;
    const Uint32 start = Read32(banks.bytes + bank * 4), end = Read32(frames.bytes + 4);
    if (end <= start || end % 4 != 0 || end - start > 4096 || a->state.frame >= (end - start) / 4 ||
        !Resolve(s->nz0, start, end - start, 4, &frames) ||
        !Resolve(s->nz0, Read32(frames.bytes + a->state.frame * 4), 2, 2, &header))
        return false;
    const Uint16 count = Read16(header.bytes);
    if (count == 0 || count > ACTOR_SPRITE_PART_LIMIT)
        return SDL_SetError("Actor death frame exceeds the native part budget");
    return Resolve(s->nz0, Read32(frames.bytes + a->state.frame * 4), 2 + (size_t)count * 22, 2,
                   out);
}
