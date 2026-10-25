// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_spear_guard.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Private NO1 retains animation pointers, movement, attack definitions and guard hitboxes.
#include "actors_internal.h"
static Actor *Owner(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[slot];
    return p->state.id == 0x4f && p->state.generation == a->enemy_state.owner_generation ? p : NULL;
}
static bool Attack(SotnActors *s, Actor *block, Uint16 id)
{
    SotnDataView api = {0}, definition = {0};
    if (id < 94 || id > 95 || !Slice(s->dra, 0x98, 4, &api))
        return SDL_SetError("Spear Guard special attack definition is invalid");
    const Uint32 address = Read32(api.bytes);
    if (address < 0x800a0000 || address % 4 ||
        !Slice(s->dra, address - 0x800a0000 + (size_t)id * 40, 10, &definition))
        return SDL_SetError("Spear Guard attack definition exceeds its private DRA table");
    block->attack = Read16(definition.bytes + 6);
    block->attack_element = Read16(definition.bytes + 8);
    return true;
}
static bool Block(SotnActors *s, Actor *a)
{
    Actor *p = Owner(s, a);
    if (p == NULL || (p->state.flags & 0x100))
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x968))
            return false;
        a->state.hitbox_state = 1;
    }
    if (!(a->hit_flags & 0x80) && a->hit_flags && !ActorSound(s, 0x611))
        return false;
    const Uint32 frame = p->state.frame >= 3 && p->state.frame <= 45 ? p->state.frame - 3U : 0;
    SotnDataView position = {0}, size = {0};
    if (!Slice(s->nz0, 0x307c + frame * 4, 4, &position) ||
        !Slice(s->nz0, 0x3128 + frame * 4, 4, &size))
        return false;
    const Sint32 x = Signed16(position.bytes), y = Signed16(position.bytes + 2);
    const Uint16 width = Read16(size.bytes), height = Read16(size.bytes + 2);
    if (SDL_abs(x) > 128 || SDL_abs(y) > 128 || width > 64 || height > 64)
        return SDL_SetError("Spear Guard private blocking hitbox exceeds its bounds");
    a->state.hitbox_x = (Sint16)x;
    a->state.hitbox_y = (Sint16)y;
    a->state.hitbox_width = width;
    a->state.hitbox_height = height;
    a->state.facing_left = p->state.facing_left;
    a->state.x = Pixel(p->state.x) * 65536;
    a->state.y = Pixel(p->state.y) * 65536;
    return true;
}
static bool Thrown(SotnActors *s, Actor *a)
{
    Actor *p = Owner(s, a);
    if ((a->state.step == 1 || a->state.step == 2) && (p == NULL || (p->state.flags & 0x100)))
    {
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xb0c))
            return false;
        a->state.rotated = true;
        a->state.rotation = 0;
        break;
    case 1:
        ++a->state.step;
        break;
    case 2:
        a->state.x = (Pixel(p->state.x) + (a->state.facing_left ? 40 : -40)) * 65536;
        a->state.y = (Pixel(p->state.y) - 16) * 65536;
        a->vx = a->state.facing_left ? 6 * 65536 : -6 * 65536;
        a->state.frame = 54;
        a->state.hitbox_state = 1;
        ++a->state.step;
        break;
    case 3: {
        a->vy += 1024;
        Sint16 angle = 0;
        if (a->vy == SDL_MIN_SINT32 || !SotnMath_Atan2Vector(s->executable, -a->vy, a->vx, &angle))
            return false;
        a->state.rotation = (Sint16)((a->state.facing_left ? angle : 2048 - angle) & 4095);
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    }
    default:
        return SDL_SetError("Thrown spear entered invalid state %u", a->state.step);
    }
    return true;
}
static bool Move(SotnActors *s, Actor *a, Uint16 animation)
{
    SotnDataView pointer = {0}, frame = {0}, velocity = {0};
    if (animation > 3 || !Slice(s->nz0, 0x3054 + animation * 4U, 4, &pointer) ||
        !Slice(s->nz0, 0x2f74 + animation * 2U, 2, &frame) ||
        !Slice(s->nz0, 0x2f7c + animation * 4U, 4, &velocity))
        return SDL_SetError("Spear Guard movement animation index is invalid");
    const Uint32 address = Read32(pointer.bytes);
    const Sint32 delta = Signed32(velocity.bytes);
    if (address < 0x80180000 || address - 0x80180000 >= s->nz0.size || Read16(frame.bytes) > 512 ||
        delta < -65536 || delta > 65536)
        return SDL_SetError("Spear Guard private movement recipe exceeds its bounds");
    Uint8 result = 0;
    if (!ActorAnimate(s, a, address - 0x80180000, &result))
        return false;
    if ((result & 128) && a->state.step == 7 && a->pose != 3)
    {
        if (a->spear.movement_facing != a->state.facing_left)
            a->vx = -a->vx;
        a->spear.movement_facing = a->state.facing_left;
    }
    Sint32 vx = a->vx;
    if (a->pose >= Read16(frame.bytes))
    {
        if (vx)
            vx += a->spear.movement_facing ? delta : -delta;
    }
    else
        vx += a->spear.movement_facing ? -delta : delta;
    a->vx = vx;
    Uint16 collision = 0;
    if (!ActorWalk(s, a, 0x3064, &collision))
        return false;
    if ((collision & 96) == 96)
        a->state.x -= vx;
    return true;
}
static bool Decide(SotnActors *s, Actor *a)
{
    if (s->player.subweapon_starting || s->player.weapon_starting)
    {
        if (Distance(a, s->player) > 72 && (ActorRandom(s) & 1))
            SetStep(a, 9);
    }
    else if (--a->spear.timer == 0)
    {
        SotnDataView step = {0}, timer = {0};
        const Uint32 index = a->spear.index++ & 7;
        if (!Slice(s->nz0, 0x2f04 + index, 1, &step) ||
            !Slice(s->nz0, 0x2f0c + (ActorRandom(s) & 3), 1, &timer))
            return false;
        if (step.bytes[0] != 6 && step.bytes[0] != 9 && step.bytes[0] != 10)
            return SDL_SetError("Spear Guard private attack cycle selects an invalid state");
        SetStep(a, step.bytes[0]);
        a->spear.timer = timer.bytes[0];
        if (a->state.parameters && Distance(a, s->player) > 80 && (ActorRandom(s) & 1))
            SetStep(a, 11);
        a->vx = 0;
    }
    if (SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) > 64)
        SetStep(a, 3);
    return true;
}
static bool Slide(SotnActors *s, Actor *a, Sint32 deceleration)
{
    if (a->vx)
        a->vx += a->vx > 0 ? -deceleration : deceleration;
    Uint16 collision = 0;
    if (!ActorWalk(s, a, 0x3064, &collision))
        return false;
    if ((collision & 96) == 96)
        a->state.x -= a->vx;
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Spear Guard requires its original adjacent blocking actor");
    Actor *block = &s->slots[a->slot + 1];
    if (a->state.step > 0 && a->state.step < 12 &&
        (block->state.id != 0x50 || block->enemy_state.owner_generation != a->state.generation))
        return SDL_SetError("Spear Guard lost its original blocking actor");
    if (a->state.step && !a->spear.alternate)
    {
        block->attack = a->attack;
        block->attack_element = a->attack_element;
    }
    if (a->state.step < 12 && (a->state.flags & 0x100))
    {
        if (block->enemy_state.owner_generation == a->state.generation)
        {
            block->remove = true;
            block->state.hitbox_state = 0;
        }
        SetStep(a, 12);
        a->state.hitbox_state = 0;
        a->spear.timer = 64;
        a->state.opacity_set = true;
        a->state.opacity = 127;
    }
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (a->state.parameters > 3 || block->state.generation)
            return SDL_SetError("Spear Guard variant or reserved blocking slot is invalid");
        if (!ActorInitialize(s, a, 0xb00))
            return false;
        a->spear.patrol = 176;
        a->spear.timer = 64;
        a->state.palette = (Uint16)(a->state.palette + (a->state.parameters & 3));
        a->state.hitbox_y = 4;
        Actor *child = NULL;
        return ActorSpawnAt(s, a, 0x50, 0, a->slot + 1U, &child);
    }
    case 1:
        if (!ActorGround(s, a, 0x306c, &collision))
            return false;
        if (collision & 1)
            SetStep(a, 2);
        break;
    case 2:
    case 3:
        a->state.facing_left = a->spear.movement_facing;
        if (!Move(s, a, a->state.step == 2 ? 0 : 1))
            return false;
        if (SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 32)
        {
            SetStep(a, 4);
            a->vx = 0;
            return ActorSound(s, 0x773);
        }
        if (Distance(a, s->player) < 24)
        {
            SetStep(a, 7);
            a->vx = 0;
        }
        else if (--a->spear.patrol == 0)
        {
            a->spear.patrol = 176;
            a->spear.movement_facing = !a->spear.movement_facing;
            a->vx = -a->vx;
        }
        break;
    case 4:
    case 5:
        if ((a->state.step == 4 && Distance(a, s->player) < 100) ||
            (a->state.step == 5 && Distance(a, s->player) > 120))
        {
            a->state.step = a->state.step == 4 ? 5 : 4;
            a->vx = 0;
        }
        a->state.facing_left = Side(a, s->player) == 0;
        a->spear.movement_facing = a->state.facing_left != (a->state.step == 5);
        return Move(s, a, 2) && Decide(s, a);
    case 6:
        if (!ActorAnimate(s, a, 0x2fb0, &animation))
            return false;
        if (!animation)
        {
            SetStep(a, 4);
            if (!ActorSound(s, 0x773))
                return false;
        }
        else if ((animation & 128) && a->state.frame == 7)
        {
            if (!ActorSound(s, 0x60b))
                return false;
            a->vx = a->state.facing_left ? 4 * 65536 : -4 * 65536;
        }
        if (a->vx && !Attack(s, block, 94))
            return false;
        return Slide(s, a, 16384);
    case 7:
        a->state.facing_left = Side(a, s->player) == 0;
        if (!Move(s, a, 3))
            return false;
        if (--a->spear.timer == 0)
        {
            SetStep(a, 8);
            a->spear.side =
                (Uint8)(Side(a, s->player) | (Pixel(a->state.y) > Pixel(s->player.y) ? 2 : 0));
            a->vx = 0;
        }
        if (Distance(a, s->player) > 72)
        {
            SetStep(a, 3);
            a->vx = 0;
        }
        break;
    case 8:
        if (!ActorAnimate(s, a, a->spear.side & 2 ? 0x3034 : 0x3018, &animation) ||
            !Attack(s, block, 94))
            return false;
        if (!animation)
        {
            SetStep(a, 7);
            SotnDataView timer = {0};
            if (!Slice(s->nz0, 0x2f10 + (ActorRandom(s) & 3), 1, &timer))
                return false;
            a->spear.timer = timer.bytes[0];
        }
        else if ((animation & 128) && (a->state.frame == 45 || a->state.frame == 38))
            return ActorSound(s, 0x60b);
        break;
    case 9:
        if (!(s->ticks & 31) && !ActorSound(s, 0x60a))
            return false;
        block->state.hitbox_state |= 2;
        block->state.flags |= 0xc000;
        if (!ActorAnimate(s, a, 0x2fbc, &animation))
            return false;
        if (!animation)
        {
            block->state.hitbox_state &= 0xfffdU;
            block->state.flags &= ~0xc000U;
            SetStep(a, 5);
        }
        break;
    case 10:
        if (!ActorAnimate(s, a, 0x2fdc, &animation))
            return false;
        if (!animation)
        {
            SetStep(a, 4);
            if (!ActorSound(s, 0x773))
                return false;
        }
        else if (animation & 128)
        {
            const Uint8 pose = (Uint8)(a->pose - 6);
            if (pose == 3 && !ActorSound(s, 0x770))
                return false;
            if (!pose)
            {
                a->vx = a->state.facing_left ? 8 * 65536 : -8 * 65536;
                if (!Attack(s, block, 95))
                    return false;
                block->state.hitbox_state |= 2;
                a->spear.alternate = true;
            }
            if (pose == 4)
            {
                a->spear.alternate = false;
                block->attack = 3;
                block->state.hitbox_state &= 0xfffdU;
            }
        }
        return Slide(s, a, 32768);
    case 11:
        if (!ActorAnimate(s, a, 0x2ff4, &animation))
            return false;
        if (!animation)
        {
            SetStep(a, 4);
            if (!ActorSound(s, 0x773))
                return false;
        }
        else if ((animation & 128) && (Uint8)(a->pose - 7) == 0)
        {
            a->vx = a->state.facing_left ? 8 * 65536 : -8 * 65536;
            if (!ActorSound(s, 0x689))
                return false;
            Actor *spear = NULL;
            if (!ActorSpawn(s, a, 0x58, 0, 96, 128, &spear))
                return false;
            if (spear != NULL)
            {
                spear->state.x = Pixel(a->state.x) * 65536;
                spear->state.y = Pixel(a->state.y) * 65536;
                spear->enemy_state.owner_slot = a->slot;
                spear->enemy_state.owner_generation = a->state.generation;
            }
        }
        return Slide(s, a, 65536);
    case 12:
        if (!ActorAnimate(s, a, 0x3008, &animation))
            return false;
        a->state.opacity = a->state.opacity > 2 ? (Uint16)(a->state.opacity - 2) : 0;
        if (--a->spear.timer == 0)
        {
            a->remove = true;
            return true;
        }
        for (Uint32 i = 0; i < 16; ++i)
        {
            SotnDataView time = {0};
            if (!Slice(s->nz0, 0x2f14 + i, 1, &time))
                return false;
            if (a->spear.timer != time.bytes[0])
                continue;
            if ((i == 14 && !ActorSound(s, 0x771)) ||
                (!(i & 1) && i != 14 && !ActorSound(s, 0x653)))
                return false;
            SotnDataView position = {0};
            if (!Slice(s->nz0, 0x2f24 + i * 4, 4, &position) ||
                SDL_abs(Signed16(position.bytes)) > 128 ||
                SDL_abs(Signed16(position.bytes + 2)) > 128)
                return SDL_SetError("Spear Guard death burst position exceeds its private bounds");
            Actor origin = *a;
            origin.state.x = (Pixel(a->state.x) + Signed16(position.bytes)) * 65536;
            origin.state.y = (Pixel(a->state.y) + Signed16(position.bytes + 2)) * 65536;
            if (!ActorExplosionInPool(s, &origin, 2, 160, 192, (Uint16)(a->state.priority - 1)))
                return false;
        }
        break;
    default:
        return SDL_SetError("Spear Guard entered invalid state %u", a->state.step);
    }
    return true;
}
bool ActorSpearGuard(SotnActors *s, Actor *a)
{
    return a->state.id == 0x50 ? Block(s, a) : a->state.id == 0x58 ? Thrown(s, a) : Body(s, a);
}
