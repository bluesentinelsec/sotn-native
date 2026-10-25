// SPDX-License-Identifier: AGPL-3.0-or-later
// st/no0/e_diplocephalus.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Private NO0 supplies motion, sensor, animation and enemy-definition tables.
#include "actors_internal.h"
Actor *ActorDiploOwner(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[slot];
    return p->state.id == 0x21 && p->state.generation == a->enemy_state.owner_generation ? p : NULL;
}
static bool Motion(SotnActors *s, size_t offset, Uint32 index, Sint32 *out)
{
    SotnDataView value = {0};
    if (index >= 16 || !Slice(s->nz0, offset + index * 4U, 4, &value))
        return false;
    const Sint32 motion = Signed32(value.bytes);
    if (motion < -16 * 65536 || motion > 16 * 65536)
        return SDL_SetError("Diplocephalus private motion exceeds its bounded velocity");
    *out = motion;
    return true;
}
static bool Offsets(SotnActors *s, const Actor *a, size_t offset, bool facing, Uint8 *out)
{
    *out = 0;
    for (Uint32 i = 0; i < 8; ++i)
    {
        SotnDataView value = {0};
        if (!Slice(s->nz0, offset + i * 4U, 2, &value))
            return false;
        const Sint32 dx = Signed16(value.bytes);
        if (dx == 255)
            return true;
        if (!Slice(s->nz0, offset + i * 4U, 4, &value))
            return false;
        const Sint32 dy = Signed16(value.bytes + 2);
        if (SDL_abs(dx) > 256 || SDL_abs(dy) > 256)
            return SDL_SetError("Diplocephalus private collider offset is outside its bounds");
        SotnCollider c = {0};
        if (!ActorQuery(s, Wrap16((Sint64)Pixel(a->state.x) + (facing ? dx : -dx)),
                        Wrap16((Sint64)Pixel(a->state.y) + dy), &c))
            return false;
        *out = (Uint8)((*out << 1) | ((c.effects & SOTN_EFFECT_SOLID) ? 1 : 0));
    }
    return SDL_SetError("Diplocephalus collider offsets lack a bounded terminator");
}
static bool UpdateFoot(SotnActors *s, Actor *a, const Actor *root)
{
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        SotnDataView frame = {0};
        if (a->diplo.index < 1 || a->diplo.index > 4 || !ActorInitialize(s, a, 0xb30) ||
            !Slice(s->nz0, 0x1d84 + a->diplo.index - 1U, 1, &frame))
            return false;
        a->state.hitbox_y = 4;
        a->state.frame = frame.bytes[0];
        SDL_FALLTHROUGH;
    }
    case 1:
        if (!ActorGround(s, a, 0x1d54, &collision))
            return false;
        if (collision & 1)
            ++a->state.step;
        break;
    case 2:
        if (root->state.step >= 4)
        {
            a->vx = a->state.facing_left ? 65536 : -65536;
            a->vy = -0x2aaa4;
            a->diplo.phase = 0;
            a->diplo.acceleration = -0x71d;
            --a->state.frame;
            a->state.step = 4;
        }
        break;
    case 3:
        a->vx = a->state.facing_left ? -0x22aaa : 0x22aaa;
        a->vy = -0x7fffe;
        a->diplo.phase = 0;
        a->diplo.acceleration = 0x26aaa;
        --a->state.frame;
        a->state.step = 4;
        break;
    case 4:
        a->vy += a->diplo.acceleration;
        ++a->diplo.phase;
        if (!ActorGround(s, a, 0x1d54, &collision))
            return false;
        if (collision & 1)
        {
            if (!ActorSound(s, 0x779))
                return false;
            a->vx = a->vy = 0;
            ++a->state.frame;
            ++a->state.step;
        }
        break;
    case 5:
        ++a->diplo.phase;
        break;
    default:
        return SDL_SetError("Diplocephalus foot entered invalid gait state %u", a->state.step);
    }
    return true;
}
static bool Limb(SotnActors *s, Actor *a, Actor *root, bool leg)
{
    if (a->diplo.index < 1 || a->diplo.index > 4 || a->state.parameters > 1)
        return SDL_SetError("Diplocephalus limb index or death variation is invalid");
    if (a->state.step == 6)
    {
        if (!Motion(s, 0x1ce8,
                    a->state.parameters * 8U + (leg ? a->diplo.index + 3U : a->diplo.index - 1U),
                    &a->vy))
            return false;
        a->vy -= 8 * 65536;
        a->diplo.acceleration = a->vy;
        a->state.rotated = leg;
        a->state.hitbox_state = 0;
        ++a->state.step;
        Actor *trail = NULL;
        if (!ActorSpawn(s, a, 0x13, 2, 96, 128, &trail))
            return false;
        if (trail != NULL)
        {
            trail->enemy_state.owner_slot = a->slot;
            trail->enemy_state.owner_generation = a->state.generation;
            trail->table.angle = a->state.id;
        }
    }
    else if (a->state.step == 7)
    {
        if (!Motion(s, leg ? 0x1cd8 : 0x1cc8, a->diplo.index - 1U, &a->vx))
            return false;
        if (!a->state.facing_left)
            a->vx = -a->vx;
        a->state.rotation =
            (Sint16)((a->state.rotation + (a->diplo.index & 1 ? 256 : -256)) & 4095);
        Uint16 collision = 0;
        if (!ActorGround(s, a, leg ? 0x1d64 : 0x1d54, &collision))
            return false;
        if (collision & 1)
        {
            if (leg)
            {
                a->diplo.acceleration /= 2;
                if (a->diplo.acceleration)
                {
                    a->vy = a->diplo.acceleration;
                    return true;
                }
                if (!ActorSound(s, 0x655))
                    return false;
            }
            else
            {
                Actor origin = *a;
                origin.state.priority = (Uint16)(a->state.priority + 9);
                if (!ActorExplosionInPool(s, &origin, 3, 96, 128, origin.state.priority))
                    return false;
                a->remove = true;
                return true;
            }
            return ActorReplaceExplosion(s, a, 0);
        }
    }
    else if (leg)
    {
        if (a->state.step == 0 && !ActorInitialize(s, a, 0xb3c))
            return false;
        if (a->state.step != 1)
            return SDL_SetError("Diplocephalus leg entered invalid linked state");
        const Actor *foot = &s->slots[root->slot + a->diplo.index];
        if (foot->enemy_state.owner_generation != root->state.generation)
            return SDL_SetError("Diplocephalus leg lost its original foot");
        const Sint32 offset = a->state.facing_left ? 18 * 65536 : -18 * 65536,
                     correction = a->state.facing_left ? 6 * 65536 : -6 * 65536;
        a->state.x = (Sint32)(((Sint64)root->state.x + offset + foot->state.x) / 2) - correction;
        if (!(a->diplo.index & 1))
            a->state.x -= offset - correction;
        a->state.y = (Sint32)(((Sint64)root->state.y + 12 * 65536 + foot->state.y) / 2) - 8 * 65536;
        if (a->diplo.index & 1)
            a->state.y -= 4 * 65536;
        Uint8 animation = 0;
        return ActorAnimate(s, a, a->diplo.index < 3 ? 0x1dac : 0x1db4, &animation);
    }
    /* Normal feet update once, from the body, in the original gait order. */
    return true;
}
static bool SpawnBody(SotnActors *s, Actor *a)
{
    const Uint32 base = a->slot;
    if (base + 18 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Diplocephalus requires its original nineteen-slot reservation");
    for (Uint32 i = 1; i <= 18; ++i)
        if (s->slots[base + i].state.generation)
            return SDL_SetError("Diplocephalus reserved body slot is occupied");
    if (!ActorInitialize(s, a, 0xb24))
        return false;
    a->state.facing_left = Side(a, s->player) != 0;
    for (Uint16 i = 1; i <= 9; ++i)
    {
        Actor *child = NULL;
        if (!ActorSpawnAt(s, a, i <= 4 ? 0x22 : i <= 8 ? 0x23 : 0x24, 0, base + i, &child))
            return false;
        child->state.facing_left = !a->state.facing_left;
        child->diplo.index = i <= 4 ? i : i <= 8 ? (Uint16)(i - 4) : 0;
        child->state.priority = (Uint16)(a->state.priority + (i <= 2   ? 8
                                                              : i <= 4 ? -8
                                                              : i <= 6 ? 4
                                                              : i <= 8 ? -4
                                                                       : 0));
        if (i <= 4)
        {
            child->state.x += (((i - 1 + a->state.facing_left) & 1) ? -18 : 18) * 65536;
            child->state.x += ((a->state.facing_left != ((i - 1) >> 1)) ? 9 : -9) * 65536;
            ShiftY(child, 14);
            if (!UpdateFoot(s, child, a))
                return false;
        }
    }
    Actor *torso = NULL;
    if (!ActorSpawnAt(s, a, 0x26, 0, base + 18, &torso))
        return false;
    a->state.step = 2;
    return true;
}
static bool Burst(SotnActors *s, const Actor *a, Uint32 count)
{
    for (Uint32 i = 0; i < count; ++i)
    {
        Actor origin = *a;
        origin.state.x += ((Sint32)(ActorRandom(s) & 63) - 32) * 65536;
        ShiftY(&origin, (Sint32)(ActorRandom(s) & 31) - 15);
        origin.state.priority = (Uint16)(a->state.priority + 2);
        if (!ActorExplosionInPool(s, &origin, 1, 96, 128, origin.state.priority))
            return false;
    }
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    a->hit_params |= a->diplo.last_hit;
    a->diplo.hits = a->hit_params;
    if ((a->state.flags & 0x100) && a->state.step < 11)
        a->state.step = 11;
    const Uint32 base = a->slot;
    if (base + 18 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Diplocephalus body has an invalid child reservation");
    Uint8 animation = 0, collision = 0;
    switch (a->state.step)
    {
    case 0:
        return SpawnBody(s, a);
    case 2: {
        const Actor *first = &s->slots[base + 1];
        a->vx = first->vx;
        a->vy = first->vy;
        a->state.x += a->vx;
        a->state.y += a->vy;
        bool grounded = true;
        for (Uint32 i = 1; i <= 4; ++i)
            grounded = grounded && s->slots[base + i].state.step >= 2;
        if (grounded)
        {
            a->vy = 0;
            ++a->state.step;
        }
        for (Uint32 i = 1; i <= 4; ++i)
            if (!UpdateFoot(s, &s->slots[base + i], a))
                return false;
        break;
    }
    case 3:
        if (++a->diplo.phase == 1)
        {
            a->vy = 0x7ed0;
            ++a->diplo.cycles;
        }
        else
            a->vy -= 2048;
        if (a->diplo.phase == 32)
        {
            a->diplo.phase = 0;
            if (a->diplo.cycles < 2)
                break;
            a->vy = 0;
            a->substep ^= 1;
            a->diplo.active_foot = (Uint16)(4 - (a->substep << 1));
            a->state.step = 4;
            if (!Offsets(s, a, 0x1d30, !a->state.facing_left, &collision))
                return false;
            if (a->diplo.reaction || (collision ^ 1))
            {
                a->state.step = 6;
                a->substep ^= 1;
            }
        }
        a->diplo.reaction |= a->hit_params;
        if (!ActorAnimate(s, a, 0x1d88, &animation))
            return false;
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (!Offsets(s, a, 0x1d48, true, &collision))
            return false;
        if (collision & 1)
            ShiftY(a, -1);
        break;
    case 4: {
        const Uint16 active = a->diplo.active_foot;
        if (active != 2 && active != 4)
            return SDL_SetError("Diplocephalus forward gait lost its active pair");
        Actor *foot = &s->slots[base + active];
        if (!UpdateFoot(s, foot, a) || (foot->diplo.phase > 12 && !UpdateFoot(s, foot - 1, a)))
            return false;
        if (foot->state.step == 5 && (foot - 1)->state.step == 5)
        {
            foot->state.step = (foot - 1)->state.step = 2;
            a->state.step = a->diplo.reaction ? 6 : 3;
        }
        a->vx = ActorShift((foot - a->substep)->vx, 1);
        a->vy = foot->diplo.phase >= 24 ? 0 : ActorShift(foot->vy, 2);
        a->diplo.reaction |= a->hit_params;
        if (!ActorAnimate(s, a, 0x1d88, &animation))
            return false;
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    }
    case 6:
        if (++a->diplo.phase == 1)
        {
            a->pose = a->remaining = 0;
            a->vy = 0x1b200;
        }
        else
            a->vy -= 65536;
        if (a->diplo.phase == 4)
        {
            a->substep ^= 1;
            a->diplo.active_foot = (Uint16)((a->substep << 1) + 1);
            a->state.step = 7;
            for (Uint32 i = 1; i <= 4; ++i)
                s->slots[base + i].state.step = 3;
            ++a->diplo.walk_count;
            a->diplo.phase = 0;
            if (!Offsets(s, a, 0x1d3c, !a->state.facing_left, &collision))
                return false;
            if (((a->diplo.walk_count & 1) && (collision ^ 1)) || a->diplo.walk_count > 8)
            {
                a->state.step = 3;
                a->diplo.reaction = a->diplo.walk_count = a->diplo.phase = a->diplo.cycles = 0;
                a->vy = 0;
                a->state.parameters = 0;
                for (Uint32 i = 1; i <= 4; ++i)
                    s->slots[base + i].state.step = 2;
                a->pose = a->remaining = 0;
                a->substep ^= 1;
                /* Original drool presentation remains part of full effects integration. */
            }
        }
        if (!ActorAnimate(s, a, 0x1da0, &animation))
            return false;
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    case 7: {
        const Uint16 active = a->diplo.active_foot;
        if (active != 1 && active != 3)
            return SDL_SetError("Diplocephalus backward gait lost its active pair");
        Actor *foot = &s->slots[base + active];
        if (!UpdateFoot(s, foot, a) || (foot->diplo.phase > 2 && !UpdateFoot(s, foot + 1, a)))
            return false;
        if (foot->state.step == 5 && (foot + 1)->state.step == 5)
        {
            foot->state.step = (foot + 1)->state.step = 3;
            a->state.step = 6;
        }
        a->vx = ActorShift((foot + a->substep)->vx, 1);
        a->vy = foot->diplo.phase >= 6 ? 0 : ActorShift(foot->vy, 3);
        if (!ActorAnimate(s, a, 0x1da0, &animation))
            return false;
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    }
    case 11:
        s->slots[base + 9].state.step = 8;
        a->vx = 0;
        ++a->state.step;
        for (Uint32 i = 0; i < 10; ++i)
            if (i == 0)
                a->state.hitbox_state = 0;
            else
                s->slots[base + i].state.hitbox_state = 0;
        break;
    case 12: {
        Uint16 ground = 0;
        if (s->slots[base + 9].state.id != 0x24)
        {
            if (!ActorGround(s, a, 0x1d74, &ground))
                return false;
            if (ground & 1)
                ++a->state.step;
        }
        break;
    }
    case 13: {
        const Uint16 variation = (Uint16)(ActorRandom(s) & 1);
        for (Uint32 i = 1; i <= 8; ++i)
        {
            s->slots[base + i].state.step = 6;
            s->slots[base + i].state.parameters = variation;
        }
        for (Uint32 i = 0; i < 3; ++i)
        {
            SotnDataView offset = {0};
            if (!Slice(s->nz0, 0x1d28 + i * 2, 2, &offset) || SDL_abs(Signed16(offset.bytes)) > 128)
                return SDL_SetError("Diplocephalus death burst offset is invalid");
            Actor origin = *a;
            origin.state.x += Signed16(offset.bytes) * 65536;
            origin.state.priority = (Uint16)(a->state.priority + 9);
            if (!ActorExplosionInPool(s, &origin, 3, 96, 128, origin.state.priority))
                return false;
        }
        if (!ActorSound(s, 0x699))
            return false;
        a->diplo.reaction = a->diplo.cycles = 0;
        a->state.frame = 0;
        s->slots[base + 18].remove = true;
        ++a->state.step;
        break;
    }
    case 14:
        if (a->diplo.reaction++ >= 6)
        {
            a->diplo.reaction = 0;
            if (a->diplo.cycles < 24)
            {
                if (!Burst(s, a, 2) ||
                    (!(s->ticks & 2) && !ActorExplosionInPool(s, a, 2, 160, 192, 0)))
                    return false;
                ++a->diplo.cycles;
            }
            else
                a->remove = true;
        }
        break;
    default:
        return SDL_SetError("Diplocephalus body entered invalid state %u", a->state.step);
    }
    if (a->state.step > 2 && a->state.step < 11)
        a->state.x =
            (Sint32)(((Sint64)s->slots[base + 1].state.x + s->slots[base + 4].state.x) / 2);
    return true;
}
static bool Torso(SotnActors *s, Actor *a, Actor *root)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xa94))
            return false;
        a->diplo.previous_x = a->state.x;
        a->diplo.previous_y = a->state.y;
    }
    const Sint32 dx = Pixel(root->state.x) - Pixel(a->diplo.previous_x);
    a->state.x = root->state.x;
    a->state.y = root->state.y;
    if (root->state.step < 13)
    {
        Uint16 contact = 0;
        if (!ActorObjectCollision(s, a, 8, 18, 4, &contact) ||
            (contact && !ActorDisplace(s, dx * 65536, 2 * 65536, 0)))
            return false;
    }
    a->diplo.previous_x = a->state.x;
    a->diplo.previous_y = a->state.y;
    return true;
}
static bool LimbTrail(SotnActors *s, Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Diplocephalus limb trail owner slot is invalid");
    if (!a->state.step && !ActorInitialize(s, a, 0xa88))
        return false;
    if (a->table.timer++ > 4)
    {
        if (!ActorExplosionInPool(s, a, a->state.parameters, 160, 192, 0))
            return false;
        a->table.timer = 0;
    }
    const Actor *parent = &s->slots[a->enemy_state.owner_slot];
    if (parent->state.generation != a->enemy_state.owner_generation ||
        parent->state.id != a->table.angle)
        a->remove = true;
    else
    {
        a->state.x = parent->state.x;
        a->state.y = parent->state.y;
    }
    return true;
}
bool ActorDiplo(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x13)
        return LimbTrail(s, a);
    if (a->state.id == 0x21)
        return Body(s, a);
    if (a->state.id == 0x25)
        return ActorDiploFireball(s, a);
    Actor *root = ActorDiploOwner(s, a);
    if (root == NULL)
    {
        if (a->state.id == 0x26)
            a->remove = true;
        else
            return ActorReplaceExplosion(s, a, 0);
        return true;
    }
    if (a->state.id == 0x24)
        return ActorDiploTail(s, a, root);
    if (a->state.id == 0x26)
        return Torso(s, a, root);
    return Limb(s, a, root, a->state.id == 0x23);
}
