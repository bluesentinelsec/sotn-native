// SPDX-License-Identifier: AGPL-3.0-or-later
// Merman motion/combat from st/e_merman.h, e_merman2.h and e_misc.h at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All tables remain private.
#include "actors_internal.h"

static void Move(Actor *a)
{
    a->state.x += a->vx;
    a->state.y += a->vy;
}

static bool Spawn(SotnActors *s, const Actor *a, Uint16 id, Uint16 params, Sint32 dx, Sint32 dy,
                  Uint32 begin, Uint32 end)
{
    Actor *child = NULL;
    if (!ActorSpawn(s, a, EntranceActorId(s, id), params, begin, end, &child))
        return false;
    if (child != NULL)
    {
        child->state.x += dx * 65536;
        child->state.y += dy * 65536;
        child->state.priority = a->state.priority;
    }
    return true;
}

static bool Water(SotnActors *s, Actor *a, Sint32 offset, bool *air)
{
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + offset, &c))
        return false;
    *air = !(c.effects & SOTN_EFFECT_SOLID);
    if ((c.effects & SOTN_EFFECT_WATER) && !a->merman.underwater)
    {
        if (!Spawn(s, a, 0x3b, 0, 0, offset, 168, 192) || !ActorSound(s, 0x7c2))
            return false;
        a->merman.underwater = true;
    }
    return true;
}

static bool WaterLimit(SotnActors *s, const Actor *a, Uint32 index, Sint32 *out)
{
    SotnDataView value = {0};
    if (index < 3 || index > 4 ||
        !Slice(s->nz0,
               EntranceTable(s, 0x13ac, 0x1230) +
                   (index + ((a->state.parameters & 0x100U) >> 8)) * 2,
               2, &value))
        return false;
    *out = Signed16(value.bytes);
    return true;
}

static bool Bounce(SotnActors *s, Actor *a)
{
    SotnDataView sensors = {0};
    if (!Slice(s->nz0, EntranceTable(s, 0x3a5c, 0x236c), 4, &sensors))
        return false;
    if (a->vy < 6 * 65536)
        a->vy += 16384;
    Move(a);
    if (a->vy >= 0)
    {
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x) + Signed16(sensors.bytes),
                        Pixel(a->state.y) + Signed16(sensors.bytes + 2), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            ShiftY(a, c.top);
            a->vy = -a->vy / 2;
        }
    }
    return true;
}

static bool FaceWall(SotnActors *s, Actor *a, Sint32 offset)
{
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x) + (a->state.facing_left ? offset : -offset),
                    Pixel(a->state.y) + 8, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
        a->vx = 0;
    return true;
}

static bool Merman(SotnActors *s, Actor *a, bool second)
{
    Uint8 animation = 0;
    Uint16 collision = 0;
    bool air = false;
    const Uint16 death = second ? 8 : 9, falling = second ? 7 : 8;
    const size_t init = second ? EntranceTable(s, 0xb48, 0xab4) : EntranceTable(s, 0xb60, 0xacc),
                 ground =
                     second ? EntranceTable(s, 0x3938, 0x2248) : EntranceTable(s, 0x3a4c, 0x235c);
    const size_t swim =
                     second ? EntranceTable(s, 0x398c, 0x229c) : EntranceTable(s, 0x3ac0, 0x23d0),
                 walking =
                     second ? EntranceTable(s, 0x3960, 0x2270) : EntranceTable(s, 0x3a84, 0x2394);
    const size_t walk_sensors =
        second ? EntranceTable(s, 0x3958, 0x2268) : EntranceTable(s, 0x3a7c, 0x238c);
    if (!second && a->merman.underwater)
        a->state.palette = a->merman.palette;
    if (a->merman.ignore_collision && a->state.step < falling)
        SetStep(a, falling);
    if ((a->state.flags & 0x100) && a->state.step < death)
    {
        if (!ActorSound(s, 0x71d))
            return false;
        a->state.hitbox_state = 0;
        if (!second && a->state.step == 7)
        {
            if (!ActorExplosion(s, a, 2))
                return false;
            a->remove = true;
            return true;
        }
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        a->state.rotated = false;
        a->state.flags &= ~0x20000000U;
        SetStep(a, death);
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, init))
            return false;
        a->state.priority = 0xa9;
        a->vy = -65536;
        if (second)
        {
            a->state.hitbox_y = 8;
            if (a->state.parameters)
                ++a->state.palette;
        }
        else
        {
            a->merman.palette = a->state.palette = 0x2b9;
            a->state.hitbox_width = 5;
            a->state.hitbox_height = 17;
        }
        break;
    case 1: {
        if (!second && a->substep == 0)
        {
            a->state.hitbox_width = 18;
            a->state.hitbox_height = 4;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, swim, &animation))
            return false;
        Move(a);
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) - 24, &c))
            return false;
        if (!(c.effects & SOTN_EFFECT_WATER))
            SetStep(a, 3);
        break;
    }
    case 3: {
        if (a->substep == 0)
        {
            SotnDataView velocity = {0};
            if (!Slice(
                    s->nz0,
                    (second ? EntranceTable(s, 0x3914, 0x2224) : EntranceTable(s, 0x3a28, 0x2338)) +
                        (ActorRandom(s) & 3) * 8,
                    8, &velocity))
                return false;
            a->vx = Signed32(velocity.bytes);
            a->vy = Signed32(velocity.bytes + 4);
            if (!second)
            {
                a->state.hitbox_width = 5;
                a->state.hitbox_height = 17;
            }
            ++a->substep;
        }
        if (!ActorAnimate(s, a, swim, &animation))
            return false;
        if (animation == 0)
            a->state.facing_left = Side(a, s->player) == 0;
        Move(a);
        SotnCollider c = {0};
        Sint32 bottom = 0;
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) - 24, &c) ||
            !WaterLimit(s, a, 4, &bottom))
            return false;
        if (!(c.effects & SOTN_EFFECT_WATER))
            a->vy = 32768;
        if (Pixel(a->state.y) - 24 > bottom)
            a->state.y = (bottom - 24) * 65536;
        if (a->merman.timer++ > 32)
        {
            a->merman.timer = 0;
            a->substep = 0;
            if (Distance(a, s->player) >= 48 && !(ActorRandom(s) & 1))
                SetStep(a, 4);
        }
        break;
    }
    case 4:
        if (a->substep == 0)
        {
            a->vx = 0;
            a->vy = -557056;
            ++a->substep;
            if (second)
            {
                if (!ActorPrimitives(s, a, 1))
                    return false;
                a->state.frame = 0;
            }
        }
        else if (a->substep == 1)
        {
            Move(a);
            Sint32 surface = 0;
            if (!WaterLimit(s, a, 3, &surface))
                return false;
            if (Pixel(a->state.y) - (second ? 24 : 20) < surface)
            {
                if (!ActorSound(s, 0x7c2) || !Spawn(s, a, 0x3b, 0, 0, second ? -24 : -20, 168, 192))
                    return false;
                ++a->substep;
                if (second)
                {
                    a->state.rotated = true;
                    a->state.rotation = 0;
                }
            }
        }
        else if (a->substep == 2)
        {
            if (!second && !ActorAnimate(s, a, EntranceTable(s, 0x3aac, 0x23bc), &animation))
                return false;
            if (second)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
                if (p == NULL)
                    return false;
                if (a->vy > -49152)
                {
                    p->state.active = false;
                    a->state.frame = 18;
                    a->state.rotation = (Sint16)((a->state.rotation - 128) & 4095);
                    a->state.hitbox_height = 8;
                }
                else
                {
                    if (!ActorSprite(s, a, 0,
                                     a->state.x + (a->state.facing_left ? -15 : -13) * 65536,
                                     a->state.y - 16 * 65536, 0, 56, 32, 56, 0x12,
                                     (Uint16)(0x28c + (a->state.parameters & 1))))
                        return false;
                    if (a->state.facing_left)
                        for (Uint32 i = 0; i < 4; ++i)
                            p->state.u[i] = (Uint8)(32 - p->state.u[i]);
                    const Uint8 tint = (Uint8)((SDL_abs(a->vy) >> 12) - 10);
                    p->state.modulate = true;
                    p->state.red = p->state.green = (Uint8)(128 - tint / 2);
                    p->state.blue = (Uint8)(tint / 2 + 192);
                }
            }
            if (a->vy < 0)
            {
                Actor *trail = NULL;
                if (!ActorSpawn(s, a, EntranceActorId(s, second ? 0x40 : 0x43), 0, 168, 192,
                                &trail))
                    return false;
                if (trail != NULL)
                {
                    trail->state.x += ((Sint32)(ActorRandom(s) & 3) * 4 - 6) * 65536;
                    trail->state.priority = (Uint16)(a->state.priority + (second ? 1 : 0));
                    if (!second)
                    {
                        SotnDataView velocity = {0};
                        if (a->pose == 0 || a->pose > 9 ||
                            !Slice(s->nz0, EntranceTable(s, 0x3b0c, 0x241c) + (a->pose - 1U) * 4, 4,
                                   &velocity))
                            return SDL_SetError("Merman jump pose exceeds private velocity table");
                        trail->vx = Signed32(velocity.bytes);
                    }
                }
            }
            if (!second)
            {
                ++a->merman.timer2;
                if (a->vy < 0)
                {
                    if (a->merman.palette > 0x2b2 && a->merman.timer2 % 4 == 0)
                        --a->merman.palette;
                    a->state.palette = a->merman.palette;
                }
                else
                    a->state.palette = 0x2b2;
            }
            if (a->vy > 0 && !Water(s, a, second ? 27 : 21, &air))
                return false;
            if (!a->merman.underwater)
            {
                if (!ActorGround(s, a, ground, &collision))
                    return false;
                if (collision & 1)
                {
                    if (second)
                    {
                        ActorPrimitivesClear(s, a);
                        a->state.primitive_count = 0;
                        a->state.rotated = false;
                        a->state.hitbox_height = 21;
                        SetStep(a, 5);
                    }
                    else
                    {
                        a->pose = a->remaining = 0;
                        ++a->substep;
                    }
                }
            }
            else
            {
                a->state.flags |= 0xc0000000U;
                Move(a);
                a->vy += 16384;
            }
        }
        else
        {
            if (!ActorAnimate(s, a, EntranceTable(s, 0x3acc, 0x23dc), &animation))
                return false;
            if (animation == 0)
                SetStep(a, 5);
        }
        break;
    case 5:
        if (second && a->substep == 0)
        {
            if (!ActorAnimate(s, a, EntranceTable(s, 0x3984, 0x2294), &animation))
                return false;
            if (animation == 0)
            {
                a->pose = a->remaining = 0;
                ++a->substep;
            }
        }
        else if (a->substep == (second ? 1 : 0))
        {
            a->state.facing_left = Side(a, s->player) == 0;
            if (!second)
                a->vx = a->state.facing_left ? 24576 : -24576;
            SotnDataView timer = {0};
            if (!Slice(
                    s->nz0,
                    (second ? EntranceTable(s, 0x3934, 0x2244) : EntranceTable(s, 0x3a48, 0x2358)) +
                        (ActorRandom(s) & 3),
                    1, &timer))
                return false;
            a->merman.timer = timer.bytes[0];
            ++a->substep;
        }
        else
        {
            if (!ActorAnimate(s, a, walking, &animation) ||
                !ActorWalk(s, a, walk_sensors, &collision))
                return false;
            if (collision == 255)
                a->state.facing_left = !a->state.facing_left;
            a->vx = a->state.facing_left ? 24576 : -24576;
            if (collision & 0x80)
                a->state.x += a->vx;
            if (second && (a->state.parameters & 1))
                a->vx *= 2;
            if ((!second || !(a->state.parameters & 1)) &&
                (!second || (Uint32)(Pixel(a->state.x) - s->camera_x) < 256))
            {
                if (a->merman.timer == 0)
                    return SDL_SetError("Merman walk timer underflow");
                if (--a->merman.timer == 0)
                {
                    if (ActorRandom(s) & 1)
                        SetStep(a, 6);
                    else if (second)
                        --a->substep;
                    else
                        SetStep(a, 7);
                }
            }
            if (!Water(s, a, second ? 27 : 21, &air))
                return false;
            if (air)
                a->merman.ignore_collision = true;
        }
        break;
    case 6:
        if (a->substep == 0)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            ++a->substep;
        }
        else if (a->substep == 1)
        {
            if (!ActorAnimate(s, a,
                              second ? EntranceTable(s, 0x396c, 0x227c)
                                     : EntranceTable(s, 0x3a9c, 0x23ac),
                              &animation))
                return false;
            if (!second)
            {
                if (animation == 0)
                    SetStep(a, 5);
                if (a->pose == 4 && a->remaining == 0 &&
                    (!ActorSound(s, 0x662) ||
                     !Spawn(s, a, 0x42, 0, a->state.facing_left ? 12 : -12, -10, 96, 128)))
                    return false;
            }
            else if (animation == 0)
            {
                if (!ActorSound(s, 0x662) || !Spawn(s, a, 0x3c, 0, 0, -12, 96, 128))
                    return false;
                for (Uint16 i = 0; i < 3; ++i)
                    if (!Spawn(s, a, 0x3e, i == 0 ? 1 : 0,
                               (a->state.facing_left ? 1 : -1) * (8 + i * 8), -12, 160, 192))
                        return false;
                a->state.rotated = true;
                a->state.rotation = 0;
                a->merman.rotation = 1;
                a->vx = a->state.facing_left ? -6 * 65536 : 6 * 65536;
                a->vy = -4096;
                ShiftY(a, -1);
                ++a->substep;
            }
        }
        else if (a->substep == 2)
        {
            if (a->merman.rotation < 256 && (s->ticks & 1))
                a->merman.rotation *= 2;
            if (a->vx)
                a->vx += a->state.facing_left ? 12288 : -12288;
            if (!FaceWall(s, a, -20))
                return false;
            a->state.rotation = (Sint16)(a->state.rotation + a->merman.rotation);
            a->vy -= 8192;
            if (!ActorGround(s, a, ground, &collision))
                return false;
            if (collision & 1)
            {
                a->vx = a->state.facing_left ? -163840 : 163840;
                a->merman.rotation = 4;
                ShiftY(a, 10);
                ++a->substep;
            }
            if (!Water(s, a, 27, &air))
                return false;
            if (a->merman.underwater)
                a->merman.ignore_collision = true;
        }
        else
        {
            if (!FaceWall(s, a, -20))
                return false;
            Move(a);
            a->state.rotation += 192;
            if (a->state.rotation > 4096)
            {
                ShiftY(a, -10);
                a->state.rotated = false;
                SetStep(a, 5);
            }
            if (!Water(s, a, 27, &air))
                return false;
            if (air)
                a->merman.ignore_collision = true;
        }
        break;
    case 7:
        if (second)
            goto falling;
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, EntranceTable(s, 0x3acc, 0x23dc), &animation))
                return false;
            if (animation == 0)
                ++a->substep;
        }
        else if (a->substep == 1)
        {
            a->vx = a->state.facing_left ? 163840 : -163840;
            a->vy = -131072;
            a->merman.timer = 64;
            a->state.frame = 12;
            a->state.hitbox_width = 18;
            a->state.hitbox_height = 4;
            ++a->substep;
        }
        else if (a->substep == 2)
        {
            if (!FaceWall(s, a, 24) || !Bounce(s, a))
                return false;
            a->vx += a->state.facing_left ? -2048 : 2048;
            if (a->merman.timer == 0)
                return SDL_SetError("Merman lunge timer underflow");
            if (--a->merman.timer == 0)
            {
                a->vx = 0;
                a->vy = 131072;
                ShiftY(a, -9);
                if (!ActorGround(s, a, ground, &collision))
                    return false;
                a->pose = 2;
                a->remaining = 0;
                a->state.hitbox_width = 5;
                a->state.hitbox_height = 17;
                ++a->substep;
            }
            if (!Water(s, a, 11, &air))
                return false;
            if (a->merman.underwater)
                a->merman.ignore_collision = true;
        }
        else
        {
            if (!ActorGround(s, a, ground, &collision) ||
                !ActorAnimate(s, a, EntranceTable(s, 0x3acc, 0x23dc), &animation))
                return false;
            if (animation == 0)
            {
                a->vy = 0;
                SetStep(a, 5);
            }
            if (!Water(s, a, 11, &air))
                return false;
            if (a->merman.underwater)
                a->merman.ignore_collision = true;
        }
        break;
    case 8:
        if (!second)
        {
        falling:
            if (a->substep == 0)
            {
                a->state.flags |= 0xc0000000U;
                ++a->substep;
            }
            Move(a);
            a->vy += 16384;
            if (!Water(s, a, second ? 27 : 21, &air))
                return false;
            if (!air && !a->merman.underwater)
            {
                a->merman.ignore_collision = false;
                SetStep(a, 5);
            }
            break;
        }
        if (a->substep == 0)
        {
            a->state.flags |= 0xc0000000U;
            a->state.frame = 14;
            a->vx = Side(a, s->player) ? 131072 : -131072;
            a->vy = -65536;
            a->merman.palette = (Uint16)(0x292 + (a->state.parameters & 1));
            a->merman.timer = 0;
            ++a->substep;
        }
        else if (a->substep == 1)
        {
            if (!ActorGround(s, a, EntranceTable(s, 0x3948, 0x2258), &collision))
                return false;
            a->vy -= 12288;
            if (s->ticks % 10 == 0)
            {
                a->merman.palette += 2;
                if (a->merman.palette > 0x29f)
                    ++a->substep;
                else if (!Spawn(s, a, 0x3f, a->merman.palette, 0, 2, 160, 192))
                    return false;
            }
            if (!Water(s, a, 27, &air))
                return false;
        }
        else
        {
            if (!ActorSound(s, 0x65b) || !Spawn(s, a, 0x62, 1, 0, 0, 160, 192))
                return false;
            a->remove = true;
        }
        break;
    case 9:
        if (a->substep == 0)
        {
            a->merman.palette = 0x2bc;
            a->vy = 0;
            ++a->substep;
        }
        if (a->substep == 1)
        {
            if (!ActorGround(s, a, EntranceTable(s, 0x3a6c, 0x237c), &collision))
                return false;
            if (collision & 1)
            {
                a->vx = Side(a, s->player) ? 32768 : -32768;
                ++a->substep;
            }
        }
        if (!ActorAnimate(s, a, EntranceTable(s, 0x3adc, 0x23ec), &animation))
            return false;
        Move(a);
        a->state.palette = a->merman.palette;
        if (s->ticks % 8 == 0)
        {
            ++a->merman.palette;
            if (a->merman.palette == 0x2c0 &&
                (!ActorSound(s, 0x65b) || !Spawn(s, a, 0x44, 2, 0, 0, 160, 192)))
                return false;
        }
        if (a->merman.palette > 0x2c4)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Merman variant %u entered invalid state %u", second, a->state.step);
    }
    return true;
}

static bool Projectile(SotnActors *s, Actor *a)
{
    const bool medium = a->state.id == EntranceActorId(s, 0x3c);
    if (a->state.step == 0)
    {
        if (!ActorInitialize(
                s, a, medium ? EntranceTable(s, 0xb54, 0xac0) : EntranceTable(s, 0xb6c, 0xad8)))
            return false;
        a->state.frame = 0;
        a->vx = (medium ? 131072 : 98304) * (a->state.facing_left ? 1 : -1);
        if (!medium)
        {
            a->state.hitbox_width = 6;
            a->state.hitbox_height = 3;
            a->state.scale_x = a->state.scale_y = 128;
        }
        return true;
    }
    Uint8 animation = 0;
    if (!ActorAnimate(s, a,
                      medium ? EntranceTable(s, 0x3994, 0x22a4) : EntranceTable(s, 0x3af0, 0x2400),
                      &animation))
        return false;
    Move(a);
    if (!medium && a->state.scale_x < 256)
        a->state.scale_y = a->state.scale_x += 8;
    if (a->state.flags & 0x100)
    {
        if (!ActorExplosion(s, a, 0))
            return false;
        a->remove = true;
    }
    return true;
}

static bool Spawner(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xad0, 0xa60)))
            return false;
        a->state.flags |= 0x2000;
    }
    if (s->ticks % 64 == 0 && Pixel(s->player.y) >= 256)
    {
        const Sint32 x = Pixel(s->player.x) + (Sint32)ActorRandom(s) - 128;
        if (x < 64 || x > 704)
            return true;
        Actor *child = NULL;
        if (!ActorSpawn(s, a, EntranceActorId(s, 0x41), 0, 96, 106, &child))
            return false;
        if (child != NULL)
        {
            if (ActorRandom(s) & 1)
            {
                child->state.id = EntranceActorId(s, 0x3a);
                child->state.parameters = (Uint16)(ActorRandom(s) & 1);
            }
            child->state.x = x * 65536;
            child->state.y = 496 * 65536;
        }
    }
    return true;
}

static bool Splash(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
        return ActorInitialize(s, a, EntranceTable(s, 0xadc, 0xa6c));
    if (a->state.step == 1)
    {
        if (!ActorPrimitives(s, a, 4))
            return false;
        for (Uint16 i = 0; i < 4; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            SotnDataView position = {0}, velocity = {0};
            if (p == NULL ||
                !Slice(s->nz0, EntranceTable(s, 0x39e8, 0x22f8) + i * 2U, 2, &position) ||
                !Slice(s->nz0, EntranceTable(s, 0x39c0, 0x22d0) + i * 8U, 8, &velocity))
                return false;
            p->state = (SotnActorPrimitive){.x = a->state.x + Signed16(position.bytes) * 65536,
                                            .y = a->state.y - 12 * 65536,
                                            .width = 2,
                                            .height = 2,
                                            .red = 64,
                                            .green = 64,
                                            .blue = 160,
                                            .blend = true,
                                            .active = true};
            p->vx = Signed32(velocity.bytes);
            p->vy = Signed32(velocity.bytes + 4);
            p->phase = (Uint16)(i & 1);
        }
        for (Uint16 i = 0; i < 7; ++i)
        {
            Actor *child = NULL;
            SotnDataView parameter = {0}, x = {0}, velocity = {0};
            if (!Slice(s->nz0, EntranceTable(s, 0x39f4, 0x2304) + i, 1, &parameter) ||
                !Slice(s->nz0, EntranceTable(s, 0x39fc, 0x230c) + i * 2U, 2, &x) ||
                !Slice(s->nz0, EntranceTable(s, 0x3a0c, 0x231c) + i * 4U, 4, &velocity) ||
                !ActorSpawn(s, a, EntranceActorId(s, 0x3d), parameter.bytes[0], 168, 192, &child))
                return false;
            if (child != NULL)
            {
                child->state.x += Signed16(x.bytes) * 65536;
                child->vx = Signed32(velocity.bytes);
                child->state.priority = (Uint16)(a->state.priority + 1);
            }
        }
        ++a->state.step;
        return true;
    }
    bool active = false;
    for (Uint16 i = 0; i < 4; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        if (!p->state.active)
            continue;
        active = true;
        p->state.x += p->vx;
        p->state.y += p->vy;
        p->vy += 8192;
        if (p->vy > 0 && p->phase == 1)
        {
            p->vx += p->state.x > a->state.x ? 16384 : -16384;
            p->phase = 0;
        }
        if ((Uint32)(Pixel(p->state.y) - s->camera_y) >= 256)
            p->state.active = false;
    }
    if (!active)
        a->remove = true;
    return true;
}

static bool Effect(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    const Uint16 id = (Uint16)(a->state.id + (s->stage_id == 7 ? 8 : 0));
    if (id == 0x3b)
        return Splash(s, a);
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a,
                             id == 0x3d || id == 0x44 ? EntranceTable(s, 0xac4, 0xa54)
                             : id == 0x43             ? EntranceTable(s, 0xb60, 0xacc)
                                                      : EntranceTable(s, 0xb48, 0xab4)))
            return false;
        a->state.hitbox_state = 0;
        if (id == 0x40 || id == 0x43)
        {
            a->state.priority = (Uint16)(a->state.priority + 4);
            a->state.frame = 0;
            a->state.flags |= 0x2000;
        }
        else if (id == 0x3f)
        {
            a->state.palette = (Uint16)(a->state.parameters + 14);
            a->state.frame = 13;
            a->timer = 32;
            a->vy = 4096;
            a->state.opacity = 128;
            a->state.flags |= 0x2000;
            return true;
        }
        else if (id == 0x44)
        {
            SotnDataView frame = {0}, velocity = {0};
            if (a->state.parameters >= 4 ||
                !Slice(s->nz0, EntranceTable(s, 0x3b44, 0x2454) + a->state.parameters, 1, &frame) ||
                !Slice(s->nz0, EntranceTable(s, 0x3b30, 0x2440) + a->state.parameters * 4U, 4,
                       &velocity))
                return SDL_SetError("Merman explosion references invalid private variant");
            a->state.animation_set = 2;
            a->state.palette = 0x82bb;
            a->state.frame = frame.bytes[0];
            a->vy = Signed32(velocity.bytes);
            ++a->state.step;
            return true;
        }
        else if (id == 0x3d)
        {
            if (a->state.parameters == 0 || a->state.parameters > 16)
                return SDL_SetError("High water splash delay is outside original variants");
            a->state.animation_set = 2;
            a->vy = -5 * 65536;
            a->state.palette = 0x8018;
            a->state.blend_mode = 0x30;
            a->state.opacity = 160;
            a->state.scale_x = 256;
            a->state.scale_y = 416;
            a->timer = a->state.parameters;
            a->merman.timer = 17;
            return true;
        }
        else if (id == 0x3e)
        {
            a->state.priority = (Uint16)(a->state.priority + 4);
            a->state.frame = 0;
            if (a->state.parameters)
            {
                if (!ActorPrimitives(s, a, 1))
                    return false;
                a->merman.timer = 96;
            }
        }
    }
    if (id == 0x3f)
    {
        Move(a);
        a->vy += 4096;
        a->state.opacity -= 2;
        if (a->timer == 0)
            return SDL_SetError("Merman death afterimage timer underflow");
        if (--a->timer == 0)
            a->remove = true;
        return true;
    }
    if (id == 0x44)
    {
        SotnDataView limit = {0};
        if (!Slice(s->nz0, EntranceTable(s, 0x3b48, 0x2458) + a->state.parameters * 2U, 2, &limit))
            return false;
        a->state.y += a->vy;
        if (++a->timer % 2 == 0)
            ++a->state.frame;
        if (a->timer > Read16(limit.bytes))
            a->remove = true;
        return true;
    }
    if (id == 0x3d)
    {
        if (a->state.step == 1)
        {
            --a->merman.timer;
            if (a->timer == 0)
                return SDL_SetError("High water splash delay underflow");
            if (--a->timer == 0)
                ++a->state.step;
            return true;
        }
        if (!ActorAnimate(s, a, EntranceTable(s, 0x39b0, 0x22c0), &animation))
            return false;
        if (a->state.step == 2 || animation == 0)
        {
            Move(a);
            a->vy += 16384;
            a->state.scale_x += 6;
            a->state.scale_y -= 4;
        }
        if (a->state.step == 2 && --a->merman.timer == 0)
        {
            a->vy = 0;
            ++a->state.step;
        }
        if (Pixel(a->state.y) - s->camera_y > 256)
            a->remove = true;
        return true;
    }
    if (id == 0x3e)
    {
        if (a->state.parameters)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
            if (p == NULL || !ActorSprite(s, a, 0, a->state.x - 32 * 65536, a->state.y - 32 * 65536,
                                          0, 0, 64, 64, 0x1a, 0x1ff))
                return false;
            if (a->merman.timer >= 4)
                a->merman.timer -= 4;
            p->state.active = a->merman.timer >= 16;
            p->state.modulate = true;
            p->state.red = p->state.green = p->state.blue = (Uint8)a->merman.timer;
        }
        if (++a->timer % 4 == 0)
            ShiftY(a, 1);
    }
    else
    {
        Move(a);
        a->vy += 10240;
    }
    if (!ActorAnimate(s, a,
                      id == 0x3e   ? EntranceTable(s, 0x39a0, 0x22b0)
                      : id == 0x40 ? EntranceTable(s, 0x39a8, 0x22b8)
                                   : EntranceTable(s, 0x3b04, 0x2414),
                      &animation))
        return false;
    if (animation == 0)
        a->remove = true;
    return true;
}

bool ActorMerman(SotnActors *s, Actor *a)
{
    switch (a->state.id + (s->stage_id == 7 ? 8 : 0))
    {
    case 0x3a:
        return Merman(s, a, true);
    case 0x41:
        return Merman(s, a, false);
    case 0x3c:
    case 0x42:
        return Projectile(s, a);
    case 0x45:
        return Spawner(s, a);
    case 0x3b:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
    case 0x43:
    case 0x44:
        return Effect(s, a);
    default:
        return SDL_SetError("Merman dispatcher received unrelated actor %u", a->state.id);
    }
}
