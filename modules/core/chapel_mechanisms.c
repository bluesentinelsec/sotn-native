// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_dai_no2_shortcut.h and st/dai/e_gargoyle.c at the pinned
// sotn-decomp revision. Initialization/sensor/artwork tables remain private.
#include "actors_internal.h"
static void ShiftHorizontal(Actor *a, Sint32 pixels)
{
    a->state.x = (Sint32)((Sint64)Wrap16((Sint64)Pixel(a->state.x) + pixels) * 65536 +
                          ((Uint32)a->state.x & 65535U));
}
static bool Child(SotnActors *s, Actor *a, Uint16 id, Uint16 parameters, Actor **out)
{
    const Uint32 slot = (Uint32)a->slot + 1U;
    if (!a->state.generation || slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Chapel mechanism lacks its adjacent child reservation");
    Actor *old = &s->slots[slot];
    if (old->state.generation)
    {
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, slot, &placement, &placed))
            return false;
        if (placed || old->state.id != id || old->enemy_state.owner_slot != a->slot)
            return SDL_SetError("Chapel mechanism would overwrite an unrelated child slot");
        ActorPrimitivesClear(s, old);
        *old = (Actor){0};
    }
    if (!ActorSpawnAt(s, a, id, parameters, slot, out))
        return false;
    (*out)->state.x = Pixel(a->state.x) * 65536;
    (*out)->state.y = Pixel(a->state.y) * 65536;
    (*out)->state.facing_left = false;
    a->chapel.child_generation = (*out)->state.generation;
    return true;
}
static bool Block(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x9b0))
            return false;
        a->state.frame = 1;
        if (s->world->flags[SOTN_CHAPEL_SHORTCUT])
        {
            ShiftHorizontal(a, 32);
            ShiftY(a, 32);
            a->state.step = 16;
        }
        else
        {
            Actor *statue = NULL;
            if (!Child(s, a, 0x1a, 0, &statue))
                return false;
            ShiftHorizontal(statue, 16);
            ShiftY(statue, 16);
        }
    }
    if (a->state.step == 1)
    {
        if ((Uint32)a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Chapel shortcut has no adjacent statue");
        const Actor *statue = &s->slots[a->slot + 1U];
        if (!statue->state.id)
        {
            s->world->flags[SOTN_CHAPEL_SHORTCUT] = 1;
            ++a->state.step;
        }
        else if (statue->state.id != 0x1a ||
                 statue->state.generation != a->chapel.child_generation ||
                 statue->enemy_state.owner_slot != a->slot ||
                 statue->enemy_state.owner_generation != a->state.generation)
            return SDL_SetError("Chapel shortcut lost its owned original statue");
    }
    else if (a->state.step == 2)
    {
        Uint16 collision = 0;
        a->vx = 16384;
        if (!ActorWalk(s, a, 0xca4, &collision) ||
            (!(s->service_ticks & 7U) && !ActorSound(s, 0x644)))
            return false;
        if (!(collision & (SOTN_EFFECT_HURT | SOTN_EFFECT_SOLID_FROM_ABOVE)))
            ++a->state.step;
    }
    else if (a->state.step != 3 && a->state.step != 16)
        return SDL_SetError("Chapel shortcut entered an invalid original state");
    Uint16 collision = 0;
    a->state.hitbox_state = 1;
    a->state.hitbox_x = 8;
    a->state.hitbox_y = -68;
    a->state.hitbox_width = 8;
    a->state.hitbox_height = 6;
    if (!ActorObjectCollision(s, a, 8, 6, 4, &collision) ||
        ((collision & 4U) && !ActorDisplace(s, 0, 65536, 0)))
        return false;
    a->state.hitbox_x = -16;
    a->state.hitbox_y = -60;
    a->state.hitbox_width = 16;
    a->state.hitbox_height = 48;
    return ActorObjectCollision(s, a, 16, 48, 7, &collision);
}
static bool Statue(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x9b0))
            return false;
        a->state.frame = 2;
        a->state.hitbox_state = Pixel(s->player.x) < 128 ? 0 : 2;
        a->state.hit_points = 4;
        a->state.hitbox_x = -8;
        a->state.hitbox_y = -32;
        a->state.hitbox_width = 8;
        a->state.hitbox_height = 16;
    }
    if (a->state.flags & 0x100U)
    {
        Actor source = {.state = {.x = Wrap16((Sint64)Pixel(a->state.x) - 8) * 65536,
                                  .y = Wrap16((Sint64)Pixel(a->state.y) - 8) * 65536}};
        if (!ActorExplosionInPool(s, &source, 1, 160, 192, 0) || !ActorSound(s, 0x655))
            return false;
        a->remove = true;
    }
    return true;
}
static bool Gargoyle(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 2)
        return SDL_SetError("Chapel gargoyle has an invalid original variant");
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x9b0))
            return false;
        a->state.priority = 92;
        a->state.frame = (Uint16)(12U + a->state.parameters);
        if (a->state.parameters == 2)
            return true;
        Actor *block = NULL;
        if (!Child(s, a, 0x21, 2, &block))
            return false;
        block->state.y = 160 * 65536;
        if (a->state.parameters)
            ShiftHorizontal(block, 16);
        return true;
    }
    case 1:
    case 16:
        return true;
    case 2: {
        const Sint64 y = (Sint64)a->state.y + 73728;
        if (y <= 70 * 65536)
        {
            a->state.y = (Sint32)y;
            return true;
        }
        const Uint32 tile = a->state.parameters ? 222U : 193U;
        if (s->map.width != 32 || tile + 96U >= (Uint64)s->map.width * s->map.height ||
            s->map.types.size <= 0x4b3 || !s->mutable_tiles)
            return SDL_SetError("Chapel gargoyle tile opening exceeds its original room");
        for (Uint32 i = 0; i < 4; ++i)
            if (!ActorTile(s, tile + i * 32U, 0x4b3))
                return false;
        a->state.y = 70 * 65536;
        a->state.step = 16;
        return true;
    }
    case 3: {
        const Sint64 y = (Sint64)a->state.y - 73728;
        a->state.y = (Sint32)SDL_max(y, 129LL * 65536);
        if (y < 129LL * 65536)
            a->state.step = 16;
        return true;
    }
    default:
        return SDL_SetError("Chapel gargoyle entered an invalid original state");
    }
}
bool ActorChapelMechanism(SotnActors *s, Actor *a)
{
    if (!s || !a || !s->world || s->stage_id != 6)
        return SDL_SetError("Chapel mechanism requires initialized original stage/world owners");
    if (a->state.id == 0x19)
        return Block(s, a);
    if (a->state.id == 0x1a)
        return Statue(s, a);
    if (a->state.id == 0x21)
        return Gargoyle(s, a);
    return SDL_SetError("Chapel mechanism identity is invalid");
}
