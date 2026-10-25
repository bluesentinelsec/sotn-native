// SPDX-License-Identifier: AGPL-3.0-or-later
// Stone Rose seed/spawn/effect routines from st/no0/e_stone_rose.c, pinned reference.
// Spawn recipes, animation and graphics remain private runtime data.
#include "actors_internal.h"
bool ActorRoseSpawn(SotnActors *s, const Actor *a, Uint16 parameter)
{
    Actor *seed = NULL;
    if (!ActorSpawn(s, a, 0x41, parameter, 96, 128, &seed))
        return false;
    if (seed != NULL)
    {
        seed->enemy_state.owner_slot = a->slot;
        seed->enemy_state.owner_generation = a->state.generation;
        seed->state.flags = a->state.flags & (0x2000 | 0x100);
    }
    return true;
}
static bool Spawner(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        SotnDataView recipe = {0};
        if (a->state.parameters >= 3 ||
            !Slice(s->nz0, 0x2268 + a->state.parameters * 8U, 8, &recipe))
            return SDL_SetError("Stone Rose spawn recipe index is invalid");
        const Uint16 id = Read16(recipe.bytes), count = Read16(recipe.bytes + 2),
                     mode = Read16(recipe.bytes + 4), delay = Read16(recipe.bytes + 6);
        a->rose.spawn_id = id & 255;
        a->rose.parameter = id & 0xff00;
        a->rose.total = count & 255;
        a->rose.drop_if_full = (count >> 8) == 1;
        a->rose.per_tick = mode & 127;
        a->rose.mode = mode >> 8;
        a->rose.increment = (mode & 128) != 0;
        a->rose.pool = delay >> 8;
        a->rose.interval = delay & 127;
        a->rose.delay = (delay & 128) ? a->rose.interval : 0;
        if (a->rose.spawn_id < 0x3e || a->rose.spawn_id > 0x40 || !a->rose.total ||
            !a->rose.per_tick || a->rose.per_tick > 32 || a->rose.mode > 2 || a->rose.pool > 1 ||
            (count >> 8) > 1)
            return SDL_SetError("Stone Rose spawn recipe is outside its bounded actor families");
        if (a->rose.mode == 0)
            a->state.flags |= 0x08000000;
        ++a->state.step;
    }
    if (a->rose.mode == 2)
    {
        const Uint16 slot = a->enemy_state.owner_slot;
        if (slot >= SOTN_STAGE_ACTOR_SLOTS ||
            s->slots[slot].state.generation != a->enemy_state.owner_generation)
        {
            a->remove = true;
            return true;
        }
        a->state.x = s->slots[slot].state.x;
        a->state.y = s->slots[slot].state.y;
    }
    if (a->rose.delay && --a->rose.delay)
        return true;
    for (Uint16 i = 0; i < a->rose.per_tick; ++i)
    {
        Actor *child = NULL;
        const Uint16 parameter =
            (Uint16)(a->rose.parameter + (a->rose.increment ? a->rose.spawned : i));
        if (!ActorSpawn(s, a, a->rose.spawn_id, parameter, a->rose.pool ? 160 : 96,
                        a->rose.pool ? 192 : 128, &child))
            return false;
        if (child == NULL)
        {
            if (a->rose.drop_if_full)
                a->remove = true;
            return true;
        }
        child->enemy_state.owner_slot = a->enemy_state.owner_slot;
        child->enemy_state.owner_generation = a->enemy_state.owner_generation;
        if (++a->rose.spawned == a->rose.total)
        {
            a->remove = true;
            return true;
        }
    }
    a->rose.delay = a->rose.interval;
    return true;
}
static bool Seed(SotnActors *s, Actor *a, bool floating)
{
    if (a->state.flags & 0x100)
    {
        if (!ActorSound(s, 0x691))
            return false;
        return ActorReplaceExplosion(s, a, 0);
    }
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, floating ? 0xb0c : 0xb00))
            return false;
        if (!floating)
        {
            a->state.frame = 44;
            a->vx = a->state.facing_left ? -114688 : 114688;
            a->state.x += (a->state.facing_left ? -40 : 40) * 65536;
            a->rose.timer = 2;
        }
        else
        {
            a->state.priority = 0xa4;
            a->animation_stream = 0x2318;
            a->rose.timer = SotnCombat_Random(&s->combat_random);
            const Sint32 angle = (SotnCombat_Random(&s->combat_random) & 511) + 1792;
            a->vx = ActorSin(angle + 1024) * 16 * (a->state.facing_left ? 1 : -1);
            a->vy = ActorSin(angle) * 16;
            a->state.x += (a->state.facing_left ? -12 : 12) * 65536;
            a->state.x += ((Sint32)(SotnCombat_Random(&s->combat_random) & 7) - 4) * 65536;
            ShiftY(a, (Sint32)(SotnCombat_Random(&s->combat_random) & 7) - 4);
        }
    }
    else if (floating)
    {
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
            return ActorReplaceExplosion(s, a, 0);
        bool ended = false;
        if (!ActorWideAnimation(s, a, a->animation_stream, &ended))
            return false;
        const Sint32 sway = ActorSin(a->rose.timer) * 4;
        a->rose.timer = (Uint16)(a->rose.timer + 96);
        a->state.x += a->vx;
        a->state.y += a->vy + sway;
    }
    else
    {
        a->state.x += a->vx;
        a->state.y += a->vy;
        SotnCollider floor = {0}, wall = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + 4, &floor))
            return false;
        if (floor.effects & SOTN_EFFECT_SOLID)
        {
            if (a->rose.timer == 0)
                return ActorReplaceExplosion(s, a, 0);
            ShiftY(a, floor.top);
            a->vx = ActorShift(a->vx, 1);
            a->vy = -2 * 65536;
            --a->rose.timer;
        }
        if (!ActorQuery(s, Pixel(a->state.x) + (a->vx < 0 ? -6 : 8), Pixel(a->state.y), &wall))
            return false;
        if (wall.effects & SOTN_EFFECT_SOLID)
        {
            a->vx = -a->vx;
            a->vy = 0;
        }
    }
    if (!floating)
    {
        a->vy += 6144;
        Sint16 angle = 0;
        if (a->vy == SDL_MIN_SINT32 || !SotnMath_Atan2Vector(s->executable, -a->vy, a->vx, &angle))
            return false;
        a->state.rotated = true;
        a->state.rotation = (Sint16)((angle + 1536) & 4095);
    }
    return true;
}
bool ActorRoseSeed(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x41)
        return Spawner(s, a);
    if (a->state.id == 0x3e || a->state.id == 0x3f)
        return Seed(s, a, a->state.id == 0x3f);
    if (a->state.id != 0x40)
        return SDL_SetError("Stone Rose effect dispatcher received an unrelated actor");
    if (a->state.step == 0)
    {
        a->state.parameters = 1;
        if (!ActorExplosionTick(s, a))
            return false;
        a->state.priority = 0xb4;
        const Sint32 angle = SotnCombat_Random(&s->combat_random) & 2047;
        a->vx = ActorSin(angle + 1024) * 64;
        a->vy = -ActorSin(angle) * 64;
        return true;
    }
    a->state.x += a->vx;
    return ActorExplosionTick(s, a);
}
