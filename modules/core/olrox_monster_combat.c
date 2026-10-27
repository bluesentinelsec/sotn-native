// SPDX-License-Identifier: AGPL-3.0-or-later
// BO0 313A8.c shared head/nextPart ring and st/collision.h at pinned
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Definitions remain private.
#include "olrox_monster_internal.h"
Actor *ActorOlroxMonsterOwner(SotnActors *s, const Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation ||
        a->slot >= SOTN_STAGE_ACTOR_SLOTS || &s->slots[a->slot] != a)
        return NULL;
    if (a->state.id == 0x33)
        return &s->slots[a->slot];
    if (!a->enemy_state.owner_generation || a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *body = &s->slots[a->enemy_state.owner_slot];
    if (body->state.id != 0x33 || body->state.generation != a->enemy_state.owner_generation ||
        a->slot <= body->slot || a->slot - body->slot > 9 ||
        OlroxChild(s, body, (Uint16)(a->slot - body->slot)) != a)
        return NULL;
    const Uint16 role = (Uint16)(a->slot - body->slot);
    const Uint16 id = role <= 2   ? 0x34
                      : role == 3 ? 0x35
                      : role == 4 ? 0x36
                      : role <= 6 ? 0x3b
                      : role == 7 ? 0x37
                                  : 0x38;
    return a->state.id == id ? body : NULL;
}
void ActorOlroxMonsterHit(SotnActors *s, Actor *recipient, Actor *hit, Uint32 channel,
                          Uint8 cooldown)
{
    if (!s || !recipient || !hit || s->stage_id != 30 || recipient->state.id != 0x35 ||
        channel > ACTOR_KICK_CHANNEL)
        return;
    Actor *body = ActorOlroxMonsterOwner(s, recipient);
    if (!body || ActorOlroxMonsterOwner(s, hit) != body || hit->slot > body->slot + 6U)
        return;
    if (hit->state.hit_points == 0x7fff)
        hit->hit_flags = 0x10;
    for (Uint16 i = 0; i <= 6; ++i)
    {
        Actor *part = i ? OlroxChild(s, body, i) : body;
        if (!part)
            continue;
        if (hit->state.hit_points != 0x7fff || part->state.hit_points == 0x7fff)
            part->cooldown[channel] =
                (Uint8)SDL_min(255U, (Uint32)cooldown + (part->slot > recipient->slot ? 1U : 0U));
        if (hit->state.hit_points != 0x7fff && !(hit->state.flags & 0x400000U))
            part->stun = recipient->stun;
        if (recipient->state.flags & 0x100U)
        {
            part->state.flags |= 0x10c100U;
            part->state.flags &= ~0x20000000U;
        }
    }
}
