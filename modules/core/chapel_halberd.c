// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_bone_halberd.c at sotn-decomp 7bc54d8157b6.
// All original animation, motion, sensor and hitbox tables remain private inputs.
#include "actors_internal.h"
static bool Interval(SotnActors *s, Actor *a)
{
    SotnDataView interval = {0};
    a->halberd.interval = (Uint16)((a->halberd.interval + 1U) & 3U);
    if (!Slice(s->nz0, 0x1e54U + (a->state.parameters & 1U) * 4U + a->halberd.interval, 1,
               &interval))
        return false;
    a->halberd.timer = interval.bytes[0];
    return true;
}
static bool MoveDecision(SotnActors *s, Actor *a)
{
    Uint16 floor = 0, field = 0;
    if (!ActorWalk(s, a, 0x1e6c, &floor) || !ActorField(s, a, 0x1e74, 3, false, &field))
        return false;
    if (floor == 128 || (field & 2U))
    {
        SetStep(a, 5);
        return true;
    }
    if (!a->halberd.timer)
    {
        if (Distance(a, s->player) < 84 && a->state.facing_left != (Side(a, s->player) != 0))
            SetStep(a, 4);
    }
    else if (!--a->halberd.timer && !(ActorRandom(s) & 3U))
        SetStep(a, 7);
    return true;
}
static bool AttackSounds(SotnActors *s, const Actor *a)
{
    return (a->remaining != 0 || a->pose != 5 || ActorPositionalSound(s, a, 0x6ba)) &&
           (a->remaining != 0 || a->pose != 18 || ActorPositionalSound(s, a, 0x62e));
}
static bool Lunge(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    Uint16 collision = 0;
    switch (a->substep)
    {
    case 0:
        a->state.facing_left = Side(a, s->player) == 0;
        a->halberd.lunge_timer = 192;
        ++a->substep;
        return true;
    case 1:
        a->state.facing_left = Side(a, s->player) == 0;
        if (!a->halberd.lunge_timer)
            return SDL_SetError("Chapel Halberd lunge has no remaining timer");
        if (!ActorAnimate(s, a, 0x1d50, &animated) || !ActorWalk(s, a, 0x1e6c, &collision))
            return false;
        a->vx = a->state.facing_left ? 131072 : -131072;
        if ((collision & 128U) || !--a->halberd.lunge_timer)
        {
            a->substep = 3;
            a->pose = a->remaining = 0;
        }
        if (Distance(a, s->player) < 56)
        {
            a->halberd.lunge_timer = (Uint16)((ActorRandom(s) & 3U) + 1U);
            // Original SetSubStep clears the preceding temporary pose/timer writes.
            a->pose = a->remaining = 0;
            a->substep = 2;
        }
        return true;
    case 2:
        if (!a->halberd.lunge_timer)
            return SDL_SetError("Chapel Halberd repeated stab has no remaining count");
        if (!AttackSounds(s, a) || !ActorAnimate(s, a, 0x1d70, &animated))
            return false;
        if (!animated)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->pose = 2;
            a->remaining = 0;
            if (!--a->halberd.lunge_timer)
            {
                a->substep = 3;
                a->pose = a->remaining = 0;
            }
        }
        return true;
    case 3:
        SetStep(a, 3);
        return Interval(s, a);
    default:
        return SDL_SetError("Chapel Halberd entered an invalid lunge phase");
    }
}
static bool Jump(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    Uint16 collision = 0;
    switch (a->substep)
    {
    case 0:
        if (!ActorAnimate(s, a, 0x1dac, &animated))
            return false;
        if (!(animated & 1U))
        {
            bool facing = a->halberd.movement_facing;
            if (!(ActorRandom(s) & 3U))
                facing = !facing;
            a->vx = facing ? 131072 : -131072;
            a->vy = -196608;
            a->pose = a->remaining = 0;
            ++a->substep;
        }
        return true;
    case 1:
        if (!ActorGround(s, a, 0x1e5c, &collision))
            return false;
        if (collision)
        {
            if (!ActorPositionalSound(s, a, 0x648))
                return false;
            ++a->substep;
        }
        return ActorField(s, a, 0x1e74, 2, true, &collision);
    case 2:
        if (!ActorAnimate(s, a, 0x1db8, &animated))
            return false;
        if (!animated)
            SetStep(a, 3);
        return true;
    default:
        return SDL_SetError("Chapel Halberd entered an invalid jump phase");
    }
}
static bool Weapon(SotnActors *s, Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || s->slots[slot].state.id != 0x32 ||
        s->slots[slot].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step && !ActorInitialize(s, a, 0xa34))
        return false;
    a->state.hitbox_state = 1;
    const Actor *body = &s->slots[slot];
    const Uint16 index =
        body->state.frame >= 13 && body->state.frame <= 20 ? (Uint16)(body->state.frame - 13) : 0;
    SotnDataView hitbox = {0};
    if (!Slice(s->nz0, 0x1e80U + index * 8U, 8, &hitbox))
        return false;
    a->state.x = body->state.x;
    a->state.y = body->state.y;
    a->state.facing_left = body->state.facing_left;
    a->state.hitbox_x = (Sint16)Signed16(hitbox.bytes);
    a->state.hitbox_y = (Sint16)Signed16(hitbox.bytes + 2);
    const Sint32 w = Signed16(hitbox.bytes + 4), h = Signed16(hitbox.bytes + 6);
    if (w < 0 || w > 127 || h < 0 || h > 127)
        return SDL_SetError("Chapel Halberd weapon hitbox exceeds its original byte extents");
    a->state.hitbox_width = (Uint16)w;
    a->state.hitbox_height = (Uint16)h;
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    for (Uint16 i = 0; i < 9; ++i)
    {
        Actor *part = NULL;
        if (!ActorSpawn(s, a, 0x34, i, 160, 192, &part))
            return false;
        if (!part)
            break;
        SotnDataView life = {0}, x = {0}, y = {0}, vx = {0}, vy = {0};
        if (!Slice(s->nz0, 0x1dd8U + i, 1, &life) || !Slice(s->nz0, 0x1e2cU + i * 2U, 2, &x) ||
            !Slice(s->nz0, 0x1e40U + i * 2U, 2, &y) || !Slice(s->nz0, 0x1de4U + i * 4U, 4, &vx) ||
            !Slice(s->nz0, 0x1e08U + i * 4U, 4, &vy))
            return false;
        const Sint32 dx = Signed16(x.bytes), dy = Signed16(y.bytes);
        part->vx = Signed32(vx.bytes);
        part->vy = Signed32(vy.bytes);
        if (!life.bytes[0] || SDL_abs(dx) > 128 || SDL_abs(dy) > 128 || part->vx < -6 * 65536 ||
            part->vx > 6 * 65536 || part->vy < -8 * 65536 || part->vy > 8 * 65536)
            return SDL_SetError("Chapel Halberd death profile exceeds selected bounds");
        part->timer = life.bytes[0];
        part->state.x =
            Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? -dx : dx)) * 65536;
        part->state.y = Wrap16((Sint64)Pixel(a->state.y) + dy) * 65536;
    }
    const Uint32 slot = (Uint32)a->slot + 1U;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Chapel Halberd death has no adjacent weapon reservation");
    Actor *child = &s->slots[slot];
    if (child->state.generation && child->state.id == 0x33 &&
        child->enemy_state.owner_slot == a->slot &&
        child->enemy_state.owner_generation == a->state.generation)
        child->remove = true;
    else if (child->state.generation)
        return SDL_SetError("Chapel Halberd death lost its owned weapon");
    if (!ActorSound(s, 0x62a))
        return false;
    a->remove = true;
    return true;
}
static bool Part(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= 9 || !a->timer)
        return SDL_SetError("Chapel Halberd part has invalid index or lifetime");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0xa28))
            return false;
        a->state.hitbox_state = 0;
        a->state.flags |= 0xc0202000U;
        a->state.frame = (Uint16)(a->state.parameters + 32U);
        a->state.rotated = true;
        if (a->state.facing_left)
            a->vx = -a->vx;
        return true;
    }
    if (!--a->timer)
    {
        // The original conversion retains this frame and rotation until explosion init.
        a->state.id = 2;
        a->state.parameters = a->state.step = 0;
        return true;
    }
    SotnDataView rotation = {0};
    if (!Slice(s->nz0, 0x1dc4U + a->state.parameters * 2U, 2, &rotation))
        return false;
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + Signed16(rotation.bytes));
    if (a->vy < 6 * 65536)
        a->vy += 16384;
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Halberd part exceeds bounded fixed-point motion");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
bool ActorChapelHalberd(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || a->state.id < 0x32 || a->state.id > 0x34)
        return SDL_SetError("Chapel Halberd requires its original stage identity");
    if (a->state.id == 0x33)
        return Weapon(s, a);
    if (a->state.id == 0x34)
        return Part(s, a);
    if (a->state.flags & 0x100U)
        a->state.step = 6;
    Uint8 animated = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0xa28))
            return false;
        a->halberd.timer = 64;
        a->halberd.interval = 0;
        a->halberd.movement_facing = false;
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x33, 0, (Uint32)a->slot + 1U, &child))
            return false;
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Pixel(a->state.y) * 65536;
        child->state.facing_left = false;
        return true;
    }
    case 1:
        if (!ActorGround(s, a, 0x1e5c, &collision))
            return false;
        if (collision)
            ++a->state.step;
        return true;
    case 2:
    case 3: {
        const bool retreating = a->state.step == 3;
        if (!ActorAnimate(s, a, retreating ? 0x1d60 : 0x1d40, &animated))
            return false;
        if (!animated)
            a->state.facing_left = Side(a, s->player) == 0;
        a->halberd.movement_facing = a->state.facing_left != retreating;
        a->vx = a->halberd.movement_facing ? 32768 : -32768;
        if (retreating ? Distance(a, s->player) > 92 : Distance(a, s->player) < 76)
            a->state.step = retreating ? 2 : 3;
        return MoveDecision(s, a);
    }
    case 4:
        if (!ActorAnimate(s, a, 0x1d70, &animated) || !AttackSounds(s, a))
            return false;
        if (!animated)
        {
            SetStep(a, 3);
            return Interval(s, a);
        }
        return true;
    case 5:
        return Jump(s, a);
    case 6:
        return Death(s, a);
    case 7:
        return Lunge(s, a);
    default:
        return SDL_SetError("Chapel Halberd entered an invalid original state");
    }
}
