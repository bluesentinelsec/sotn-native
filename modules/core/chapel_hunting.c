// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_hunting_girl.h at pinned sotn-decomp 7bc54d8157b6.
#include "actors_internal.h"
static void Transition(Actor *a, Uint8 next)
{
    a->hunt.attack_step = a->hunt.frames = 0;
    a->hunt.next_step = next;
    SetStep(a, 6);
}
static bool OwnedChildren(SotnActors *s, const Actor *a, bool retire)
{
    if ((Uint32)a->slot + 3U >= SOTN_STAGE_ACTOR_SLOTS ||
        (!retire && s->effect_serial > 0x7fffffffU - 3U))
        return SDL_SetError("Chapel Hunting Girl cannot reserve three owned blade children");
    for (Uint16 i = 1; i <= 3; ++i)
    {
        const Actor *child = &s->slots[a->slot + i];
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, a->slot + i, &placement, &placed))
            return false;
        if (placed || (child->state.generation &&
                       (child->state.id != 0x3e || child->enemy_state.owner_slot != a->slot ||
                        (retire && child->enemy_state.owner_generation != a->state.generation))))
            return SDL_SetError("Chapel Hunting Girl would overwrite an unrelated blade child");
    }
    if (retire)
        for (Uint16 i = 1; i <= 3; ++i)
            if (s->slots[a->slot + i].state.generation)
                s->slots[a->slot + i].remove = true;
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!ActorInitialize(s, a, 0xa94))
        return false;
    a->state.hitbox_width = a->state.hitbox_height = 10;
    a->state.hitbox_state = 2;
    a->state.priority = 176;
    a->state.rotated = true;
    a->state.rotation = 0;
    if (!ActorPrimitives(s, a, 32))
    {
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    for (Uint16 i = 0; i < 32; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.page = 21,
                                        .palette = 0x231,
                                        .priority_set = true,
                                        .priority = (Uint16)(176U + (i != 0)),
                                        .textured = i == 0,
                                        .quad = i == 0,
                                        .line = i != 0,
                                        .blend = i != 0,
                                        .line_blend_mode = 1};
    }
    if (!OwnedChildren(s, a, false))
        return false;
    for (Uint16 i = 1; i <= 3; ++i)
    {
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x3e, i, (Uint32)a->slot + i, &child))
            return false;
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Pixel(a->state.y) * 65536;
        child->state.facing_left = false;
    }
    a->hunt.trail = 1;
    a->hunt.cycle_timer = 512;
    return true;
}
static bool Child(SotnActors *s, Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (a->state.parameters < 1 || a->state.parameters > 3)
        return SDL_SetError("Chapel Hunting Girl blade has an invalid position index");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0xa94))
            return false;
        a->state.hitbox_width = a->state.hitbox_height = 2;
        a->state.hitbox_state = 1;
        a->hunt.base_attack = a->attack;
        if (a->attack > 4999)
            return SDL_SetError("Chapel Hunting Girl blade power exceeds its doubled range");
    }
    a->attack = (Uint16)(a->hunt.base_attack * ((s->player.status & 5U) ? 2U : 1U));
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || s->slots[slot].state.id != 0x3d ||
        s->slots[slot].state.generation != a->enemy_state.owner_generation)
        a->remove = true;
    return true;
}
static bool Move(Actor *a, Sint32 gravity, Uint16 friction)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 vy = (Sint64)a->vy + gravity;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        vy < SDL_MIN_SINT32 || vy > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Hunting Girl exceeds bounded fixed-point motion");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    if (friction)
        a->vx -= a->vx / friction;
    a->vy = (Sint32)vy;
    return true;
}
static bool HideTrails(SotnActors *s, const Actor *a)
{
    for (Uint16 i = 1; i < 32; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    return true;
}
static bool Knockback(SotnActors *s, Actor *a, bool death)
{
    bool done = false;
    switch (a->substep)
    {
    case 0:
        if (death && !OwnedChildren(s, a, true))
            return false;
        if (death)
            a->state.hitbox_state = 0;
        a->hunt.attacking = false;
        if (!HideTrails(s, a))
            return false;
        a->hunt.random = (Uint8)(ActorRandom(s) & 1U);
        if (!ActorHuntingSpirit(s, a, (Uint16)(a->hunt.random + 3U), 0, &done))
            return false;
        a->vx = death ? 0 : (Side(a, s->player) ? 98304 : -98304);
        a->vy = -196608;
        if (!ActorPositionalSound(s, a, death ? 0x72c : 0x72b))
            return false;
        ++a->substep;
        SDL_FALLTHROUGH;
    case 1:
        if (!ActorHuntingSpirit(s, a, (Uint16)(a->hunt.random + 3U), 1, &done) ||
            !Move(a, 8192, death ? 0 : 32))
            return false;
        if (a->vy > (death ? 0 : 131072))
            ++a->substep;
        return true;
    case 2:
        if (!ActorHuntingSpirit(s, a, (Uint16)(a->hunt.random + 3U), 2, &done))
            return false;
        if (done)
        {
            if (death)
            {
                a->state.flags |= 0x80000000U;
                ++a->substep;
            }
            else
                Transition(a, 4);
        }
        return true;
    case 3:
        if (!death)
            return SDL_SetError("Chapel Hunting Girl hit reaction entered a death phase");
        if (!Move(a, 4096, 0))
            return false;
        if (!(s->service_ticks & 7U))
        {
            Sint32 x = 0, y = 0;
            Actor *flame = NULL;
            if (!ActorPositionalSound(s, a, 0x691) || !ActorHuntingTip(s, a, &x, &y))
                return false;
            // Allocation precedes this random draw in the original optional effect path.
            if (!ActorSpawn(s, a, 6, 0, 160, 192, &flame))
                return false;
            if (flame)
            {
                const Sint32 fraction = (Sint32)(ActorRandom(s) & 3U);
                flame->state.x =
                    Wrap16((Sint64)Pixel(a->state.x) + (x - Pixel(a->state.x)) * fraction / 4) *
                    65536;
                flame->state.y =
                    Wrap16((Sint64)Pixel(a->state.y) + (y - Pixel(a->state.y)) * fraction / 4) *
                    65536;
                flame->state.facing_left = false;
            }
        }
        return true;
    default:
        return SDL_SetError("Chapel Hunting Girl entered an invalid knockback phase");
    }
}
static bool Idle(SotnActors *s, Actor *a)
{
    bool done = false;
    if (a->substep == 0)
    {
        if (!ActorHuntingSpirit(s, a, 0, 0, &done))
            return false;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        if (!ActorHuntingSpirit(s, a, 0, 1, &done))
            return false;
        const Sint64 dy = (Sint64)Pixel(a->state.y) - Pixel(s->player.y);
        if (Distance(a, s->player) < 80 && dy > -48 && dy < 48)
            ++a->substep;
        return true;
    }
    if (a->substep != 2)
        return SDL_SetError("Chapel Hunting Girl entered an invalid idle fade");
    if (!ActorHuntingSpirit(s, a, 0, 2, &done))
        return false;
    if (done)
        Transition(a, 3);
    return true;
}
static bool Recover(SotnActors *s, Actor *a)
{
    bool done = false;
    if (a->substep == 0)
    {
        if (!ActorHuntingSpirit(s, a, 1, 0, &done))
            return false;
        a->hunt.attack_timer = 48;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        if (!ActorHuntingDrawAttack(s, a, 0x21c8, &done) || !ActorHuntingSpirit(s, a, 1, 1, &done))
            return false;
        if (!a->hunt.attack_timer)
            return SDL_SetError("Chapel Hunting Girl recovery lost its bounded attack wait");
        if (!--a->hunt.attack_timer)
            ++a->substep;
        if (Distance(a, s->player) < 64)
            Transition(a, 5);
        return true;
    }
    if (a->substep != 2)
        return SDL_SetError("Chapel Hunting Girl entered an invalid recovery fade");
    if (!ActorHuntingSpirit(s, a, 1, 2, &done))
        return false;
    if (done)
        Transition(a, 3);
    return true;
}
static bool Prepare(SotnActors *s, Actor *a, bool retreat)
{
    bool done = false;
    if (!a->substep)
    {
        a->hunt.attack_timer = retreat ? 16 : 64;
        ++a->substep;
    }
    if (!ActorHuntingDrawAttack(s, a, retreat ? 0x2198 : 0x2180, &done))
        return false;
    if (Distance(a, s->player) > 80)
    {
        if (!a->hunt.cycle_timer)
            return SDL_SetError("Chapel Hunting Girl lost its bounded approach cycle");
        --a->hunt.cycle_timer;
    }
    if (!a->hunt.cycle_timer)
    {
        a->hunt.cycle_timer = 512;
        Transition(a, 4);
    }
    if (retreat && Distance(a, s->player) > 96)
        Transition(a, 3);
    if ((Side(a, s->player) == 0) != a->state.facing_left)
        Transition(a, 5);
    if (!a->hunt.attack_timer)
    {
        if (Distance(a, s->player) < 80)
            Transition(a, 5);
    }
    else
    {
        --a->hunt.attack_timer;
        if (!retreat && Distance(a, s->player) < 56)
            Transition(a, 7);
    }
    return true;
}
static bool Attack(SotnActors *s, Actor *a)
{
    bool done = false;
    if (a->substep == 0)
    {
        if (!ActorHuntingSpirit(s, a, 2, 0, &done))
            return false;
        ++a->substep;
    }
    switch (a->substep)
    {
    case 1:
        if (!ActorHuntingSpirit(s, a, 2, 1, &done) || !ActorHuntingDrawAttack(s, a, 0x21b0, &done))
            return false;
        if (done)
            ++a->substep;
        return true;
    case 2:
        if (!ActorHuntingSpirit(s, a, 2, 2, &done))
            return false;
        if (done)
        {
            a->hunt.attacking = true;
            a->hunt.trail = 1;
            a->hunt.attack_step = a->hunt.frames = 0;
            if (!ActorPositionalSound(s, a, 0x72a))
                return false;
            ++a->substep;
        }
        return true;
    case 3: {
        SotnDataView pointer = {0};
        if (a->hunt.pattern >= 3 || !Slice(s->nz0, 0x2288U + a->hunt.pattern * 4U, 4, &pointer))
            return SDL_SetError("Chapel Hunting Girl selected an invalid attack pattern");
        const Uint32 address = Read32(pointer.bytes);
        if (address != 0x801821d8 && address != 0x80182218 && address != 0x80182250)
            return SDL_SetError(
                "Chapel Hunting Girl attack pointer is outside its private patterns");
        if (!ActorHuntingDrawAttack(s, a, address - 0x80180000U, &done))
            return false;
        if (done)
        {
            a->hunt.attacking = false;
            a->hunt.pattern = (Uint8)((a->hunt.pattern + 1U) % 3U);
            Transition(a, 3);
        }
        return true;
    }
    default:
        return SDL_SetError("Chapel Hunting Girl entered an invalid attack phase");
    }
}
bool ActorChapelHunting(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || (a->state.id != 0x3d && a->state.id != 0x3e))
        return SDL_SetError("Chapel Hunting Girl requires its original stage identity");
    if (a->state.id == 0x3e)
        return Child(s, a);
    if ((a->hit_flags & 3U) && a->state.step != 8)
    {
        a->hunt.attacking = false;
        SetStep(a, 8);
    }
    if ((a->state.flags & 0x100U) && a->state.step < 9)
        SetStep(a, 9);
    bool ok = true;
    switch (a->state.step)
    {
    case 0:
        ok = Initialize(s, a);
        break;
    case 1: {
        Uint16 floor = 0;
        if (!ActorGround(s, a, 0x212c, &floor))
            return false;
        if (floor & 1U)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->hunt.reference_y = Pixel(a->state.y);
            Transition(a, 2);
        }
        break;
    }
    case 2:
        ok = Idle(s, a);
        break;
    case 3:
        ok = Prepare(s, a, false);
        break;
    case 4:
        ok = Recover(s, a);
        break;
    case 5:
        ok = Attack(s, a);
        break;
    case 6:
        if (a->substep == 0)
        {
            const Sint32 target =
                (s->player.status & 1U) ? Pixel(s->player.y) : a->hunt.reference_y;
            const Sint32 delta = Pixel(a->state.y) - target;
            if (!delta)
                ++a->substep;
            else
                ShiftY(a, delta > 0 ? -1 : 1);
        }
        else if (a->substep == 1)
        {
            const bool facing = Side(a, s->player) == 0;
            if (a->state.facing_left != facing)
                a->state.rotation = (Sint16)Wrap16(-(Sint64)a->state.rotation);
            a->state.facing_left = facing;
            SetStep(a, a->hunt.next_step);
        }
        else
            return SDL_SetError("Chapel Hunting Girl entered an invalid transition phase");
        break;
    case 7:
        ok = Prepare(s, a, true);
        break;
    case 8:
        ok = Knockback(s, a, false);
        break;
    case 9:
        ok = Knockback(s, a, true);
        break;
    default:
        return SDL_SetError("Chapel Hunting Girl entered an invalid original state");
    }
    if (!ok || a->remove)
        return ok;
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, 0x211c, &animated))
        return false;
    if (!(a->state.flags & 0x100U))
    {
        Sint32 x = 0, y = 0;
        if (!ActorHuntingTip(s, a, &x, &y))
            return false;
        for (Uint16 i = 1; i <= 3; ++i)
        {
            Actor *child = &s->slots[a->slot + i];
            if (child->state.id != 0x3e || child->enemy_state.owner_slot != a->slot ||
                child->enemy_state.owner_generation != a->state.generation)
                return SDL_SetError("Chapel Hunting Girl lost an owned blade child");
            child->state.x =
                Wrap16((Sint64)Pixel(a->state.x) + (x - Pixel(a->state.x)) * i / 4) * 65536;
            child->state.y =
                Wrap16((Sint64)Pixel(a->state.y) + (y - Pixel(a->state.y)) * i / 4) * 65536;
        }
    }
    return ActorHuntingTrails(s, a);
}
