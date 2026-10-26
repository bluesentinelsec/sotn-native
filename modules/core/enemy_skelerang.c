// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_skelerang.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original private NO0/DAI/NO2 animation, position, sine and sensor tables remain external.
#include "actors_internal.h"
static Uint16 Body(const SotnActors *s)
{
    return s->stage_id == 6 ? 0x3a : s->stage_id == 4 ? 0x16 : 0x2e;
}
static size_t Table(const SotnActors *s, size_t no0)
{
    if (s->stage_id == 0)
        return no0;
    if (s->stage_id == 4)
    {
        if (no0 >= 0x1e94 && no0 < 0x1f80)
            return no0 - 0x45c;
        switch (no0)
        {
        case 0xb6c:
            return 0x88c;
        case 0xb78:
            return 0x898;
        case 0xa94:
            return 0x850;
        default:
            SDL_SetError("NO2 Skelerang has no shared table at %zu", no0);
            return s->nz0.size;
        }
    }
    if (no0 >= 0x1e94 && no0 < 0x1f80)
        return no0 + 0x19c;
    switch (no0)
    {
    case 0xb6c:
        return 0xa7c;
    case 0xb78:
        return 0xa88;
    case 0xa94:
        return 0x968;
    default:
        SDL_SetError("Chapel Skelerang has no selected shared table at %zu", no0);
        return s->nz0.size;
    }
}
static bool Sound(SotnActors *s, const Actor *a, Uint16 id)
{
    return s->stage_id != 0 ? ActorPositionalSound(s, a, id) : ActorSound(s, id);
}
static bool Velocity(SotnActors *s, Actor *a)
{
    if (s->stage_id == 6 || s->stage_id == 4)
    {
        const size_t sine = s->stage_id == 6 ? 0x1474U : 0x142cU;
        SotnDataView x = {0}, y = {0};
        const Uint8 angle = (Uint8)a->table.angle;
        if (!Slice(s->nz0, sine + angle * 2U, 2, &x) ||
            !Slice(s->nz0, sine + (Uint8)(angle - 64) * 2U, 2, &y))
            return false;
        const Sint32 sx = Signed16(x.bytes), sy = Signed16(y.bytes);
        if (sx < -4096 || sx > 4096 || sy < -4096 || sy > 4096)
            return SDL_SetError("Chapel Skelerang sine profile exceeds original amplitude");
        a->vx = sx * 48;
        a->vy = sy * 48;
        return true;
    }
    a->vx = ActorSin(a->table.angle * 16) * 48;
    a->vy = ActorSin(a->table.angle * 16 - 1024) * 48;
    return true;
}
static bool PrepareChildren(SotnActors *s, const Actor *a)
{
    for (Uint32 i = 1; i <= 3; ++i)
    {
        const Actor *old = &s->slots[a->slot + i];
        if (!old->state.generation)
            continue;
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, a->slot + i, &placement, &placed))
            return false;
        const Uint16 id = (Uint16)(Body(s) + (i == 1 ? 2U : 1U));
        if (placed || old->state.id != id || old->enemy_state.owner_slot != a->slot)
            return SDL_SetError("Skelerang reserved child slot is occupied by another owner");
    }
    for (Uint32 i = 1; i <= 3; ++i)
    {
        Actor *old = &s->slots[a->slot + i];
        ActorPrimitivesClear(s, old);
        *old = (Actor){0};
    }
    return true;
}
static Actor *Owner(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[slot];
    return p->state.id == Body(s) && p->state.generation == a->enemy_state.owner_generation ? p
                                                                                            : NULL;
}
static bool Boomerang(SotnActors *s, Actor *a)
{
    Actor *p = Owner(s, a);
    if (a->state.parameters > 1)
        return SDL_SetError("Skelerang boomerang hand exceeds its original range");
    if (p == NULL)
        a->state.step = 4;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, Table(s, 0xb78)))
            return false;
        a->state.rotated = true;
        a->state.frame = a->state.hitbox_state = 0;
        break;
    case 1:
        a->state.x = p->state.x;
        a->state.y = p->state.y;
        a->state.frame = a->state.hitbox_state = a->substep = 0;
        a->table.timer = 48;
        a->table.angle = a->state.parameters ? 0 : 128;
        break;
    case 2: {
        a->state.hitbox_state = 1;
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->state.rotation = (Sint16)((a->state.rotation + 256) & 4095);
        if (a->table.timer == 0)
            a->substep = 1;
        else
            --a->table.timer;
        Actor *target = a->substep ? p : &s->slots[p->slot + 1];
        const Sint32 dx = Pixel(target->state.x) - Pixel(a->state.x),
                     dy = Pixel(target->state.y) - Pixel(a->state.y);
        Sint16 direction = 0;
        if (!SotnMath_Atan2(s->executable, dy, dx, &direction))
            return false;
        const Uint8 goal = (Uint8)(ActorShift(direction, 4) + 64);
        const Uint8 current = (Uint8)a->table.angle;
        const Uint8 bits = (Uint8)(goal - current);
        const Sint32 delta = bits < 128 ? bits : (Sint32)bits - 256;
        const Sint32 threshold = a->substep + 3;
        a->table.angle = SDL_abs(delta) > threshold
                             ? (Uint8)(current + (delta < 0 ? -threshold : threshold))
                             : goal;
        if (!Velocity(s, a))
            return false;
        if (SDL_abs(dx) < 6 && SDL_abs(dy) < 6)
        {
            if (!Sound(s, a, 0x6bc))
                return false;
            ++a->substep;
        }
        if (a->substep == 2)
            ++a->state.step;
        break;
    }
    case 3:
        a->state.hitbox_state = 0;
        a->state.rotation = 512;
        break;
    case 4:
        if (!ActorExplosionInPool(s, a, 1, 160, 192, 0))
            return false;
        a->remove = true;
        break;
    default:
        return SDL_SetError("Skelerang boomerang entered invalid state %u", a->state.step);
    }
    return true;
}
static bool DeathParts(SotnActors *s, Actor *a)
{
    a->state.x += a->vx;
    a->state.y += a->vy;
    a->vy += 12288;
    if (!a->table.timer || a->state.parameters >= 8)
        return SDL_SetError("Skelerang death part has invalid lifetime or index");
    if (--a->table.timer == 0)
    {
        SotnDataView offset = {0};
        if (!Slice(s->nz0, Table(s, 0x1ea4) + a->state.parameters * 4U, 4, &offset))
            return false;
        const Sint32 x = Signed16(offset.bytes), y = Signed16(offset.bytes + 2);
        if (SDL_abs(x) > 128 || SDL_abs(y) > 128)
            return SDL_SetError("Skelerang death offset exceeds its bounded private positions");
        Actor origin = *a;
        origin.state.x += x * (a->state.facing_left ? -1 : 1) * 65536;
        ShiftY(&origin, y);
        if (!ActorExplosionInPool(s, &origin, 0, 160, 192, 0))
            return false;
        a->remove = true;
    }
    return true;
}
bool ActorSkelerang(SotnActors *s, Actor *a)
{
    if (!s || !a || (s->stage_id != 0 && s->stage_id != 4 && s->stage_id != 6) ||
        a->state.id < Body(s) || a->state.id > Body(s) + 2U)
        return SDL_SetError("Skelerang requires its original stage identity");
    if (a->state.id == Body(s) + 1U)
        return Boomerang(s, a);
    if (a->state.id == Body(s) + 2U)
    {
        if (!a->state.step && !ActorInitialize(s, a, Table(s, 0xa94)))
            return false;
        a->remove = Owner(s, a) == NULL;
        return true;
    }
    if (a->state.step == 14)
        return DeathParts(s, a);
    if ((a->state.step & 1) && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 32 &&
        Distance(a, s->player) < 80)
        SetStep(a, 8);
    if ((a->state.flags & 0x100) && a->state.step < 10)
    {
        a->state.hitbox_state = 0;
        if (!Sound(s, a, 0x62b))
            return false;
        for (Uint32 i = 1; i <= 3; ++i)
        {
            if ((Uint32)a->slot + i >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Skelerang death has an invalid reserved child range");
            Actor *child = &s->slots[a->slot + i];
            if (child->enemy_state.owner_generation != a->state.generation)
                continue;
            if (i == 1)
                child->remove = true;
            else
                child->state.step = 4;
        }
        SetStep(a, a->state.frame > 39 ? 10 : 12);
    }
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0:
        if (!a->state.generation || a->state.parameters > 1 ||
            (Uint32)a->slot + 3 >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Skelerang requires an original facing and three reserved slots");
        if (!PrepareChildren(s, a))
            return false;
        if (!ActorInitialize(s, a, Table(s, 0xb6c)))
            return false;
        a->state.facing_left = a->state.parameters != 0;
        for (Uint16 i = 1; i <= 3; ++i)
        {
            Actor *child = NULL;
            if (!ActorSpawnAt(s, a, (Uint16)(Body(s) + (i == 1 ? 2U : 1U)), i == 3 ? 1 : 0,
                              a->slot + i, &child))
                return false;
        }
        break;
    case 1:
        if (!ActorGround(s, a, Table(s, 0x1e94), &collision))
            return false;
        if (collision & 1)
            SetStep(a, 3);
        break;
    case 3:
        a->state.frame = 1;
        if (!((Pixel(a->state.y) - s->camera_y) & 256) && Distance(a, s->player) < 128 &&
            (Side(a, s->player) == 0) == a->state.facing_left)
            SetStep(a, 5);
        break;
    case 4:
        if (!ActorAnimate(s, a, Table(s, 0x1f00), &animation))
            return false;
        if (!animation)
            SetStep(a, 6);
        if (a->state.frame == 30)
        {
            if (!Sound(s, a, 0x69e))
                return false;
            Actor *target = &s->slots[a->slot + 1];
            target->state.x = s->player.x;
            target->state.y = s->player.y;
            for (Uint32 i = 2; i <= 3; ++i)
            {
                Actor *child = &s->slots[a->slot + i];
                if (child->state.id != Body(s) + 1U ||
                    child->enemy_state.owner_generation != a->state.generation)
                    return SDL_SetError("Skelerang lost its original boomerang child");
                child->state.x = a->state.x;
                child->state.y = a->state.y;
                child->state.frame = 43;
                ++child->state.step;
            }
        }
        break;
    case 5:
        if (!ActorAnimate(s, a, Table(s, 0x1ec8), &animation))
            return false;
        if (!animation)
            SetStep(a, 4);
        break;
    case 6:
        if (s->slots[a->slot + 2].state.step == 3 && s->slots[a->slot + 3].state.step == 3)
        {
            s->slots[a->slot + 2].state.step = s->slots[a->slot + 3].state.step = 1;
            SetStep(a, 7);
        }
        break;
    case 7:
        if (!ActorAnimate(s, a, Table(s, 0x1f34), &animation))
            return false;
        if (!animation)
            SetStep(a, ++a->table.timer > 2 ? 9 : 4);
        break;
    case 8: {
        const size_t stream = a->substep == 0 ? 0x1f38 : a->substep == 1 ? 0x1f60 : 0x1f4c;
        if (a->substep > 2 || !ActorAnimate(s, a, Table(s, stream), &animation))
            return SDL_SetError("Skelerang cower animation or substate is invalid");
        if (a->substep == 1 ? Distance(a, s->player) > 80 ||
                                  SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) > 48
                            : animation == 0)
        {
            a->pose = a->remaining = 0;
            if (++a->substep == 3)
                SetStep(a, 9);
        }
        break;
    }
    case 9:
        if (!a->substep)
        {
            a->table.timer = 0;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, Table(s, 0x1edc), &animation))
            return false;
        if (!animation)
            ++a->table.timer;
        if (a->table.timer == 3)
        {
            a->table.timer = 0;
            SetStep(a, Distance(a, s->player) < 144 &&
                               SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 144 &&
                               (Side(a, s->player) == 0) == a->state.facing_left
                           ? 4
                           : 3);
        }
        break;
    case 10: {
        Actor origin = *a;
        ShiftY(&origin, 24);
        if (!ActorExplosionInPool(s, &origin, 2, 160, 192, 0))
            return false;
        a->remove = true;
        break;
    }
    case 12:
        for (Uint16 i = 0; i < 8; ++i)
        {
            Actor *part = NULL;
            if (!ActorSpawn(s, a, Body(s), i, 160, 192, &part))
                return false;
            if (part == NULL)
                break;
            part->state.x = Pixel(a->state.x) * 65536;
            part->state.y = Pixel(a->state.y) * 65536;
            part->state.animation_set = a->state.animation_set;
            part->state.palette = a->state.palette;
            part->state.texture_offset = a->state.texture_offset;
            part->state.priority = a->state.priority;
            part->state.flags = 0x88002000;
            part->state.frame = (Uint16)(44 + i);
            part->vx = (Sint32)(ActorRandom(s) & 3) * (Side(a, s->player) == 0 ? -65536 : 65536);
            part->vy = (-2 - (Sint32)(ActorRandom(s) & 3)) * 65536;
            part->table.timer = 16;
            part->state.step = 14;
        }
        if (!ActorExplosionInPool(s, a, 2, 160, 192, 0))
            return false;
        a->remove = true;
        break;
    default:
        return SDL_SetError("Skelerang entered invalid state %u", a->state.step);
    }
    a->state.hitbox_x = a->state.frame > 39 ? -1 : 0;
    a->state.hitbox_y = a->state.frame > 39 ? 16 : 2;
    a->state.hitbox_width = a->state.frame > 39 ? 8 : 5;
    a->state.hitbox_height = a->state.frame > 39 ? 8 : 21;
    return true;
}
