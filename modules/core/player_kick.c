// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntityDiveKickAttack and 5D5BC.c:GetEquipProperties,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Definitions stay private.
#include "player_internal.h"
#include <sotn/combat.h>

bool SotnPlayer_KickTick(SotnPlayer *p)
{
    if (p->step != SOTN_PLAYER_JUMP || p->substep != 0x70)
    {
        p->kick = (SotnPlayerKick){0};
        p->kick_hit = 0;
        return true;
    }
    p->movement_flags &= ~0x80U;
    if (p->kick.generation == 0)
    {
        SotnEquipment empty = {0};
        if (!SotnEquipment_Read(p->dra, 0, &empty))
            return false;
        const Uint32 value =
            p->attributes.total[3] * 2U + (SotnCombat_Random(&p->damage_random) & 15U);
        Sint32 root = 0;
        while ((Uint32)((root + 1) * (root + 1)) <= value)
            ++root;
        if (++p->kick_counter == 0)
            ++p->kick_counter;
        p->kick = (SotnPlayerKick){
            .generation = p->kick_counter,
            .attack = {.offset_x = 9,
                       .offset_y = 21,
                       .width = 4,
                       .height = 5,
                       .element = empty.element,
                       .hit_type = empty.hit_type,
                       .hit_effect = empty.hit_effect,
                       .stun_frames = empty.stun,
                       .invincibility_frames = empty.invincibility,
                       .critical_rate = (Uint8)SDL_clamp(empty.critical_rate + root - 5, 0, 255),
                       .power = (Uint16)(p->attributes.unarmed_attack >> (p->poison_ticks ? 1 : 0)),
                       .active = true}};
    }
    else if (p->kick_hit == 1)
        p->movement_flags |= 0x80;
    p->kick_hit = 0;
    return true;
}

bool SotnPlayer_KickHit(SotnPlayer *p, Uint32 generation, Uint16 flags)
{
    if (p == NULL || generation == 0 || p->kick.generation != generation || flags < 1 || flags > 3)
        return SDL_SetError("Dive-kick feedback requires its current child and original hit flags");
    p->kick_hit = flags;
    return true;
}
