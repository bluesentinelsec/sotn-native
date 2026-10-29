// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_particles.h:EntityEnemyBlood and collision.h blood contacts,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Stage initialization stays private.
#include "actors_internal.h"

static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Screen(Sint32 position, Sint32 camera)
{
    return Fixed((Uint32)position - (Uint32)camera * 65536U);
}
bool ActorEnemyBlood(SotnActors *s, Actor *a, SotnPlayerState player)
{
    if (!s || !a || a->state.id != 0xd || a->state.parameters > 1 || a->state.step > 1 ||
        (a->state.step &&
         (a->state.primitive_count != SOTN_ENEMY_BLOOD_PARTICLES || a->enemy_blood.phase != 1)))
        return SDL_SetError("Enemy blood requires its actual common actor and twelve owned rows");
    SotnCharacterState character = {0};
    if (s->character && !SotnCharacter_Get(s->character, &character, NULL))
        return false;
    if (a->state.step)
        for (Uint16 i = 0; i < SOTN_ENEMY_BLOOD_PARTICLES; ++i)
            if (!ActorPrimitiveGet(s, a, i))
                return false;
    bool newborn = !a->state.step;
    if (newborn)
    {
        if (!ActorPrimitivesContiguous(s, a, SOTN_ENEMY_BLOOD_PARTICLES))
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, ActorTable(s, 0xbd4, 0xab8, 0xa48)))
        {
            ActorPrimitivesClear(s, a);
            a->state.primitive_count = 0;
            return false;
        }
        a->state.animation_set = 0;
        a->state.flags |= 0x800000U;
        a->state.priority = 192;
    }
    const Sint32 x = Screen(a->state.x, s->camera_x), y = Screen(a->state.y, s->camera_y);
    SotnEnemyBlood blood = a->enemy_blood;
    if (!newborn)
    {
        const Sint32 dx = Wrap16(Pixel(x) - Pixel(blood.x)), dy = Wrap16(Pixel(y) - Pixel(blood.y));
        if (!SotnEnemyBlood_Translate(&blood, dx, dy))
            return false;
        blood.x = x;
        blood.y = y;
    }
    const bool stone =
        character.equipment[SOTN_EQUIP_RING1] == 60 || character.equipment[SOTN_EQUIP_RING2] == 60;
    SotnEnemyBloodEvents events = {0};
    if (!SotnEnemyBlood_Tick(
            &blood,
            (SotnEnemyBloodInput){.executable = s->executable,
                                  .x = x,
                                  .y = y,
                                  .facing_left = a->state.parameters != 0,
                                  .allocated = true,
                                  .absorbing = (player.status & 0x2000000U) != 0,
                                  .hit = a->hit_flags != 0,
                                  .heal_pending = player.healing.pending || s->combat.blood_heal,
                                  .bloodstone = stone},
            &s->random, &events))
    {
        if (newborn)
        {
            ActorPrimitivesClear(s, a);
            a->state.primitive_count = 0;
        }
        return false;
    }
    a->enemy_blood = blood;
    a->remove = blood.removed;
    if (a->remove)
        return true;
    if (events.contact)
    {
        if (s->combat.blood_contacts >= 32)
            return SDL_SetError("Enemy blood exceeded its original contact actor pool");
        ++s->combat.blood_contacts;
        if (!s->combat.blood_heal)
            s->combat.blood_heal = events.heal;
    }
    a->state.x = Fixed((Uint32)blood.x + (Uint32)s->camera_x * 65536U);
    a->state.y = Fixed((Uint32)blood.y + (Uint32)s->camera_y * 65536U);
    a->vx = blood.velocity_x;
    a->vy = 0;
    a->state.hitbox_width = blood.hitbox_width;
    a->state.hitbox_height = blood.hitbox_height;
    a->state.hitbox_state = blood.hitbox ? 1 : 0;
    for (Uint16 i = 0; i < SOTN_ENEMY_BLOOD_PARTICLES; ++i)
    {
        const SotnEnemyBloodParticle *tile = &blood.particles[i];
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){
            .x = Fixed(((Uint32)(Sint32)tile->x + (Uint32)s->camera_x) * 65536U),
            .y = Fixed(((Uint32)(Sint32)tile->y + (Uint32)s->camera_y) * 65536U),
            .width = tile->width,
            .height = tile->height,
            .red = 128,
            .blue = 16,
            .priority_set = true,
            .priority = 192,
            .active = tile->width && tile->height};
    }
    return true;
}
