#include "actors_internal.h"
#include "bat_runtime_internal.h"
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
static bool FairyLease(const SotnFairyEffectLease *lease, SotnPrimitiveSlots *rows)
{
    if (!lease || !rows || !lease->generation || lease->slot < 5 || lease->slot > 7 ||
        (lease->identity != 0xdd && lease->identity != 0xdf) ||
        (lease->identity == 0xdd ? lease->count != SOTN_FAIRY_APPLE_ROWS
                                 : lease->count < 2 || lease->count > SOTN_FAIRY_ITEM_ROWS))
        return SDL_SetError("Fairy effect lease requires its actual DD/DF slot/generation/rows");
    SotnPrimitiveSlots used = {0};
    for (Uint16 i = 0; i < lease->count; ++i)
    {
        const Uint16 id = lease->rows[i];
        if (id >= SOTN_PRIMITIVE_SLOTS || (used.occupied[id / 64U] & ((Uint64)1 << (id % 64U))))
            return SDL_SetError("Fairy effect lease contains a duplicate or out-of-range row");
        used.occupied[id / 64U] |= (Uint64)1 << (id % 64U);
    }
    *rows = used;
    return true;
}
static Uint16 FairyTag(const SotnFairyEffectLease *lease)
{
    return (Uint16)(0x8000U | (Uint16)(lease->slot << 8) | lease->identity);
}
bool SotnActors_CommitFairyEffect(SotnActors *s, const SotnFairyEffectLease *before,
                                  const SotnFairyEffectLease *after)
{
    if (!s || (!before && !after) || (before && after && before->slot != after->slot))
        return SDL_SetError("Fairy effect commit requires one actual event slot and stage pool");
    SotnPrimitiveSlots old_rows = {0}, next_rows = {0};
    if ((before && !FairyLease(before, &old_rows)) || (after && !FairyLease(after, &next_rows)))
        return false;
    const Uint16 slot = before ? before->slot : after->slot;
    if (before && after && before->generation == after->generation &&
        (before->identity != after->identity || before->count != after->count ||
         SDL_memcmp(before->rows, after->rows, before->count * sizeof(*before->rows))))
        return SDL_SetError("A live Fairy generation cannot resize or replace its actual lease");
    for (Uint16 id = 0; id < SOTN_PRIMITIVE_SLOTS; ++id)
    {
        const ActorPrimitive *p = &s->primitives[id];
        const bool old = (old_rows.occupied[id / 64U] & ((Uint64)1 << (id % 64U))) != 0;
        const bool next = (next_rows.occupied[id / 64U] & ((Uint64)1 << (id % 64U))) != 0;
        const bool own = (p->ability_kind & 0xff00U) == (0x8000U | slot << 8) &&
                         ((p->ability_kind & 255U) == 0xdd || (p->ability_kind & 255U) == 0xdf);
        if ((old &&
             (!before || p->owner != before->generation || p->ability_kind != FairyTag(before))) ||
            (own && !old) || (next && p->owner && !old))
            return SDL_SetError(
                "Fairy effect lease is stale, incomplete or collides with a real row");
    }
    if (before)
        for (Uint16 i = 0; i < before->count; ++i)
            s->primitives[before->rows[i]] = (ActorPrimitive){0};
    if (after)
        for (Uint16 i = 0; i < after->count; ++i)
            s->primitives[after->rows[i]] =
                (ActorPrimitive){.owner = after->generation, .ability_kind = FairyTag(after)};
    return true;
}
bool SotnActors_FairyEffectRow(const SotnActors *s, const SotnFairyEffectLease *lease, Uint16 index)
{
    SotnPrimitiveSlots used = {0};
    if (!s || !FairyLease(lease, &used) || index >= lease->count)
        return SDL_SetError("Fairy row read requires its actual event lease and bounded index");
    const ActorPrimitive *p = &s->primitives[lease->rows[index]];
    if (p->owner != lease->generation || p->ability_kind != FairyTag(lease))
        return SDL_SetError("Fairy row read lost its actual event generation");
    return true;
}
bool SotnActors_ValidateFairyEffects(const SotnActors *s, const SotnFairyEffectLease leases[3])
{
    if (!s || !leases)
        return SDL_SetError("Fairy namespace validation requires its actual pool and three leases");
    SotnPrimitiveSlots rows[3] = {0};
    for (Uint16 i = 0; i < 3; ++i)
        if (leases[i].count && (leases[i].slot != i + 5U || !FairyLease(&leases[i], &rows[i])))
            return SDL_SetError("Fairy namespace lease lost its actual5..7 slot or bounded rows");
    for (Uint16 id = 0; id < SOTN_PRIMITIVE_SLOTS; ++id)
    {
        const ActorPrimitive *p = &s->primitives[id];
        const Uint16 slot = (Uint16)(p->ability_kind >> 8);
        const Uint16 identity = p->ability_kind & 255U;
        const bool own = slot >= 0x85 && slot <= 0x87 && (identity == 0xdd || identity == 0xdf);
        Uint16 matches = 0;
        for (Uint16 i = 0; i < 3; ++i)
            if (rows[i].occupied[id / 64U] & ((Uint64)1 << (id % 64U)))
            {
                if (++matches > 1 || p->owner != leases[i].generation ||
                    p->ability_kind != FairyTag(&leases[i]))
                    return SDL_SetError(
                        "Fairy namespace contains overlapping or stale actual rows");
            }
        if (own && !matches)
            return SDL_SetError("Fairy namespace contains an unclaimed actual DD/DF row");
    }
    return true;
}
bool SotnActors_ReleaseFairyEffects(SotnActors *s, const SotnFairyEffectLease leases[3])
{
    if (!SotnActors_ValidateFairyEffects(s, leases))
        return false;
    for (Uint16 i = 0; i < 3; ++i)
        for (Uint16 row = 0; row < leases[i].count; ++row)
            s->primitives[leases[i].rows[row]] = (ActorPrimitive){0};
    return true;
}
static bool EchoValid(const SotnBatEcho *echo)
{
    return echo && echo->phase && echo->phase <= 3 && !echo->removed && echo->priority <= 509;
}
static bool BatPrimitiveKind(Uint16 kind)
{
    return kind == 5 || kind == 11 || kind == 19 || kind == 31 || kind == 35 || kind == 37 ||
           kind == 46 || kind == 52 || kind == 61 || kind == 63 || kind == 25 || kind == 28 ||
           kind == 29 || kind == 47 || kind == 30 || kind == 32 || kind == 33 || kind == 65 ||
           kind == 38 || kind == 0xe0 || kind == 0xf0 || kind == 0xe1 || kind == 0xf1 ||
           kind == 0xe2 || kind == 0xf2 || kind == 0xe3 || kind == 0xf3 || kind == 0xe4 ||
           kind == 0xf4;
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
               s->primitives[id].ability_kind != SOTN_FOOD_TEXT_TAG &&
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
static bool CommitBatPrimitives(SotnActors *s, const SotnBatRuntime *r, bool apply)
{
    if (!s || !r)
        return SDL_SetError("Bat primitive commit requires its actual session and runtime owner");
    SotnFoodTextPlan food = r->food_text;
    if (food.changed ? !ActorFoodTextPlanCheck(s, &food) : !SotnActors_GetFoodTextPlan(s, &food))
        return false;
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
        if (!SotnBatRuntime_HellfireChildValid(c, i))
            return SDL_SetError("Hellfire primitive commit lost its actual child lifecycle");
        if (!SotnBatRuntime_TalismanValid(c, i))
            return SDL_SetError("Talisman commit lost its actual child/lifetime/rows");
        if (!SotnBatRuntime_ImmunityTextValid(c))
            return SDL_SetError("Immunity text lost its actual child/profile/row ownership");
        if (!SotnBatRuntime_DissolveValid(c))
            return SDL_SetError("Dissolve commit lost its actual thirty-six-row ownership");
        if (!SotnBatRuntime_MedicineValid(c))
            return SDL_SetError("Medicine commit lost its actual rowless child/cursor ownership");
        if (!SotnBatRuntime_CircleValid(c))
            return SDL_SetError("Player circle commit lost its actual thirty-two-row ownership");
        if (!SotnBatRuntime_TeleportValid(c))
            return SDL_SetError("Teleport commit lost its actual thirty-six-row ownership");
        if (!SotnBatRuntime_IceHitValid(c))
            return SDL_SetError("Ice hit lost its actual twenty-four-row child ownership");
        if (!SotnBatRuntime_ThunderHitValid(c))
            return SDL_SetError("Thunder hit lost its actual six-row child ownership");
        if (!SotnBatRuntime_FireHitValid(c))
            return SDL_SetError("Fire effects lost their actual shared child/parent ownership");
        if (!SotnBatRuntime_DarkHitValid(c))
            return SDL_SetError("Dark hit lost its actual rowless child ownership");
        if (!SotnBatRuntime_HolyHitValid(c))
            return SDL_SetError("Holy hit lost its actual child/global/row ownership");
        if (!SotnBatRuntime_HealingNumberValid(c, i))
            return SDL_SetError("Healing number commit lost its actual child/lifetime/rows");
        if (!SotnBatRuntime_SwordChildValid(c, i))
            return SDL_SetError("Sword commit lost its actual child/lifetime/slot ownership");
        if (!SotnBatRuntime_MagicMissileValid(c, i))
            return SDL_SetError("Magic Missile commit lost its actual bow/arrow ownership");
        if (!SotnBatRuntime_StarValid(c, i))
            return SDL_SetError("Weapon15 commit lost actual role/row ownership");
        if (!SotnBatRuntime_ThrownValid(c, i))
            return SDL_SetError("Weapon16 commit lost its actual role/row/parent ownership");
        if (!SotnBatRuntime_PentagramValid(c, i))
            return SDL_SetError("Pentagram commit lost its actual twenty-five-row lifecycle");
        if (!SotnBatRuntime_SireValid(c, i))
            return SDL_SetError("Sire commit lost its actual child/lifecycle ownership");
        if (!SotnBatRuntime_NeutronValid(c, i))
            return SDL_SetError("Neutron commit lost its actual child/lifetime/row ownership");
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
        if (((c->kind == 5 || c->kind == 11 || c->kind == 46 || c->kind == 25 || c->kind == 28 ||
              c->kind == 29 || c->kind == 31 || c->kind == 35) &&
             c->initialized && !c->primitive_count) ||
            (c->kind == 52 && c->initialized && r->echo_generation != c->generation))
            return SDL_SetError("Bat primitive commit lost a retained child reservation");
        if (c->primitive_count &&
            (!c->generation || !c->initialized ||
             !((c->kind == 5 && c->state.particles.phase == 1 && !c->state.particles.removed &&
                c->state.particles.count &&
                c->state.particles.count <= SOTN_PLAYER_PARTICLE_LIMIT &&
                c->primitive_count == c->state.particles.count + 1U) ||
               ((c->kind == 11 || c->kind == 46) && c->primitive_count == 8) ||
               ((c->kind == 25 || c->kind == 28 || c->kind == 29 || c->kind == 31 ||
                 c->kind == 35) &&
                c->primitive_count == 1) ||
               (c->kind == 61 && c->primitive_count == 16) ||
               (c->hand_weapon && c->state.hand.family == 43 && c->primitive_count == 16) ||
               (c->hand_weapon && c->state.hand.family == 1 &&
                c->primitive_count == SOTN_HAND_WEAPON_FIST_ROWS) ||
               (c->hand_weapon && (c->state.hand.family == 54 || c->state.hand.family == 56) &&
                c->primitive_count == SOTN_HAND_WEAPON_GURTHANG_ROWS) ||
               (c->weapon_secondary &&
                c->primitive_count ==
                    SotnWeaponSecondary_RoleRows(c->state.secondary.family, c->kind & 15U)) ||
               (c->kind == 19 && c->primitive_count == 20) ||
               (c->kind == 37 && c->primitive_count == 18) ||
               (c->kind == 47 && c->primitive_count == 16) ||
               (c->kind == 30 && c->primitive_count == 6) ||
               (c->kind == 32 && c->primitive_count == SOTN_STONE_DISSOLVE_ROWS) ||
               (c->kind == 65 && c->primitive_count == SOTN_TELEPORT_ROWS) ||
               (c->kind == 38 && c->primitive_count == SOTN_PLAYER_CIRCLE_ROWS) ||
               (c->kind == 33 && c->primitive_count == 24) ||
               (c->kind == 63 && c->primitive_count == 9) ||
               (c->neutron && c->primitive_count == 1) ||
               (c->thrown && (c->kind & 15U) == 0 && c->primitive_count == 1) ||
               (c->star && (c->kind & 15U) == 3 && c->primitive_count == 32) ||
               (c->karma && (c->kind & 15U) == 0 && c->primitive_count == SOTN_KARMA_ROWS) ||
               (c->pentagram && c->primitive_count == SOTN_PENTAGRAM_ROWS))))
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
    if (food.lease.count &&
        !PlanRows(s, &used, food.lease.rows, food.lease.count, 0, r->wolf_bound))
        return false;
    if (!apply)
        return true;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (BatPrimitiveKind(s->primitives[i].ability_kind) ||
            (food.changed && s->primitives[i].ability_kind == SOTN_FOOD_TEXT_TAG) ||
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
    if (food.changed)
        ActorFoodTextPlanApply(s, &food);
    for (Uint16 i = 0; i < SOTN_SPELL_TARGETS; ++i)
        if (r->target_marks[i / 64U] & ((Uint64)1 << (i % 64U)))
            s->slots[i].state.flags |= 0x80000U;
    return true;
}
bool SotnActors_CommitBatPrimitives(SotnActors *s, const SotnBatRuntime *r)
{
    return CommitBatPrimitives(s, r, true);
}
static bool MatchPlayerRows(const SotnActors *s, SotnPrimitiveSlots *claimed, const Uint16 *rows,
                            Uint16 count, Uint16 kind, Uint32 generation)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint16 row = rows[i];
        if (!generation || row >= SOTN_PRIMITIVE_SLOTS ||
            (claimed->occupied[row / 64U] & ((Uint64)1 << (row % 64U))) ||
            s->primitives[row].owner != generation || s->primitives[row].ability_kind != kind)
            return SDL_SetError("Player cleanup lost an actual row's identity/generation");
        claimed->occupied[row / 64U] |= (Uint64)1 << (row % 64U);
    }
    return true;
}
bool SotnActors_ClearPlayerRuntime(SotnActors *s, SotnBatRuntime *r)
{
    if (!CommitBatPrimitives(s, r, false))
        return false;
    if (r->target_marks[0] || r->target_marks[1])
        return SDL_SetError("Player cleanup requires acknowledgement of actual stage target marks");
    SotnPrimitiveSlots claimed = {0};
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!MatchPlayerRows(s, &claimed, r->children[i].primitives, r->children[i].primitive_count,
                             r->children[i].kind, r->children[i].generation))
            return false;
    const SotnWolfRuntime *w = &r->wolf;
    if ((r->echo_generation &&
         !MatchPlayerRows(s, &claimed, r->echo_primitives, SOTN_BAT_ECHO_PRIMITIVES, 52,
                          r->echo_generation)) ||
        (w->root_reserved &&
         !MatchPlayerRows(s, &claimed, w->root_rows, 6, 56, w->children[0].generation)) ||
        (w->environment_reserved &&
         !MatchPlayerRows(s, &claimed, w->environment_rows, 2, SOTN_WOLF_ENVIRONMENT_TAG,
                          w->environment_generation)) ||
        (r->common_generation &&
         !MatchPlayerRows(s, &claimed, r->afterimage_rows, 6, SOTN_PLAYER_AFTERIMAGE_TAG,
                          r->common_generation)))
        return false;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if ((BatPrimitiveKind(s->primitives[i].ability_kind) ||
             (r->wolf_bound && WolfPrimitiveKind(s->primitives[i].ability_kind))) &&
            !(claimed.occupied[i / 64U] & ((Uint64)1 << (i % 64U))))
            return SDL_SetError("Player cleanup found an actual row missing from its owner");
    SotnBatRuntime *next = SDL_malloc(sizeof(*next));
    if (!next)
        return SDL_OutOfMemory();
    *next = *r;
    SDL_zero(next->factories);
    SDL_zero(next->children);
    SDL_zero(next->wolf.children);
    SDL_zero(next->wolf.body.parts);
    next->wolf.generation = 0;
    next->wolf.root_reserved = false;
    SDL_zero(next->wolf.root_rows);
    next->echo_generation = 0;
    SDL_zero(next->echo);
    SDL_zero(next->echo_primitives);
    next->wing_mask = next->wolf_mask = 0;
    next->fire_mask = next->spirit_mask = next->hellfire_mask = next->sword_mask = next->axe_mask =
        0;
    next->neutron_mask = next->sire_mask = next->pentagram_mask = next->magic_mask =
        next->thrown_mask = next->star_mask = next->karma_mask = next->secondary_mask = 0;
    next->magic_clear_shield = false;
    next->hand_attack = false;
    next->hand_shield = 0;
    // These are consumed callback notifications, not original retained global counters.
    SDL_zero(next->medicine);
    SDL_zero(next->wolf_events);
    SDL_zero(next->wolf_attacks);
    SDL_zero(next->captures);
    next->capture_count = next->sound_count = next->wolf_shake_count = 0;
    next->use_ticks = next->spell_use_ticks = 0;
    next->soul_attack = next->echo_attack = next->hellfire_upload = false;
    next->dissolve_frame_load = next->afterimage_reset = next->freeze_changed = false;
    const bool ok = CommitBatPrimitives(s, next, true);
    if (ok)
        *r = *next;
    SDL_free(next);
    return ok;
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
