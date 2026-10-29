#include "actors_internal.h"
static bool Reserve(SotnActors *s, Actor *a, Uint16 count, bool contiguous)
{
    if (s == NULL || a == NULL || count > ACTOR_PRIMITIVE_LIMIT || a->state.primitive_count != 0 ||
        a->state.generation == 0)
        return SDL_SetError(
            "Actor primitive allocation requires an active owner and bounded count");
    Uint16 available = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY && available < count; ++i)
    {
        if (s->primitives[i].owner == 0)
            a->primitive[available++] = (Uint16)i;
        else if (contiguous)
            available = 0;
    }
    if (available < count)
        return SDL_SetError("Laboratory actor primitive pool is full");
    for (Uint16 i = 0; i < count; ++i)
        s->primitives[a->primitive[i]] = (ActorPrimitive){.owner = a->state.generation};
    a->state.primitive_count = count;
    return true;
}
bool ActorPrimitives(SotnActors *s, Actor *a, Uint16 count)
{
    return Reserve(s, a, count, false);
}
bool ActorPrimitivesContiguous(SotnActors *s, Actor *a, Uint16 count)
{
    return Reserve(s, a, count, true);
}
ActorPrimitive *ActorPrimitiveGet(SotnActors *s, const Actor *a, Uint16 index)
{
    if (s == NULL || a == NULL || index >= a->state.primitive_count ||
        index >= ACTOR_PRIMITIVE_LIMIT || a->primitive[index] >= ACTOR_PRIMITIVE_CAPACITY ||
        s->primitives[a->primitive[index]].owner != a->state.generation ||
        s->primitives[a->primitive[index]].ability_kind)
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
            s->primitives[a->primitive[i]].owner == a->state.generation &&
            !s->primitives[a->primitive[i]].ability_kind)
            s->primitives[a->primitive[i]] = (ActorPrimitive){0};
}
bool SotnActors_HasTargets(const SotnActors *s)
{
    if (!s)
        return false;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        if (s->primitives[i].owner && s->primitives[i].state.active &&
            s->primitives[i].state.target)
            return true;
    return false;
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
    if (id >= ACTOR_PRIMITIVE_CAPACITY || s->primitives[id].owner != a->state.generation ||
        s->primitives[id].ability_kind)
        return SDL_SetError("Actor primitive generation is stale");
    *out = s->primitives[id].state;
    return true;
}
bool SotnActors_PrimitiveSlots(const SotnActors *s, SotnPrimitiveSlots *out)
{
    if (!s || !out)
        return SDL_SetError("Primitive availability requires its actual pool and output");
    SotnPrimitiveSlots slots = {0};
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].owner)
            slots.occupied[i / 64U] |= (Uint64)1 << (i % 64U);
    *out = slots;
    return true;
}
static bool EchoValid(const SotnBatEcho *echo)
{
    return echo && echo->phase && echo->phase <= 3 && !echo->removed && echo->priority <= 509;
}
static bool BatPrimitiveKind(Uint16 kind)
{
    return kind == 5 || kind == 11 || kind == 31 || kind == 35 || kind == 52 || kind == 61 ||
           kind == 63;
}
static bool WolfPrimitiveKind(Uint16 kind)
{
    return kind == 56 || kind == SOTN_WOLF_ENVIRONMENT_TAG || kind == SOTN_PLAYER_AFTERIMAGE_TAG;
}
static bool PlanRows(const SotnActors *s, SotnPrimitiveSlots *used, const Uint16 *indices,
                     Uint16 count, Uint16 replace_kind, bool include_wolf)
{
    if (!indices)
        return SDL_SetError("Player effect reservation has no actual row indices");
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint16 id = indices[i];
        if (id >= SOTN_PRIMITIVE_SLOTS || (used->occupied[id / 64U] & ((Uint64)1 << (id % 64U))) ||
            (s->primitives[id].owner &&
             ((!BatPrimitiveKind(s->primitives[id].ability_kind) &&
               !(include_wolf && WolfPrimitiveKind(s->primitives[id].ability_kind))) ||
              (replace_kind && s->primitives[id].ability_kind != replace_kind))))
            return SDL_SetError("Player effect reservation collides with a live primitive owner");
        used->occupied[id / 64U] |= (Uint64)1 << (id % 64U);
    }
    return true;
}
static void WriteEcho(SotnActors *s, Uint32 generation, const Uint16 *indices,
                      const SotnBatEcho *echo)
{
    for (Uint16 i = 0; i < SOTN_BAT_ECHO_PRIMITIVES; ++i)
    {
        ActorPrimitive *p = &s->primitives[indices[i]];
        *p = (ActorPrimitive){.owner = generation, .ability_kind = 52};
        if (i < SOTN_BAT_ECHO_DRIVERS)
            continue;
        const SotnBatEchoQuad *q = &echo->quads[i - SOTN_BAT_ECHO_DRIVERS];
        p->state = (SotnActorPrimitive){.active = !echo->hidden,
                                        .screen_space = true,
                                        .priority_set = true,
                                        .priority = echo->priority,
                                        .page = 0x60,
                                        .blend = true,
                                        .gouraud = true,
                                        .quad = true};
        for (Uint16 v = 0; v < 4; ++v)
        {
            p->state.offset_x[v] = q->x[v];
            p->state.offset_y[v] = q->y[v];
            p->state.vertex_red[v] = q->red[v];
            p->state.vertex_green[v] = q->green[v];
            p->state.vertex_blue[v] = q->blue[v];
        }
    }
}
bool SotnActors_CommitBatEcho(SotnActors *s, Uint32 generation, const Uint16 *indices,
                              const SotnBatEcho *echo)
{
    if (!s || (generation && !EchoValid(echo)) || (!generation && (indices || echo)))
        return SDL_SetError("Echo commit requires its actual child, reservation and live packets");
    SotnPrimitiveSlots used = {0};
    if (generation && !PlanRows(s, &used, indices, SOTN_BAT_ECHO_PRIMITIVES, 52, false))
        return false;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].ability_kind == 52)
            s->primitives[i] = (ActorPrimitive){0};
    if (generation)
        WriteEcho(s, generation, indices, echo);
    return true;
}
bool SotnActors_CommitBatPrimitives(SotnActors *s, const SotnBatRuntime *r)
{
    if (!s || !r)
        return SDL_SetError("Bat primitive commit requires its actual session and runtime owner");
    for (Uint16 i = 0; i < SOTN_SPELL_TARGETS; ++i)
        if (r->target_marks[i / 64U] & ((Uint64)1 << (i % 64U)))
        {
            const SotnActorState actual = s->slots[i].state;
            if (!r->target_generations[i] || !r->target_identities[i] ||
                actual.generation != r->target_generations[i] ||
                actual.id != r->target_identities[i])
                return SDL_SetError("Spirit mark lost its actual target%u identity/generation", i);
        }
    SotnPrimitiveSlots used = {0};
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        const SotnBatRuntimeChild *c = &r->children[i];
        if ((c->kind == 61 &&
             (i < 1 || i > 31 || c->parameter != c->state.spirit.motion.parameter ||
              c->state.spirit.motion.removed ||
              (c->initialized ? !c->state.spirit.motion.phase || c->state.spirit.motion.phase > 3 ||
                                    c->primitive_count != 16 || c->group < 3 || c->group > 10 ||
                                    (i < 16 ? c->group > 6 : c->group < 7)
                              : c->state.spirit.motion.phase || c->primitive_count))) ||
            (c->kind == 63 &&
             (c->parameter != c->state.portal.parameter || c->state.portal.removed ||
              (c->initialized ? !c->state.portal.phase || c->state.portal.phase > 3 ||
                                    (c->state.portal.phase == 1 ? c->primitive_count != 0
                                                                : c->primitive_count != 9)
                              : c->state.portal.phase || c->primitive_count))))
            return SDL_SetError("Spirit primitive commit lost its actual child/lifecycle rows");
        if (c->kind == 31 && c->initialized &&
            (c->state.outline.parameter != c->parameter || c->state.outline.removed ||
             !c->state.outline.phase || c->state.outline.phase > 3))
            return SDL_SetError("Player outline commit lost its actual child/profile lifecycle");
        if (((c->kind == 5 || c->kind == 11 || c->kind == 31 || c->kind == 35) && c->initialized &&
             !c->primitive_count) ||
            (c->kind == 52 && c->initialized && r->echo_generation != c->generation))
            return SDL_SetError("Bat primitive commit lost a retained child reservation");
        if (c->primitive_count &&
            (!c->generation || !c->initialized ||
             !((c->kind == 5 && c->state.particles.phase == 1 && !c->state.particles.removed &&
                c->state.particles.count &&
                c->state.particles.count <= SOTN_PLAYER_PARTICLE_LIMIT &&
                c->primitive_count == c->state.particles.count + 1U) ||
               (c->kind == 11 && c->primitive_count == 8) ||
               ((c->kind == 31 || c->kind == 35) && c->primitive_count == 1) ||
               (c->kind == 61 && c->primitive_count == 16) ||
               (c->kind == 63 && c->primitive_count == 9))))
            return SDL_SetError("Bat primitive reservation lost its actual initialized child");
        if (c->primitive_count &&
            !PlanRows(s, &used, c->primitives, c->primitive_count, 0, r->wolf_bound))
            return false;
    }
    if (r->echo_generation &&
        (r->children[31].kind != 52 || r->children[31].generation != r->echo_generation ||
         !r->children[31].initialized || !EchoValid(&r->echo) ||
         !PlanRows(s, &used, r->echo_primitives, SOTN_BAT_ECHO_PRIMITIVES, 0, r->wolf_bound)))
        return SDL_SetError("Bat Echo primitive reservation lost its actual child or free rows");
    const SotnWolfRuntime *w = &r->wolf;
    if (r->common_generation && (!r->wolf_bound || !w->environment_reserved ||
                                 r->common_generation != w->environment_generation ||
                                 !PlanRows(s, &used, r->afterimage_rows, 6, 0, true)))
        return SDL_SetError("Common player commit lost its original eight-row lease");
    bool common_owned = false;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].ability_kind == SOTN_PLAYER_AFTERIMAGE_TAG)
        {
            common_owned = true;
            bool retained = false;
            for (Uint16 j = 0; j < 6; ++j)
                retained = retained || r->afterimage_rows[j] == i;
            if (!r->common_generation || s->primitives[i].owner != r->common_generation ||
                !retained)
                return SDL_SetError("Common player commit cannot detach its live stage lease");
        }
    if (common_owned)
        for (Uint16 i = 0; i < 2; ++i)
            if (w->environment_rows[i] >= SOTN_PRIMITIVE_SLOTS ||
                s->primitives[w->environment_rows[i]].ability_kind != SOTN_WOLF_ENVIRONMENT_TAG ||
                s->primitives[w->environment_rows[i]].owner != r->common_generation)
                return SDL_SetError("Common player commit cannot replace its environment rows");
    if ((w->root_reserved || w->environment_reserved) && !r->wolf_bound)
        return SDL_SetError("Shared primitive commit lost its actual Wolf lifetime binding");
    if (w->body.parts[0].step > 1 || w->root_reserved != (w->body.parts[0].step == 1) ||
        (w->root_reserved &&
         (!w->generation || !w->children[0].generation || w->children[0].lineage != w->generation ||
          w->children[0].slot != 17 || w->children[0].identity != 56)) ||
        w->environment_reserved != (w->environment_generation != 0) ||
        (w->root_reserved && !PlanRows(s, &used, w->root_rows, 6, 0, true)) ||
        (w->environment_reserved && !PlanRows(s, &used, w->environment_rows, 2, 0, true)))
        return SDL_SetError("Shared primitive commit has an invalid actual Wolf reservation");
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (BatPrimitiveKind(s->primitives[i].ability_kind) ||
            (r->wolf_bound && WolfPrimitiveKind(s->primitives[i].ability_kind)))
            s->primitives[i] = (ActorPrimitive){0};
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        const SotnBatRuntimeChild *c = &r->children[i];
        for (Uint16 j = 0; j < c->primitive_count; ++j)
            s->primitives[c->primitives[j]] =
                (ActorPrimitive){.owner = c->generation, .ability_kind = c->kind};
    }
    if (r->echo_generation)
        WriteEcho(s, r->echo_generation, r->echo_primitives, &r->echo);
    if (w->root_reserved)
        for (Uint16 i = 0; i < 6; ++i)
            s->primitives[w->root_rows[i]] =
                (ActorPrimitive){.owner = w->children[0].generation, .ability_kind = 56};
    if (w->environment_reserved)
        for (Uint16 i = 0; i < 2; ++i)
            s->primitives[w->environment_rows[i]] = (ActorPrimitive){
                .owner = w->environment_generation, .ability_kind = SOTN_WOLF_ENVIRONMENT_TAG};
    if (r->common_generation)
        for (Uint16 i = 0; i < 6; ++i)
            s->primitives[r->afterimage_rows[i]] = (ActorPrimitive){
                .owner = r->common_generation, .ability_kind = SOTN_PLAYER_AFTERIMAGE_TAG};
    for (Uint16 i = 0; i < SOTN_SPELL_TARGETS; ++i)
        if (r->target_marks[i / 64U] & ((Uint64)1 << (i % 64U)))
            s->slots[i].state.flags |= 0x80000U;
    return true;
}
bool SotnActors_CommitWolfPrimitives(SotnActors *s, const SotnWolfRuntime *r)
{
    if (!s || !r || r->body.parts[0].step > 1 || r->root_reserved != (r->body.parts[0].step == 1) ||
        (r->root_reserved &&
         (!r->generation || !r->children[0].generation || r->children[0].lineage != r->generation ||
          r->children[0].slot != 17 || r->children[0].identity != 56)) ||
        r->environment_reserved != (r->environment_generation != 0))
        return SDL_SetError("Wolf primitive commit requires its actual root/environment owners");
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].ability_kind == SOTN_PLAYER_AFTERIMAGE_TAG)
            return SDL_SetError("Common player rows require the shared primitive commit");
    SotnPrimitiveSlots used = {0};
    const Uint16 *rows[2] = {r->root_rows, r->environment_rows};
    const Uint16 counts[2] = {(Uint16)(r->root_reserved ? 6 : 0),
                              (Uint16)(r->environment_reserved ? 2 : 0)};
    const Uint16 kinds[2] = {56, SOTN_WOLF_ENVIRONMENT_TAG};
    const Uint32 owners[2] = {r->children[0].generation, r->environment_generation};
    for (Uint16 owner = 0; owner < 2; ++owner)
        for (Uint16 i = 0; i < counts[owner]; ++i)
        {
            const Uint16 id = rows[owner][i];
            if (id >= SOTN_PRIMITIVE_SLOTS ||
                (used.occupied[id / 64U] & ((Uint64)1 << (id % 64U))) ||
                (s->primitives[id].owner && (s->primitives[id].owner != owners[owner] ||
                                             s->primitives[id].ability_kind != kinds[owner])))
                return SDL_SetError("Wolf primitive reservation collides with an actual owner");
            used.occupied[id / 64U] |= (Uint64)1 << (id % 64U);
        }
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].ability_kind == 56 ||
            s->primitives[i].ability_kind == SOTN_WOLF_ENVIRONMENT_TAG)
            s->primitives[i] = (ActorPrimitive){0};
    for (Uint16 owner = 0; owner < 2; ++owner)
        for (Uint16 i = 0; i < counts[owner]; ++i)
            s->primitives[rows[owner][i]] =
                (ActorPrimitive){.owner = owners[owner], .ability_kind = kinds[owner]};
    return true;
}
bool SotnActors_GetBatEchoPrimitive(const SotnActors *s, Uint32 generation, Uint16 index,
                                    SotnActorPrimitive *out)
{
    if (!s || !out || !generation || index >= SOTN_PRIMITIVE_SLOTS ||
        s->primitives[index].ability_kind != 52 || s->primitives[index].owner != generation)
        return SDL_SetError("Echo packet read requires its actual primitive and child generation");
    *out = s->primitives[index].state;
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
