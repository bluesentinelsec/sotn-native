// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_021.c pickup text and st/st_update.h timer/5087C.c room clear,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "actors_internal.h"
#include <sotn/food_popup.h>
bool ActorFoodTextValid(const SotnActors *s)
{
    if (!s || s->food_text.count > SOTN_FOOD_POPUP_ROWS ||
        (s->food_text.count != 0) != (s->food_text.generation != 0) ||
        (s->food_text.count != 0) != (s->food_text.timer != 0))
        return SDL_SetError("Food text lost its actual shared row lifetime");
    SotnPrimitiveSlots used = {0};
    for (Uint16 i = 0; i < s->food_text.count; ++i)
    {
        const Uint16 row = s->food_text.rows[i];
        if (row >= SOTN_PRIMITIVE_SLOTS ||
            (used.occupied[row / 64U] & ((Uint64)1 << (row % 64U))) ||
            s->primitives[row].ability_kind != SOTN_FOOD_TEXT_TAG ||
            s->primitives[row].owner != s->food_text.generation)
            return SDL_SetError("Food text lost an actual primitive row or generation");
        used.occupied[row / 64U] |= (Uint64)1 << (row % 64U);
    }
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].ability_kind == SOTN_FOOD_TEXT_TAG &&
            !(used.occupied[i / 64U] & ((Uint64)1 << (i % 64U))))
            return SDL_SetError("Food text has an actual row outside its global lease");
    return true;
}
bool ActorFoodTextClear(SotnActors *s)
{
    if (!ActorFoodTextValid(s))
        return false;
    for (Uint16 i = 0; i < s->food_text.count; ++i)
        s->primitives[s->food_text.rows[i]] = (ActorPrimitive){0};
    SDL_zero(s->food_text);
    return true;
}
bool ActorFoodTextTick(SotnActors *s)
{
    if (!ActorFoodTextValid(s))
        return false;
    if (s->food_text.timer == 1)
        return ActorFoodTextClear(s);
    if (s->food_text.timer)
        --s->food_text.timer;
    return true;
}
bool SotnActors_ShowFoodText(SotnActors *s, Uint16 item)
{
    if (!s || !s->entered)
        return SDL_SetError("Food text requires its actual stage and private pickup name");
    SotnFoodTextPlan plan = {0};
    SotnPrimitiveSlots slots = {0};
    if (!SotnActors_GetFoodTextPlan(s, &plan) || !SotnActors_PrimitiveSlots(s, &slots) ||
        !SotnFoodTextPlan_Replace(&plan, s->dra, item, &slots) || !ActorFoodTextPlanCheck(s, &plan))
        return false;
    ActorFoodTextPlanApply(s, &plan);
    return true;
}
bool SotnActors_GetFoodTextPlan(const SotnActors *s, SotnFoodTextPlan *out)
{
    if (!s || !out || !ActorFoodTextValid(s))
        return SDL_SetError("Food text snapshot requires its actual shared owner");
    SotnFoodTextPlan plan = {.owner = s,
                             .lease = s->food_text,
                             .previous = s->food_text,
                             .serial = s->effect_serial,
                             .previous_serial = s->effect_serial};
    plan.popup.count = s->food_text.count;
    for (Uint16 i = 0; i < plan.popup.count; ++i)
        plan.popup.rows[i] = s->primitives[s->food_text.rows[i]].state;
    *out = plan;
    return true;
}
bool ActorFoodTextPlanCheck(const SotnActors *s, const SotnFoodTextPlan *p)
{
    if (!s || !p || !ActorFoodTextValid(s))
        return false;
    if (!p->owner)
        return !s->food_text.count && !p->lease.count && !p->changed;
    if (p->owner != s || p->previous_serial != s->effect_serial ||
        p->previous.generation != s->food_text.generation ||
        p->previous.count != s->food_text.count || p->previous.timer != s->food_text.timer ||
        p->lease.count > SOTN_FOOD_POPUP_ROWS || p->popup.count != p->lease.count ||
        (p->lease.count != 0) != (p->lease.generation != 0) ||
        (p->lease.count != 0) != (p->lease.timer != 0))
        return SDL_SetError("Food text transaction lost its expected actual stage snapshot");
    for (Uint16 i = 0; i < p->previous.count; ++i)
        if (p->previous.rows[i] != s->food_text.rows[i])
            return SDL_SetError("Food text transaction replaced a different departing lease");
    return true;
}
void ActorFoodTextPlanApply(SotnActors *s, const SotnFoodTextPlan *p)
{
    if (!p->owner)
        return;
    // Joint player commits may already have reused released old rows for later children.
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (s->primitives[i].ability_kind == SOTN_FOOD_TEXT_TAG)
            s->primitives[i] = (ActorPrimitive){0};
    s->food_text = p->lease;
    s->effect_serial = p->serial;
    for (Uint16 i = 0; i < p->lease.count; ++i)
    {
        s->primitives[p->lease.rows[i]] = (ActorPrimitive){.state = p->popup.rows[i],
                                                           .owner = p->lease.generation,
                                                           .ability_kind = SOTN_FOOD_TEXT_TAG};
    }
}
Uint16 SotnActors_FoodTextRows(const SotnActors *s)
{
    return s ? s->food_text.count : 0;
}
bool SotnActors_GetFoodText(const SotnActors *s, Uint16 index, SotnActorPrimitive *out)
{
    if (!s || !out || !ActorFoodTextValid(s) || index >= s->food_text.count)
        return SDL_SetError("Food text read requires a current generation-owned row");
    *out = s->primitives[s->food_text.rows[index]].state;
    return true;
}
