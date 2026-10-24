// SPDX-License-Identifier: AGPL-3.0-or-later
// st/nz0/e_spittle_bone.c at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Geometry, animations and particle trajectories remain private runtime data.
#include "actors_internal.h"
static Actor *Parent(SotnActors *s, const Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *parent = &s->slots[a->enemy_state.owner_slot];
    return parent->state.generation != 0 &&
                   parent->state.generation == a->enemy_state.owner_generation &&
                   parent->state.id == 0x31
               ? parent
               : NULL;
}
static bool Follow(SotnActors *s, Actor *a)
{
    Actor *parent = Parent(s, a);
    if (parent == NULL)
    {
        a->remove = true;
        return true;
    }
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xbf8))
            return false;
        a->state.flags &= ~(0x2000U | 0x200U);
        return true;
    }
    const Uint8 before = (Uint8)(a->enemy_state.counter & 63);
    Uint8 after = 0;
    if (!ActorCrawl(s, a, (Uint8)a->enemy_state.counter, &after))
        return false;
    a->enemy_state.counter = after;
    if ((after & 63) != before)
    {
        Uint8 index = after & 63;
        size_t table = 0x24dc;
        if (index & 0x3c)
        {
            index = (index & 0x3c) >> 2;
            table = 0x24e4;
        }
        else
            index &= 3;
        SotnDataView turn = {0};
        if (!Slice(s->nz0, table + index * 2, 2, &turn))
            return false;
        Sint32 angle = Signed16(turn.bytes) * (a->state.facing_left ? -1 : 1);
        a->state.rotation = (Sint16)angle;
        angle -= parent->state.rotation;
        if (angle > 0x800)
            angle -= 0x1000;
        if (angle < -0x800)
            angle += 0x1000;
        parent->enemy_state.angle_delta = (Sint16)(angle / 32);
        parent->enemy_state.timer = 32;
    }
    return true;
}
static bool Spit(SotnActors *s, Actor *a)
{
    Uint8 anim = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xcb8))
            return false;
        a->enemy_state.timer = 0;
        break;
    case 1: {
        Actor *parent = Parent(s, a);
        if (parent == NULL)
        {
            a->remove = true;
            return true;
        }
        if ((parent->state.rotation & 4095) == 0x800)
        {
            a->state.x = parent->state.x + (parent->state.facing_left ? -3 : 3) * 65536;
            a->state.y = parent->state.y + 27 * 65536;
            if (++a->enemy_state.timer > 16)
            {
                a->enemy_state.timer = 0;
                ++a->state.step;
            }
        }
        else
            ++a->state.step;
        break;
    }
    case 2: {
        if (!ActorAnimate(s, a, 0x2534, &anim))
            return false;
        a->state.x += a->vx * 2;
        a->state.y += a->vy * 2;
        a->vy += 4096;
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y), &c))
            return false;
        if (c.effects != 0)
        {
            if (!ActorSound(s, 0x73c) || !ActorExplosion(s, a, 1))
                return false;
            a->state.frame = 0;
            a->state.hitbox_state = 0;
            ++a->state.step;
        }
        break;
    }
    case 3:
        if (!ActorPrimitives(s, a, 9))
            return false;
        for (Uint16 i = 0; i < 9; ++i)
        {
            SotnDataView trajectory = {0};
            if (!Slice(s->nz0, 0x253c + i * 8, 8, &trajectory))
                return false;
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            p->state = (SotnActorPrimitive){.x = a->state.x,
                                            .y = a->state.y,
                                            .width = 1,
                                            .height = 2,
                                            .red = 32,
                                            .green = 32,
                                            .blue = 192,
                                            .blend = true,
                                            .active = true};
            p->vx = Signed32(trajectory.bytes);
            p->vy = Signed32(trajectory.bytes + 4);
        }
        a->enemy_state.timer = 48;
        ++a->state.step;
        break;
    case 4:
        for (Uint16 i = 0; i < a->state.primitive_count; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            p->state.x += p->vx;
            p->state.y += p->vy;
            p->vy += 6144;
            if (p->vy > 0)
            {
                SotnCollider c = {0};
                if (!ActorQuery(s, Pixel(p->state.x), Pixel(p->state.y) + 16, &c))
                    return false;
                if (c.effects != 0)
                    p->state.active = false;
            }
        }
        if (a->enemy_state.timer == 0)
            return SDL_SetError("Spittle Bone splash timer underflow");
        if (--a->enemy_state.timer == 0)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Spittle Bone projectile entered an invalid state");
    }
    return true;
}
bool ActorSpittle(SotnActors *s, Actor *a, SotnPlayerState player)
{
    (void)player;
    if (a->state.id == 0x32)
        return Follow(s, a);
    if (a->state.id == 0x33)
        return Spit(s, a);
    Uint16 collision = 0;
    Uint8 animation = 0, state = 0;
    if ((a->state.flags & 0x100) && a->state.step < 3)
        a->state.step = 3;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xcac))
            return false;
        a->state.flags &= ~(0x2000U | 0x200U);
        a->state.facing_left = a->state.parameters != 0;
        a->state.rotated = true;
        a->state.rotation = 0;
        break;
    case 1:
        if (!ActorGround(s, a, 0x24cc, &collision))
            return false;
        if (collision & 1)
        {
            Actor *child = NULL;
            a->enemy_state.counter = 0;
            if (!ActorSpawnAt(s, a, 0x32, 0, a->slot + 1U, &child))
                return false;
            a->vx = a->state.facing_left ? 65536 : -65536;
            child->state.x += (a->state.facing_left ? 16 : -16) * 65536;
            child->vx = a->vx;
            ++a->state.step;
        }
        break;
    case 2:
        if (!ActorAnimate(s, a, 0x2524, &animation) ||
            !ActorCrawl(s, a, (Uint8)a->enemy_state.counter, &state))
            return false;
        a->enemy_state.counter = state;
        if (a->enemy_state.timer != 0)
        {
            a->state.rotation = (Sint16)(a->state.rotation + a->enemy_state.angle_delta);
            if (--a->enemy_state.timer == 0)
            {
                if (a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS)
                    return SDL_SetError("Spittle Bone rotation child exceeds its slot");
                Actor *child = &s->slots[a->slot + 1U];
                if (child->state.id == 0x32 &&
                    child->enemy_state.owner_generation == a->state.generation)
                    a->state.rotation = child->state.rotation;
            }
        }
        if ((state & 63) == 1 && (ActorRandom(s) & 31) == 0)
        {
            Actor *spit = NULL;
            if (!ActorSpawn(s, a, 0x33, 0, 96, 128, &spit))
                return false;
            if (spit != NULL)
            {
                spit->state.y += 24 * 65536;
                spit->enemy_state.owner_slot = a->slot;
                spit->enemy_state.owner_generation = a->state.generation;
            }
        }
        a->state.hitbox_x = (Sint16)ActorShift((Sint64)ActorSin(a->state.rotation) * 16, 12);
        a->state.hitbox_y =
            (Sint16)ActorShift((Sint64)ActorSin(a->state.rotation + 1024) * -16, 12);
        break;
    case 3:
        for (Uint16 i = 0; i < 6; ++i)
        {
            Actor *piece = NULL;
            if (!ActorSpawn(s, a, 0x31, 0, 160, 192, &piece))
                return false;
            if (piece == NULL)
                break;
            SotnDataView vx = {0}, timer = {0};
            if (!Slice(s->nz0, 0x2504 + i * 4, 4, &vx) || !Slice(s->nz0, 0x251c + i, 1, &timer))
                return false;
            piece->state.flags = 0x2000U | 0x08000000U | 0x40000000U | 0x80000000U;
            piece->state.animation_set = a->state.animation_set;
            piece->state.texture_offset = a->state.texture_offset;
            piece->state.palette = 0x20d;
            piece->state.priority = a->state.priority;
            piece->state.frame = (Uint16)(i + 0x3a);
            piece->state.rotated = true;
            piece->state.rotation = a->state.rotation;
            piece->state.step = 4;
            piece->vx = Signed32(vx.bytes);
            piece->vy = -3 * 65536 - (Sint32)((ActorRandom(s) & 3) * 32768);
            piece->enemy_state.timer = timer.bytes[0];
        }
        {
            Actor origin = *a;
            origin.state.x += ActorShift((Sint64)ActorSin(a->state.rotation) * -16, 12) * 65536;
            origin.state.y +=
                ActorShift((Sint64)ActorSin(a->state.rotation + 1024) * -16, 12) * 65536;
            if (!ActorExplosion(s, &origin, 1) || !ActorSound(s, 0x62b))
                return false;
        }
        a->remove = true;
        break;
    case 4:
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += 10240;
        if (a->enemy_state.timer == 0)
            return SDL_SetError("Spittle Bone piece timer underflow");
        if (--a->enemy_state.timer == 0)
        {
            if (!ActorExplosion(s, a, 0))
                return false;
            a->remove = true;
        }
        break;
    default:
        return SDL_SetError("Spittle Bone entered an invalid original state");
    }
    return true;
}
