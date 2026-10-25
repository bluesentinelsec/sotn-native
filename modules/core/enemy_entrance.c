// SPDX-License-Identifier: AGPL-3.0-or-later
// Bat and Zombie behaviors: st/e_bat.h and st/e_zombie.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables remain in the private overlay.
#include "actors_internal.h"

bool ActorBat(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (a->state.flags & 0x100)
    {
        if (!ActorExplosion(s, a, 1) ||
            !(s->stage_id == 6 ? ActorPositionalSound(s, a, 0x69c) : ActorSound(s, 0x69c)))
            return false;
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, s->stage_id == 6 ? 0xa58 : EntranceTable(s, 0xb90, 0xafc)))
            return false;
        a->state.frame = 31;
        break;
    case 1:
        if (Distance(a, s->player) < 96 && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 96 &&
            Pixel(s->player.y) >= Pixel(a->state.y))
            ++a->state.step;
        break;
    case 2:
        if (!ActorAnimate(s, a, s->stage_id == 6 ? 0x1f3c : EntranceTable(s, 0x3c60, 0x2570),
                          &animation))
            return false;
        if (animation == 0)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->vy = 57344;
            a->vx = a->state.facing_left ? 16384 : -16384;
            a->pose = (Uint16)((ActorRandom(s) & 3) * 3);
            a->remaining = 0;
            ++a->state.step;
        }
        break;
    case 3:
    case 4:
        if (!ActorAnimate(s, a, s->stage_id == 6 ? 0x1f20 : EntranceTable(s, 0x3c44, 0x2554),
                          &animation))
            return false;
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (a->state.step == 3)
        {
            if (SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 32)
            {
                a->vx = a->state.facing_left ? 65536 : -65536;
                a->gravity = 2048;
                ++a->state.step;
            }
        }
        else
        {
            if (a->vy < -65536 || a->vy > 65536)
                a->gravity = -a->gravity;
            a->vy += a->gravity;
        }
        break;
    default:
        return SDL_SetError("Entrance Bat entered invalid state %u", a->state.step);
    }
    return true;
}

static bool Zombie(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    Uint16 collision = 0;
    if ((a->state.flags & 0x100) && a->state.step < 4)
    {
        Actor *puff = NULL;
        if (!ActorSound(s, 0x65e) ||
            !ActorSpawn(s, a, EntranceActorId(s, 0x62), 3, 160, 192, &puff))
            return false;
        if (puff != NULL)
        {
            puff->state.priority = (Uint16)(a->state.priority + 1);
            puff->state.y += 12 * 65536;
        }
        a->state.hitbox_state = 0;
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, EntranceTable(s, 0xba8, 0xb08)))
            return false;
        a->state.hitbox_width = 8;
        a->state.hitbox_height = 0;
        a->state.hitbox_y = 16;
        a->state.priority = (Uint16)(a->state.priority + 4);
        a->state.palette = (Uint16)(a->state.palette + (s->ticks & 1) + (ActorRandom(s) & 1));
        a->state.frame = 0;
        break;
    case 1:
        if (!ActorGround(s, a, EntranceTable(s, 0x3cac, 0x25bc), &collision))
            return false;
        if (collision & 1)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            ++a->state.step;
        }
        break;
    case 2:
        if (!ActorAnimate(s, a, EntranceTable(s, 0x3c84, 0x2594), &animation))
            return false;
        if (animation == 0)
            SetStep(a, 3);
        if (a->remaining == 0)
        {
            a->state.hitbox_y -= 2;
            a->state.hitbox_height += 2;
        }
        break;
    case 3:
        if (!ActorAnimate(s, a, EntranceTable(s, 0x3c7c, 0x258c), &animation) ||
            !ActorWalk(s, a, EntranceTable(s, 0x3cbc, 0x25cc), &collision))
            return false;
        a->vx = a->state.facing_left ? 32768 : -32768;
        if (collision & 0xc0)
        {
            a->state.hitbox_state = 0;
            SetStep(a, 4);
        }
        break;
    case 4:
        if (!ActorAnimate(s, a, EntranceTable(s, 0x3c98, 0x25a8), &animation))
            return false;
        if (animation == 0)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Entrance Zombie entered invalid state %u", a->state.step);
    }
    return true;
}

static bool Spawner(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xad0, 0xa60)))
            return false;
        a->state.flags &= 0x2000;
        a->timer = 1;
    }
    if (s->world->flags[SOTN_ENTRANCE_ON] && a->timer && --a->timer == 0)
    {
        Actor *child = NULL;
        a->state.x = (s->camera_x + 128) * 65536;
        if (!ActorSpawn(s, a, EntranceActorId(s, 0x4c), 0, 96, 104, &child))
            return false;
        if (child != NULL)
        {
            const Sint32 offset = (Sint32)(ActorRandom(s) & 63) + 96;
            child->state.x += (a->substep ? offset : -offset) * 65536;
            child->state.y -= 48 * 65536;
            a->substep ^= 1;
            if (Pixel(child->state.x) < s->camera->left + 128 ||
                Pixel(child->state.x) > s->camera->right - 128)
                child->remove = true;
        }
        a->timer = (Uint16)((ActorRandom(s) & 63) + 32);
    }
    return true;
}

static bool Camera(SotnActors *s, Actor *a)
{
    const Uint16 parameter = a->state.parameters;
    SotnDataView sizes = {0}, axes = {0}, bounds = {0};
    if (parameter >= 7 || !Slice(s->nz0, EntranceTable(s, 0xdc0, 0xd98), 8, &sizes) ||
        !Slice(s->nz0, EntranceTable(s, 0xdc8, 0xda0), 8, &axes))
        return SDL_SetError("Entrance camera parameter exceeds private table");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xaf4, 0xa84)))
            return false;
        a->camera.horizontal = axes.bytes[parameter] != 0;
        a->state.hitbox_width = a->camera.horizontal ? sizes.bytes[parameter] : 16;
        a->state.hitbox_height = a->camera.horizontal ? 16 : sizes.bytes[parameter];
        return true;
    }
    if ((parameter == 4 || parameter == 5) && s->camera->left != 0)
        return true;
    if (parameter == 6 && (s->buttons & SOTN_BUTTON_CANCEL))
        return SotnCamera_Bounds(s->camera, 0, s->camera->top, 1280, s->camera->bottom);
    if (Distance(a, s->player) > a->state.hitbox_width ||
        SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) > a->state.hitbox_height)
        return true;
    const size_t side = a->camera.horizontal ? (Pixel(a->state.y) > Pixel(s->player.y) ? 4U : 0U)
                                             : (size_t)Side(a, s->player) * 4;
    if (!Slice(s->nz0, EntranceTable(s, 0xdd0, 0xda8) + (size_t)parameter * 16 + side * 2, 8,
               &bounds))
        return false;
    return SotnCamera_Bounds(s->camera, Read16(bounds.bytes), Read16(bounds.bytes + 2),
                             Read16(bounds.bytes + 4), Read16(bounds.bytes + 6));
}

static bool Puff(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (a->state.id == EntranceActorId(s, 0x62))
        return ActorBossPuff(s, a);
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xac4, 0xa54)))
            return false;
        a->pose = a->remaining = 0;
        a->state.animation_set = 14;
        a->state.texture_offset = 0x79;
        a->state.palette = 0xd0;
        a->state.blend_mode = 0x30;
        a->state.opacity = 96;
        if (a->state.parameters & 0xff00)
            a->state.priority = (Uint16)(a->state.parameters >> 8);
        a->vy -= 65536;
        return true;
    }
    a->state.y += a->vy;
    if (!ActorAnimate(s, a, 0x383c, &animation))
        return false;
    if (animation == 0)
        a->remove = true;
    return true;
}

// e_background_lightning.h: first-hallway progression and music trigger.
// Palette lightning/thunder/cloud presentation remains a separate effects integration.
static bool Hallway(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xadc, 0xa6c)))
            return false;
        if (s->stage_id == 7)
            s->world->flags[SOTN_ENTRANCE_ON] = 1;
        if (a->state.parameters & 256)
            a->state.step = 4;
        else if (s->world->flags[SOTN_ENTRANCE_ON])
            a->state.parameters = 0;
        if (a->state.parameters == 1)
        {
            s->combat.music_change = true;
            s->combat.music = 0x32c;
        }
    }
    if (a->state.parameters == 1 && a->state.step == 1 && Pixel(s->player.x) > 768)
    {
        s->world->flags[SOTN_ENTRANCE_ON] = 1;
        s->combat.music_change = true;
        s->combat.music = 0x30f;
        SetStep(a, 2);
    }
    if (s->stage_id == 65 && a->state.step == 2)
    {
        Uint8 animation = 0;
        a->state.frame = 0;
        if (!ActorAnimate(s, a, 0xfbc, &animation))
            return false;
        if (animation == 0)
        {
            a->state.parameters = 0;
            SetStep(a, 1);
        }
    }
    a->state.frame = 0;
    return true;
}

bool ActorEntrance(SotnActors *s, Actor *a)
{
    if ((s->stage_id == 65 && (a->state.id == 0x5c || a->state.id == 0x5d)) ||
        (s->stage_id == 7 && (a->state.id == 0x4b || a->state.id == 0x4c)))
        return ActorEntranceStairway(s, a);
    if (a->state.id >= 0x24 && a->state.id <= 0x27)
        return ActorEntranceRock(s, a);
    if (s->stage_id == 7)
    {
        switch (a->state.id)
        {
        case 5:
            return ActorDoor(s, a);
        case 0x12:
            return Camera(s, a);
        case 0x17:
            return Hallway(s, a);
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x3c:
        case 0x3d:
            return ActorMerman(s, a);
        case 0x3e:
        case 0x3f:
            return ActorSkeleton(s, a, s->player);
        case 0x40:
            return ActorBat(s, a);
        case 0x42:
            return Zombie(s, a);
        case 0x43:
            return Spawner(s, a);
        case 0x48:
            return ActorBloodyZombie(s, a, s->player);
        case 0x4d:
            return ActorBossPuff(s, a);
        default:
            a->state.unsupported = true;
            return true;
        }
    }
    switch (a->state.id)
    {
    case 0x17:
        return Hallway(s, a);
    case 0x57:
    case 0x58:
    case 0x59:
    case 0x5a:
    case 0x5b:
    case 0x5e:
        return ActorDeath(s, a);
    case 5:
        return ActorDoor(s, a);
    case 0x12:
        return Camera(s, a);
    case 0x61:
    case 0x62:
        return Puff(s, a);
    case 0x3a:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
    case 0x40:
    case 0x43:
    case 0x44:
    case 0x41:
    case 0x42:
    case 0x45:
        return ActorMerman(s, a);
    case 0x46:
    case 0x47:
        return ActorSkeleton(s, a, s->player);
    case 0x48:
        return ActorBat(s, a);
    case 0x4a:
    case 0x4b:
        return ActorWarg(s, a);
    case 0x4c:
        return Zombie(s, a);
    case 0x4d:
        return Spawner(s, a);
    default:
        a->state.unsupported = true;
        return true;
    }
}
