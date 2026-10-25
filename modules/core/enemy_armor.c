// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_armor_lord.h logic at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Original tables remain private.
#include "actors_internal.h"

static bool Shadow(SotnActors *s, Actor *a)
{
    const Uint16 owner = a->enemy_state.owner_slot;
    if (owner >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Armor Lord sword owner exceeds the actor slots");
    const Actor *p = &s->slots[owner];
    if (p->state.id != 0x4a || p->state.generation != a->enemy_state.owner_generation || p->remove)
    {
        a->remove = true;
        return true;
    }
    a->state.x = Pixel(p->state.x) * 65536;
    a->state.y = Pixel(p->state.y) * 65536;
    a->state.facing_left = p->state.facing_left;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0xae8))
            return false;
        a->state.blend_mode |= 0x30;
        a->state.opacity_set = true;
        a->state.frame = 0;
    }
    else if (a->state.step == 1)
    {
        if (p->state.frame == 16 && p->state.step == 6)
        {
            a->state.step = 2;
            a->state.frame = 32;
            a->state.opacity = 96;
            a->state.scale_x = a->state.scale_y = 456;
        }
        if (p->state.frame == 21)
        {
            a->state.step = 3;
            a->state.frame = 33;
            a->state.opacity = 96;
            a->state.scale_x = a->state.scale_y = 440;
        }
        a->timer = 3;
    }
    else if (a->state.step == 2 || a->state.step == 3)
    {
        a->timer = (Uint16)(a->timer - 1);
        if (!a->timer)
            a->state.frame = 0;
        else
            a->state.opacity = (Uint8)(a->state.opacity - 32);
        if (p->state.frame != (a->state.step == 2 ? 16 : 21))
            a->state.step = 1;
    }
    else
        return SDL_SetError("Armor Lord sword entered an invalid state");
    a->state.hitbox_state = p->state.frame == 30 ? 3 : 1;
    const Uint32 index = p->state.frame >= 5 && p->state.frame <= 30 ? p->state.frame - 4U : 0;
    SotnDataView dimensions = {0}, offsets = {0};
    if (!Slice(s->nz0, 0x2e2c + index * 4, 4, &dimensions) ||
        !Slice(s->nz0, 0x2e98 + index * 4, 4, &offsets))
        return false;
    a->state.hitbox_width = Read16(dimensions.bytes);
    a->state.hitbox_height = Read16(dimensions.bytes + 2);
    a->state.hitbox_x = (Sint16)Signed16(offsets.bytes);
    a->state.hitbox_y = (Sint16)Signed16(offsets.bytes + 2);
    if (a->state.hitbox_width > 64 || a->state.hitbox_height > 64 ||
        SDL_abs(a->state.hitbox_x) > 128 || SDL_abs(a->state.hitbox_y) > 128)
        return SDL_SetError("Armor Lord sword exceeds its private hitbox bounds");
    return true;
}

static bool Shield(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
        if (!ActorPrimitives(s, a, 2))
        {
            SDL_ClearError();
            a->substep = 4;
            break;
        }
        a->state.hitbox_state = 1;
        a->armor.timer = 0;
        a->armor.width = a->armor.height = 0;
        a->armor.brightness = 128;
        ++a->substep;
        if (!ActorSound(s, 0x6c5))
            return false;
        break;
    case 1:
        ++a->armor.width;
        a->armor.height = (Uint16)(a->armor.height + 8);
        if (a->armor.timer++ > 8)
        {
            a->armor.timer = 0;
            ++a->substep;
        }
        break;
    case 2:
        break;
    case 3:
        a->armor.brightness = (Uint8)(a->armor.brightness - 8);
        if (a->armor.timer++ > 15)
        {
            ActorPrimitivesClear(s, a);
            a->state.primitive_count = 0;
        }
        break;
    case 4:
        break;
    default:
        return SDL_SetError("Armor Lord shield entered an invalid state");
    }
    if (a->state.primitive_count)
    {
        if (!ActorSprite(s, a, 0, (Pixel(a->state.x) + (a->state.facing_left ? 22 : -22)) * 65536,
                         (Pixel(a->state.y) - 4) * 65536, 20, 192, 24, 63, 0x1a, 0x161))
            return false;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (p == NULL)
            return false;
        p->state.x = (Pixel(a->state.x) + (a->state.facing_left ? 22 : -22)) * 65536;
        p->state.y = (Pixel(a->state.y) - 4) * 65536;
        p->state.width = a->armor.width;
        p->state.height = a->armor.height;
        p->state.quad = true;
        for (Uint32 i = 0; i < 4; ++i)
        {
            p->state.offset_x[i] = (Sint16)((i & 1U ? 1 : -1) * (a->armor.width / 2));
            p->state.offset_y[i] = (Sint16)((i < 2 ? -1 : 1) * (a->armor.height / 2));
        }
        p->state.red = p->state.green = p->state.blue = a->armor.brightness;
        p->state.blend = p->state.modulate = true;
        p->state.active = a->armor.width && a->armor.height;
    }
    return true;
}

static bool Body(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    Uint16 collision = 0;
    if ((a->state.flags & 0x100) && a->state.step < 8)
    {
        if (a->slot + 1U < SOTN_STAGE_ACTOR_SLOTS)
        {
            Actor *child = &s->slots[a->slot + 1U];
            if (child->state.id == 0x4b &&
                child->enemy_state.owner_generation == a->state.generation)
                child->remove = true;
        }
        if (!ActorSound(s, 0x753))
            return false;
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        a->state.flags &= ~0x20000000U;
        a->state.priority = (Uint16)(a->state.priority - 4);
        a->state.hitbox_state = 0;
        s->armor_lord_killed = true;
        SetStep(a, 8);
    }
    switch (a->state.step)
    {
    case 0: {
        if (s->armor_lord_killed)
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, 0xadc))
            return false;
        Actor *child = NULL;
        if (!ActorSpawnAt(s, a, 0x4b, 0, a->slot + 1U, &child))
            return false;
        a->state.facing_left = Side(a, s->player) == 0;
        break;
    }
    case 1:
        if (!ActorGround(s, a, 0x2d58, &collision))
            return false;
        if (collision & 1)
            SetStep(a, 2);
        break;
    case 2:
        if (Distance(a, s->player) < 160)
            SetStep(a, 3);
        break;
    case 3:
        if (!a->substep)
        {
            SotnDataView timer = {0};
            a->state.facing_left = Side(a, s->player) == 0;
            a->armor.movement_facing = a->state.facing_left;
            if (!Slice(s->nz0, 0x2d70 + (ActorRandom(s) & 3U) * 2, 2, &timer))
                return false;
            a->timer = Read16(timer.bytes);
            if (!a->timer || a->timer > 256)
                return SDL_SetError("Armor Lord attack delay exceeds its private bounds");
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x2d78, &animation) || !ActorWalk(s, a, 0x2d68, &collision))
            return false;
        if (!animation)
            a->state.facing_left = Side(a, s->player) == 0;
        a->vx = a->armor.movement_facing ? 16384 : -16384;
        if (Distance(a, s->player) < 80)
            a->armor.movement_facing = !a->state.facing_left;
        if (Distance(a, s->player) > 112)
            a->armor.movement_facing = a->state.facing_left;
        if (!--a->timer)
        {
            SotnDataView step = {0};
            if (!Slice(s->nz0, 0x2d50 + (ActorRandom(s) & 7U), 1, &step))
                return false;
            if (step.bytes[0] < 4 || step.bytes[0] > 6)
                return SDL_SetError("Armor Lord attack selection is outside its original states");
            a->state.facing_left = Side(a, s->player) == 0;
            SetStep(a, step.bytes[0]);
        }
        if (s->player.weapon_starting)
        {
            if (!a->armor.did_shield && (ActorRandom(s) & 1))
            {
                a->state.facing_left = Side(a, s->player) == 0;
                SetStep(a, 7);
            }
            a->armor.did_shield = true;
        }
        else
            a->armor.did_shield = false;
        break;
    case 4:
        if (!ActorAnimate(s, a,
                          a->substep == 0   ? 0x2d84
                          : a->substep == 1 ? 0x2d9c
                                            : 0x2da4,
                          &animation))
            return false;
        if (a->substep == 0 && !animation)
        {
            a->pose = a->remaining = 0;
            a->substep = 1;
            a->timer = 128;
        }
        else if (a->substep == 1)
        {
            if (!--a->timer)
            {
                a->pose = a->remaining = 0;
                ++a->substep;
            }
            if (a->timer % 7 == 0 && !ActorSound(s, 0x60a))
                return false;
        }
        else if (a->substep == 2 && !animation)
            SetStep(a, 3);
        break;
    case 5:
        if (!a->substep)
        {
            a->vx = a->vy = 0;
            ++a->substep;
        }
        if (!a->remaining && a->state.frame == 20 &&
            (!ActorSound(s, 0x6c7) || !ActorSound(s, 0x751)))
            return false;
        if (!a->remaining && a->pose == 5)
            a->vx = a->state.facing_left ? 4 * 65536 : -4 * 65536;
        if (!ActorWalk(s, a, 0x2d68, &collision))
            return false;
        a->vx -= a->vx / 8;
        if (!ActorAnimate(s, a, 0x2dac, &animation))
            return false;
        if (!animation)
            SetStep(a, 3);
        break;
    case 6:
        if (a->pose > 5 && !a->substep)
        {
            if (!ActorSound(s, 0x660) || !ActorSound(s, 0x752))
                return false;
            Actor *child = NULL;
            if (!ActorSpawn(s, a, 0x4c, 0, 160, 192, &child))
                return false;
            if (child != NULL)
            {
                child->state.x = Pixel(a->state.x) * 65536;
                child->state.y = Pixel(a->state.y) * 65536;
            }
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x2dc4, &animation))
            return false;
        if (!animation)
            SetStep(a, 3);
        break;
    case 7:
        if (!ActorAnimate(s, a, 0x2de0, &animation))
            return false;
        if (!animation)
        {
            a->state.hitbox_state = 3;
            a->substep = 3;
        }
        if (a->pose > 1)
        {
            if (!Shield(s, a))
                return false;
            if (!a->state.primitive_count)
                SetStep(a, 3);
        }
        break;
    case 8:
        if (!ActorAnimate(s, a, 0x2dec, &animation))
            return false;
        if (!animation)
            SetStep(a, 9);
        if (s->ticks % 8 == 0)
        {
            Actor *child = NULL;
            if (!ActorSound(s, 0x653) || !ActorSpawn(s, a, 6, 0, 160, 192, &child))
                return false;
            if (child != NULL)
            {
                child->state.x = (Pixel(a->state.x) + (Sint32)(ActorRandom(s) & 31U) - 16) * 65536;
                child->state.y = (Pixel(a->state.y) + (Sint32)(ActorRandom(s) & 63U) - 16) * 65536;
                child->state.facing_left = false;
            }
        }
        break;
    case 9:
        if (!ActorArmorDeath(s, a))
            return false;
        break;
    default:
        return SDL_SetError("Armor Lord entered invalid state %u", a->state.step);
    }
    a->state.hitbox_height = 28;
    a->state.hitbox_x = 0;
    a->state.hitbox_y = 12;
    switch (a->state.frame)
    {
    case 5:
    case 16:
    case 17:
    case 29:
        a->state.hitbox_x = 16;
        break;
    case 12:
    case 13:
    case 21:
    case 25:
        a->state.hitbox_x = -2;
        a->state.hitbox_height = 26;
        break;
    case 4:
    case 6:
    case 7:
    case 30:
        a->state.hitbox_x = 7;
        break;
    default:
        break;
    }
    if (Pixel(a->state.x) > 144)
        a->state.x = 144 * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    return true;
}

bool ActorArmorLord(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x4a)
        return Body(s, a);
    if (a->state.id == 0x4b)
        return Shadow(s, a);
    if (a->state.id == 0x4c || a->state.id == 0x4d)
        return ActorArmorWave(s, a);
    if (a->state.id == 0x4e)
        return true;
    return SDL_SetError("Armor Lord dispatch received a foreign actor ID");
}
