// SPDX-License-Identifier: AGPL-3.0-or-later
// Reviewed Gaibon coordination/attacks from nz0/gaibon.c, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables remain private runtime input.
#include "actors_internal.h"
static void Substep(Actor *a, Uint16 step)
{
    a->substep = step;
    a->pose = a->remaining = 0;
}
static void Move(Actor *a)
{
    a->state.x += a->vx;
    a->state.y += a->vy;
}
static bool Flap(SotnActors *s, Actor *a, size_t stream, Uint8 *animation)
{
    return ActorAnimate(s, a, stream, animation) &&
           (!(a->remaining == 0 && a->pose == 1) || ActorSound(s, 0x68d));
}
static void Velocity(Actor *a, Sint16 angle, Sint32 speed)
{
    a->vx = ActorShift((Sint64)speed * ActorSin(angle + 1024), 12);
    a->vy = ActorShift((Sint64)speed * ActorSin(angle), 12);
}
static bool Fire(SotnActors *s, Actor *a, bool big, bool diagonal)
{
    Actor *shot = NULL;
    if (!ActorSpawn(s, a, big ? 0x46 : 0x45, 0, 96, 128, &shot))
        return false;
    if (shot != NULL)
    {
        if (!ActorSound(s, big ? 0x655 : 0x652))
            return false;
        ShiftY(shot, diagonal ? -2 : -6);
        shot->state.x += (a->state.facing_left ? 1 : -1) * (diagonal ? 12 : 16) * 65536;
        shot->state.rotation =
            diagonal ? (a->state.facing_left ? 0x220 : 0x5e0) : (a->state.facing_left ? 0 : 0x800);
        shot->state.priority = (Uint16)(a->state.priority + 1);
    }
    return true;
}
static bool Flying(SotnActors *s, Actor *a, bool fire)
{
    Uint8 animation = 0;
    if (a->substep == 0)
    {
        a->state.facing_left = (Side(a, s->player) ^ 1) != 0;
        const Sint32 dx =
            Pixel(s->player.x) - Pixel(a->state.x) + (fire ? (Side(a, s->player) ? 96 : -96) : 0);
        const Sint32 dy = Pixel(s->player.y) - Pixel(a->state.y) - (fire ? 128 : 32);
        if (!SotnMath_Atan2(s->executable, dy, dx, &a->boss.angle))
            return false;
        a->boss.speed = 0;
        a->boss.timer = (Uint16)(fire ? 80 : 96);
        if (a->boss.near_death)
            a->boss.timer /= 2;
        a->substep = 1;
    }
    if (a->substep == 1)
    {
        a->boss.speed = SDL_min(a->boss.speed + 2560, (a->boss.near_death ? 4 : 2) * 65536);
        Velocity(a, a->boss.angle, a->boss.speed);
        Move(a);
        if (!Flap(s, a, fire ? 0x1298 : 0x1250, &animation))
            return false;
        if (!fire)
            a->state.facing_left = (Side(a, s->player) ^ 1) != 0;
        if (fire && (a->boss.timer & (a->boss.near_death ? 7 : 15)) == 0 &&
            !Fire(s, a, false, true))
            return false;
        if (--a->boss.timer == 0)
            ++a->substep;
    }
    else if (a->substep == 2)
    {
        Move(a);
        a->vx -= a->vx / 32;
        a->vy -= a->vy / 32;
        if (!ActorAnimate(s, a, 0x1264, &animation))
            return false;
        if (animation == 0)
            SetStep(a, fire ? (a->boss.near_death ? 8 : 5) : 4);
        if (a->remaining == 0 && a->pose == 1 && !ActorSound(s, 0x68d))
            return false;
    }
    else
        return SDL_SetError("Gaibon flight substep is invalid");
    return true;
}
static bool Landing(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    SotnCollider c = {0};
    if (a->substep == 0)
    {
        a->state.frame = 9;
        a->vx = a->vy = 0;
        a->substep = 1;
    }
    switch (a->substep)
    {
    case 1:
        Move(a);
        a->vy += 6144;
        if (Pixel(s->player.y) - Pixel(a->state.y) < 48)
            ++a->substep;
        break;
    case 2:
        Move(a);
        a->vy += 6144;
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + 28, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            ShiftY(a, c.top);
            ++a->substep;
        }
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x12c0, &animation))
            return false;
        if (animation == 0)
            SetStep(a, 6);
        break;
    default:
        return SDL_SetError("Gaibon landing substep is invalid");
    }
    return true;
}
static bool GroundFire(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (a->substep == 0)
    {
        a->state.facing_left = (Side(a, s->player) ^ 1) != 0;
        a->substep = 1;
    }
    if (a->substep == 1)
    {
        if (!ActorAnimate(s, a, 0x12cc, &animation))
            return false;
        if (animation == 0)
        {
            a->boss.timer = a->boss.near_death ? 128 : 64;
            ++a->substep;
        }
    }
    else if (a->substep == 2)
    {
        if ((a->boss.timer & 15) == 0 && !Fire(s, a, a->boss.near_death, false))
            return false;
        if (--a->boss.timer == 0)
            SetStep(a, 3);
    }
    else
        return SDL_SetError("Gaibon grounded fire substep is invalid");
    return true;
}
static bool Combo(SotnActors *s, Actor *a)
{
    Actor *slogra = &s->slots[16];
    Uint8 animation = 0;
    if (slogra->state.id != 0x40 || slogra->state.generation == 0)
    {
        a->boss.ascending = false;
        SetStep(a, 4);
        return true;
    }
    if (a->substep == 0)
    {
        a->state.facing_left = Pixel(slogra->state.x) > Pixel(a->state.x);
        a->boss.speed = 0;
        a->substep = 1;
    }
    switch (a->substep)
    {
    case 1: {
        const Sint32 dx = Pixel(slogra->state.x) - Pixel(a->state.x),
                     dy = Pixel(slogra->state.y) - Pixel(a->state.y) - 28;
        Sint16 angle = 0;
        if (!SotnMath_Atan2(s->executable, dy, dx, &angle))
            return false;
        a->boss.speed = SDL_min(a->boss.speed + 32768, 229376);
        Velocity(a, angle, a->boss.speed);
        Move(a);
        if (SDL_abs(dx) < 8 && SDL_abs(dy) < 8)
        {
            a->boss.ascending = true;
            a->vx = a->vy = 0;
            ++a->substep;
        }
        else if (!slogra->boss.pickup)
        {
            a->boss.ascending = false;
            SetStep(a, 4);
        }
        break;
    }
    case 2:
        if (!Flap(s, a, 0x12ac, &animation))
            return false;
        Move(a);
        a->vy = SDL_max(a->vy - 2560, -2 * 65536);
        slogra->state.x = Pixel(a->state.x) * 65536 + (Sint32)((Uint32)slogra->state.x & 65535);
        ShiftY(slogra, Pixel(a->state.y) + 28 - Pixel(slogra->state.y));
        a->boss.ascending = true;
        if (Pixel(a->state.y) - s->camera_y < 16)
        {
            a->vx = a->vy = 0;
            a->boss.timer = 96;
            ++a->substep;
        }
        break;
    case 3:
        if (!Flap(s, a, 0x12ac, &animation))
            return false;
        a->vx = SDL_clamp(a->vx + (Side(a, s->player) ? -2560 : 2560), -2 * 65536, 2 * 65536);
        Move(a);
        slogra->state.x = Pixel(a->state.x) * 65536 + (Sint32)((Uint32)slogra->state.x & 65535);
        ShiftY(slogra, Pixel(a->state.y) + 28 - Pixel(slogra->state.y));
        slogra->vy = 0;
        a->boss.ascending = false;
        if (--a->boss.timer == 0)
            ++a->substep;
        break;
    case 4:
        a->boss.ascending = false;
        SetStep(a, 3);
        break;
    default:
        return SDL_SetError("Gaibon pickup substep is invalid");
    }
    return true;
}
static bool NearDeath(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    Uint16 collision = 0;
    SotnDataView init = {0};
    if (a->substep == 0)
    {
        a->state.frame = 9;
        a->vx = a->vy = 0;
        a->substep = 1;
    }
    switch (a->substep)
    {
    case 1:
        if (!ActorGround(s, a, 0x1240, &collision))
            return false;
        if (collision & 1)
        {
            if (!ActorSound(s, 0x70b))
                return false;
            Substep(a, 2);
        }
        break;
    case 2:
        if (!ActorAnimate(s, a, 0x12f0, &animation))
            return false;
        if (animation == 0)
        {
            a->boss.flag = 0;
            Substep(a, 3);
            if (a->state.flags & 0x100)
            {
                if (!ActorSound(s, 0x672))
                    return false;
                SetStep(a, 16);
            }
            else if (!ActorSound(s, 0x70b))
                return false;
        }
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x12fc, &animation))
            return false;
        if (animation == 0)
        {
            if (!Slice(s->nz0, 0xd36, 2, &init))
                return false;
            ++a->boss.flag;
            a->state.palette = (Uint16)(Read16(init.bytes) + a->boss.flag);
            if (a->boss.flag == 6)
            {
                a->state.flags &= ~15U;
                SetStep(a, 4);
            }
        }
        break;
    default:
        return SDL_SetError("Gaibon transformation substep is invalid");
    }
    return true;
}
static bool Dying(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    SotnDataView palette = {0};
    if (a->substep == 0)
    {
        if (!ActorAnimate(s, a, 0x1304, &animation))
            return false;
        if (animation == 0)
        {
            a->boss.timer = 96;
            a->state.frame = 31;
            a->state.flags &= ~15U;
            if (!Slice(s->nz0, 0xd36, 2, &palette) || !ActorSound(s, 0x65e))
                return false;
            a->state.palette = Read16(palette.bytes);
            ++a->substep;
        }
    }
    else if (a->substep == 1)
    {
        if ((a->boss.timer & 7) == 0)
        {
            Actor *puff = NULL;
            if (!ActorSound(s, 0x691) || !ActorSpawn(s, a, 0x38, 0, 160, 192, &puff))
                return false;
            if (puff != NULL)
            {
                ShiftY(puff, 28);
                puff->state.x += ((Sint32)(ActorRandom(s) & 63) - 32) * 65536;
                puff->state.priority = (Uint16)(a->state.priority + 1);
            }
        }
        if (--a->boss.timer == 0)
        {
            s->boss_flags |= 8;
            a->remove = true;
        }
    }
    else
        return SDL_SetError("Gaibon death substep is invalid");
    return true;
}
static bool Main(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (a->state.step && !a->boss.near_death && a->state.hit_points < a->boss.maximum / 2)
    {
        a->boss.ascending = false;
        a->boss.near_death = true;
        SetStep(a, 15);
    }
    if ((a->state.flags & 0x100) && a->state.step < 15)
    {
        a->boss.ascending = false;
        a->state.hitbox_state = 0;
        SetStep(a, 15);
    }
    else if (s->slots[16].state.id == 0x40 && s->slots[16].boss.pickup && a->state.step < 5)
        SetStep(a, 9);
    switch (a->state.step)
    {
    case 0: {
        Actor *leg = NULL;
        if (a->slot != 24 || !ActorInitialize(s, a, 0xd30) ||
            !ActorSpawnAt(s, a, 0x44, 0, (Uint32)a->slot + 1, &leg))
            return false;
        a->boss.maximum = (Uint16)SDL_max(0, a->state.hit_points);
        if (leg != NULL)
        {
            leg->state.priority = (Uint16)(a->state.priority + 4);
            leg->enemy_state.owner_slot = a->slot;
            leg->enemy_state.owner_generation = a->state.generation;
        }
        SetStep(a, 2);
        break;
    }
    case 2:
        if (!Flap(s, a, 0x1250, &animation))
            return false;
        if (a->hit_flags)
            s->boss_flags |= 4;
        if (s->boss_flags & 4)
            SetStep(a, 3);
        break;
    case 3:
    case 4:
        if (!Flying(s, a, a->state.step == 4))
            return false;
        break;
    case 5:
        if (!Landing(s, a))
            return false;
        break;
    case 6:
        if (!GroundFire(s, a))
            return false;
        break;
    case 8:
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, 0x12dc, &animation))
                return false;
            if (animation == 0)
                ++a->substep;
            break;
        }
        if (a->substep == 1)
        {
            if (!Fire(s, a, true, true))
                return false;
            a->vy = -2 * 65536;
            a->vx = a->state.facing_left ? -2 * 65536 : 2 * 65536;
            a->boss.timer = 32;
            ++a->substep;
        }
        if (a->substep != 2)
            return SDL_SetError("Gaibon large shot substep is invalid");
        Move(a);
        a->vx -= a->vx / 16;
        a->vy -= a->vy / 16;
        if (--a->boss.timer == 0)
            SetStep(a, 5);
        break;
    case 9:
        if (!Combo(s, a))
            return false;
        break;
    case 15:
        if (!NearDeath(s, a))
            return false;
        break;
    case 16:
        if (!Dying(s, a))
            return false;
        break;
    default:
        return SDL_SetError("Gaibon entered an unsupported step");
    }
    if (!(a->state.flags & 0x100))
    {
        if (a->vx > 0 && Pixel(a->state.x) > 784)
            a->state.x = 784 * 65536 + (Sint32)((Uint32)a->state.x & 65535);
        if (a->vx < 0 && Pixel(a->state.x) < 96)
            a->state.x = 96 * 65536 + (Sint32)((Uint32)a->state.x & 65535);
        if (a->vy > 0 && Pixel(a->state.y) > 420)
            ShiftY(a, 420 - Pixel(a->state.y));
        return ActorBossBox(s, a, 0x1310, 12, 0x1340, 44);
    }
    return true;
}
static bool Leg(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xd30))
            return false;
        a->state.hitbox_state = 0;
    }
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Gaibon leg parent slot is invalid");
    const Actor *parent = &s->slots[a->enemy_state.owner_slot];
    if (parent->state.id != 0x43 || parent->state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    a->state.facing_left = parent->state.facing_left;
    a->state.palette = parent->state.palette;
    a->state.x = Pixel(parent->state.x) * 65536 + (Sint32)((Uint32)a->state.x & 65535);
    ShiftY(a, Pixel(parent->state.y) - Pixel(a->state.y));
    a->state.frame = 0;
    if (parent->state.frame > 31 && parent->state.frame < 35)
        a->state.frame = 38;
    else if (parent->state.frame == 35)
        a->state.frame = 39;
    else if (parent->state.frame > 35 && parent->state.frame < 38)
        a->state.frame = 40;
    return true;
}
static bool Projectile(SotnActors *s, Actor *a)
{
    const bool big = a->state.id == 0x46;
    Uint8 animation = 0;
    if (a->state.flags & 0x100)
    {
        a->state.rotated = false;
        a->state.scale_x = a->state.scale_y = a->state.opacity = 0;
        a->state.id = 2;
        a->state.parameters = big ? 1 : 0;
        a->state.step = 0;
        return true;
    }
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, big ? 0xd48 : 0xd3c))
            return false;
        if (!big || a->state.parameters == 0)
        {
            a->state.animation_set = 2;
            a->state.rotated = true;
            if (!big)
            {
                a->state.frame = 1;
                a->state.scale_x = 192;
            }
            Velocity(a, a->state.rotation, big ? 229376 : 163840);
            a->state.rotation = (Sint16)Wrap16(a->state.rotation - 1024);
            a->state.palette = 0x81b6;
        }
        else
        {
            a->state.animation_set = 14;
            a->state.texture_offset = 0x79;
            a->state.rotated = true;
            a->state.scale_x = 256;
            a->state.opacity = 128;
            a->state.palette = 0x81f3;
            a->state.blend_mode = 0x30;
            a->state.step = 2;
            a->state.hitbox_state = 0;
            a->state.flags |= 0x2000;
        }
        if (big)
            return true;
    }
    if (a->state.step == 1)
    {
        Move(a);
        if (!ActorAnimate(s, a, big ? 0x1378 : 0x136c, &animation))
            return false;
        if (big && (s->ticks & 3) == 0)
        {
            Actor *trail = NULL;
            if (!ActorSpawn(s, a, 0x46, 1, 160, 192, &trail))
                return false;
            if (trail != NULL)
            {
                trail->state.rotation = a->state.rotation;
                trail->state.priority = (Uint16)(a->state.priority + 1);
            }
        }
    }
    else if (big && a->state.step == 2)
    {
        if (a->state.opacity < 2 || a->state.scale_x < 4)
            return SDL_SetError(
                "Gaibon trail exhausted its original color/scale before animation expiry");
        a->state.opacity -= 2;
        a->state.scale_x -= 4;
        if (!ActorAnimate(s, a, 0x1388, &animation))
            return false;
        if (animation == 0)
            a->remove = true;
    }
    else
        return SDL_SetError("Gaibon projectile step is invalid");
    return true;
}
bool ActorGaibon(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x43)
        return Main(s, a);
    if (a->state.id == 0x44)
        return Leg(s, a);
    if (a->state.id == 0x45 || a->state.id == 0x46)
        return Projectile(s, a);
    return SDL_SetError("Unknown Gaibon actor");
}
