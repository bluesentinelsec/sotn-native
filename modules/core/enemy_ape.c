// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_skeleton_ape.h at the pinned reference; original NO1 tables remain private.
#include "actors_internal.h"
static Actor *Parent(SotnActors *s, const Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[a->enemy_state.owner_slot];
    return p->state.id == 0x53 && p->state.generation == a->enemy_state.owner_generation ? p : NULL;
}
static void Link(Actor *a, const Actor *p)
{
    a->enemy_state.owner_slot = p->slot;
    a->enemy_state.owner_generation = p->state.generation;
    a->state.x = Pixel(p->state.x) * 65536;
    a->state.y = Pixel(p->state.y) * 65536;
    a->state.facing_left = false;
}
static bool Burst(SotnActors *s, Actor *a, Uint16 kind, Uint16 sound)
{
    return ActorSound(s, sound) && ActorReplaceExplosion(s, a, kind);
}
static bool Punch(SotnActors *s, Actor *a)
{
    Actor *p = Parent(s, a);
    if (p == NULL)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step && !ActorInitialize(s, a, 0xb3c))
        return false;
    const Uint16 frame =
        p->state.frame >= 9 && p->state.frame <= 14 ? (Uint16)(p->state.frame - 9) : 0;
    SotnDataView position = {0}, size = {0};
    if (frame > 4 || !Slice(s->nz0, 0x32fc + frame * 4U, 4, &position) ||
        !Slice(s->nz0, 0x3310 + frame * 4U, 4, &size) || SDL_abs(Signed16(position.bytes)) > 64 ||
        SDL_abs(Signed16(position.bytes + 2)) > 64 || Read16(size.bytes) > 64 ||
        Read16(size.bytes + 2) > 64)
        return SDL_SetError("Skeleton Ape punch frame or private hitbox is invalid");
    a->state.hitbox_x = (Sint16)Signed16(position.bytes);
    a->state.hitbox_y = (Sint16)Signed16(position.bytes + 2);
    a->state.hitbox_width = Read16(size.bytes);
    a->state.hitbox_height = Read16(size.bytes + 2);
    a->state.facing_left = p->state.facing_left;
    a->state.hitbox_state = p->state.hitbox_state & 1;
    a->state.x = Pixel(p->state.x) * 65536;
    a->state.y = Pixel(p->state.y) * 65536;
    return true;
}
static void Carry(Actor *a, const Actor *p)
{
    a->state.x = (Pixel(p->state.x) + (a->state.facing_left ? -10 : 10)) * 65536;
    a->state.y = (Pixel(p->state.y) - 28) * 65536;
}
static bool Fragment(SotnActors *s, Actor *a)
{
    const Uint16 kind = a->state.parameters;
    if (kind > 3)
        return SDL_SetError("Skeleton Ape barrel fragment exceeds its private four-part recipe");
    if (!a->state.step)
    {
        SotnDataView vx = {0}, vy = {0}, frame = {0}, time = {0};
        if (!ActorInitialize(s, a, 0xb30) || !Slice(s->nz0, 0x334c + kind * 4U, 4, &vx) ||
            !Slice(s->nz0, 0x335c + kind * 4U, 4, &vy) ||
            !Slice(s->nz0, 0x336c + kind * 2U, 2, &frame) ||
            !Slice(s->nz0, 0x3374 + kind * 2U, 2, &time))
            return false;
        const Sint32 speed = Signed32(vx.bytes), vertical = Signed32(vy.bytes);
        if (speed < -4 * 65536 || speed > 4 * 65536 || vertical < -4 * 65536 || vertical > 0 ||
            Read16(frame.bytes) > 256 || !Read16(time.bytes) || Read16(time.bytes) > 256)
            return SDL_SetError("Skeleton Ape barrel fragment motion or lifetime is invalid");
        a->ape.speed = (a->state.facing_left ? -speed : speed) + a->vx;
        a->vy = vertical;
        a->state.frame = Read16(frame.bytes);
        a->ape.counter = Read16(time.bytes);
        a->state.flags = 0xcc002000;
        a->state.hitbox_state = 0;
        return true;
    }
    a->vx = a->ape.speed;
    const Sint32 vy = a->vy;
    Uint16 collision = 0;
    if (!ActorGround(s, a, 0x337c + kind * 16U, &collision))
        return false;
    if (collision & 1)
    {
        if (vy > 0)
        {
            Sint32 rebound = vy / 2;
            a->ape.speed /= 2;
            if (rebound < 16384)
                rebound = a->ape.speed = 0;
            a->vy = -rebound;
            a->vx = a->ape.speed;
        }
        else
            return Burst(s, a, 0, 0x655);
    }
    SotnCollider wall = {0};
    if (!ActorQuery(s, Pixel(a->state.x) + (a->state.facing_left ? 16 : -16), Pixel(a->state.y),
                    &wall))
        return false;
    if ((wall.effects & SOTN_EFFECT_SIDE) || --a->ape.counter == 0)
        return Burst(s, a, 0, 0x655);
    return true;
}
static bool Barrel(SotnActors *s, Actor *a)
{
    Actor *p = Parent(s, a);
    if (a->state.parameters > 1)
        return SDL_SetError("Skeleton Ape barrel variant is invalid");
    if ((a->state.flags & 0x100) && a->state.step != 4)
        SetStep(a, 4);
    if (a->state.step < 3)
    {
        if (p == NULL)
        {
            a->remove = true;
            return true;
        }
        if (p->state.step == 7)
            SetStep(a, 4);
    }
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xb30))
            return false;
        a->state.facing_left = p->state.facing_left;
        Carry(a, p);
        if (a->state.parameters)
            a->state.flags |= 0x2000;
        break;
    case 1:
        Carry(a, p);
        break;
    case 2:
        if (a->ape.counter)
        {
            --a->ape.counter;
            Carry(a, p);
            break;
        }
        a->vy = -4 * 65536;
        a->vx = a->state.facing_left ? 2 * 65536 : -2 * 65536;
        a->ape.bounces = a->state.parameters ? 3 : 0;
        a->state.rotated = true;
        a->state.flags |= 0xc0000000;
        ++a->state.step;
        break;
    case 3: {
        if (!ActorAnimate(s, a, 0x3324, &animation))
            return false;
        a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation - 64);
        const Sint32 vy = a->vy;
        if (!ActorGround(s, a, 0x332c, &collision))
            return false;
        if ((collision & 1) && vy > 0)
        {
            const Sint32 rebound = vy / 2;
            a->vy = rebound < 32768 ? 0 : -rebound;
            if (++a->ape.bounces >= 3)
            {
                SetStep(a, 4);
                break;
            }
        }
        a->vx = a->state.facing_left ? 2 * 65536 : -2 * 65536;
        if (!(a->ape.counter & 7))
        {
            Actor *smoke = NULL;
            if (!ActorSpawn(s, a, 2, 0, 160, 192, &smoke))
                return false;
            if (smoke != NULL)
            {
                const Uint32 index = ActorRandom(s) & 7;
                SotnDataView x = {0}, y = {0};
                if (!Slice(s->nz0, 0x333c + index * 2U, 2, &x) ||
                    !Slice(s->nz0, 0x333c + ((index + 4) & 7) * 2U, 2, &y) ||
                    SDL_abs(Signed16(x.bytes)) > 32 || SDL_abs(Signed16(y.bytes)) > 32)
                    return SDL_SetError("Skeleton Ape barrel smoke offset is invalid");
                smoke->state.x = (Pixel(a->state.x) + Signed16(x.bytes)) * 65536;
                smoke->state.y = (Pixel(a->state.y) + Signed16(y.bytes)) * 65536;
                smoke->state.facing_left = false;
            }
        }
        ++a->ape.counter;
        if (!a->state.parameters)
        {
            SotnCollider wall = {0};
            if (!ActorQuery(s, Pixel(a->state.x) + (a->state.facing_left ? 16 : -16),
                            Pixel(a->state.y) + 8, &wall))
                return false;
            if (wall.effects & SOTN_EFFECT_SIDE)
                SetStep(a, 4);
        }
        break;
    }
    case 4:
        if (a->substep)
            return Burst(s, a, 3, 0x65a);
        if (p != NULL && p->ape.barrel_generation == a->state.generation)
            p->ape.barrel_generation = 0;
        for (Uint16 i = 0; i < 4; ++i)
        {
            Actor *piece = NULL;
            if (!ActorSpawn(s, a, 0x56, i, 160, 192, &piece))
                return false;
            if (piece != NULL)
            {
                piece->state.x = Pixel(a->state.x) * 65536;
                piece->state.y = Pixel(a->state.y) * 65536;
                piece->state.facing_left = a->state.facing_left;
                piece->vx = a->vx;
            }
        }
        ++a->substep;
        break;
    default:
        return SDL_SetError("Skeleton Ape barrel entered invalid state %u", a->state.step);
    }
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100) && a->state.step != 7)
    {
        a->ape.counter = 0;
        SetStep(a, 7);
    }
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (a->state.parameters > 2 || a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS ||
            s->slots[a->slot + 1].state.generation)
            return SDL_SetError("Skeleton Ape variant or reserved punch slot is invalid");
        if (!ActorInitialize(s, a, 0xb24))
            return false;
        if (a->state.parameters == 2)
            a->state.flags |= 0x2000;
        a->state.facing_left = Side(a, s->player) == 0;
        Actor *child = NULL;
        if (!ActorSpawnAt(s, a, 0x54, 0, a->slot + 1U, &child))
            return false;
        Link(child, a);
        if (!a->state.parameters)
            break;
        if (!ActorSpawn(s, a, 0x55, a->state.parameters == 2 ? 1 : 0, 96, 128, &child))
            return false;
        if (child == NULL)
        {
            a->state.parameters = 0;
            break;
        }
        Link(child, a);
        child->state.priority = (Uint16)(a->state.priority - 1);
        a->ape.barrel_slot = child->slot;
        a->ape.barrel_generation = child->state.generation;
        if (a->state.parameters == 2)
            a->state.hitbox_state = 0;
        a->state.frame = 15;
        break;
    }
    case 1:
        if (a->substep)
        {
            if (!ActorAnimate(s, a, 0x32e8, &animation))
                return false;
            if (!animation)
                SetStep(a, 2);
        }
        else
        {
            if (!ActorGround(s, a, 0x3230, &collision))
                return false;
            if (collision & 1)
            {
                if (a->state.parameters)
                    SetStep(a, 2);
                else
                {
                    ++a->substep;
                    a->pose = a->remaining = 0;
                }
            }
        }
        break;
    case 2:
        if (a->state.parameters)
        {
            if (!ActorAnimate(s, a, 0x329c, &animation))
                return false;
            Actor *barrel =
                a->ape.barrel_slot < SOTN_STAGE_ACTOR_SLOTS ? &s->slots[a->ape.barrel_slot] : NULL;
            if (barrel == NULL || !a->ape.barrel_generation ||
                barrel->state.generation != a->ape.barrel_generation || barrel->state.id != 0x55)
            {
                a->state.parameters = 0;
                break;
            }
            if ((Distance(a, s->player) < 96 &&
                 SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 80) ||
                a->state.parameters == 2)
            {
                SetStep(a, 6);
                ++barrel->state.step;
                barrel->ape.counter = 4;
                if (a->state.parameters == 2)
                    a->state.hitbox_state = 3;
                return true;
            }
        }
        else
        {
            if (!ActorAnimate(s, a, 0x3268, &animation))
                return false;
            const bool front = a->state.facing_left != (Side(a, s->player) != 0);
            if (Distance(a, s->player) < (front ? 34 : 28))
            {
                SetStep(a, front ? 4 : 5);
                break;
            }
        }
        a->vx = a->state.facing_left ? 16384 : -16384;
        if (!ActorWalk(s, a, 0x3240, &collision))
            return false;
        if ((collision & 96) == 96)
            a->state.x -= a->vx;
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x32e8, &animation))
            return false;
        if (!animation)
        {
            a->state.step = 1;
            a->vx = a->state.facing_left ? 16384 : -16384;
            a->vy = -4 * 65536;
        }
        break;
    case 4:
    case 5:
    case 6: {
        const Uint16 step = a->state.step;
        if (!ActorAnimate(s, a, step == 4 ? 0x3280 : step == 5 ? 0x3294 : 0x32b4, &animation))
            return false;
        if (!animation)
        {
            SetStep(a, 2);
            if (step == 6)
                a->state.parameters = 0;
        }
        else if ((animation & 128) && ((step == 4 && a->pose == 3) || (step == 5 && a->pose == 2)))
            return ActorSound(s, step == 4 ? 0x62d : 0x6c8);
        break;
    }
    case 7:
        if (!ActorAnimate(s, a, 0x32c4, &animation))
            return false;
        if (!animation)
            return Burst(s, a, 2, 0x684);
        if ((animation & 128) && a->state.frame > 28)
        {
            if (a->ape.counter >= 8 || !ActorSound(s, 0x655))
                return SDL_SetError("Skeleton Ape death burst exceeds its private positions");
            Actor *burst = NULL;
            if (!ActorSpawn(s, a, 2, 0, 160, 192, &burst))
                return false;
            if (burst != NULL)
            {
                SotnDataView x = {0}, y = {0};
                if (!Slice(s->nz0, 0x3248 + a->ape.counter * 2U, 2, &x) ||
                    !Slice(s->nz0, 0x3258 + a->ape.counter * 2U, 2, &y) ||
                    SDL_abs(Signed16(x.bytes)) > 64 || SDL_abs(Signed16(y.bytes)) > 64)
                    return SDL_SetError("Skeleton Ape death burst offset is invalid");
                burst->state.x =
                    (Pixel(a->state.x) + (a->state.facing_left ? -1 : 1) * Signed16(x.bytes)) *
                    65536;
                burst->state.y = (Pixel(a->state.y) + Signed16(y.bytes)) * 65536;
                burst->state.facing_left = false;
            }
            ++a->ape.counter;
        }
        break;
    default:
        return SDL_SetError("Skeleton Ape entered invalid state %u", a->state.step);
    }
    return true;
}
bool ActorSkeletonApe(SotnActors *s, Actor *a)
{
    return a->state.id == 0x54   ? Punch(s, a)
           : a->state.id == 0x55 ? Barrel(s, a)
           : a->state.id == 0x56 ? Fragment(s, a)
                                 : Body(s, a);
}
