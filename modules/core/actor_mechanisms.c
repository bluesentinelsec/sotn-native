// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed Laboratory switches/lifts/blocks/secrets from nz0/e_nz0_room2.c,
// e_puzzle.c, e_elevator.c, e_elevator2.c, e_left_secret_room_wall.c at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables stay private.
#include "actors_internal.h"
static bool Sprite(SotnActors *s, Actor *a, Uint8 u, Uint8 v, Uint16 w, Uint16 h, Sint32 dx,
                   Sint32 dy)
{
    return ActorSprite(s, a, 0, a->state.x + dx * 65536, a->state.y + dy * 65536, u, v, w, h, 15,
                       9);
}
static bool Initialize(SotnActors *s, Actor *a, Uint16 count, Uint16 priority)
{
    if (!ActorInitialize(s, a, 0xbf8) || !ActorPrimitives(s, a, count))
        return false;
    a->state.priority = priority;
    a->mechanism.base_y = Pixel(a->state.y);
    return true;
}
static bool TileAt(SotnActors *s, Sint32 x, Sint32 y, Uint32 *index)
{
    if (x < 0 || y < 0 || (Uint32)x >= s->map.width * 16 || (Uint32)y >= s->map.height * 16)
        return SDL_SetError("Room mechanism references a tile outside its room");
    *index = (Uint32)x / 16 + (Uint32)y / 16 * s->map.width;
    return true;
}
static bool Button(SotnActors *s, Actor *a)
{
    const bool puzzle = a->state.id == 0x19, normal = a->state.id == 0x24,
               blood = a->state.id == 0x20;
    Uint16 contact = 0;
    if ((normal || puzzle) && !ActorObjectCollision(s, a, 8, 8, 4, &contact))
        return false;
    if (a->state.step == 0)
    {
        if (!Initialize(s, a, 1, 0x5f))
            return false;
        if (normal)
        {
            Uint32 tile = 0;
            if (!TileAt(s, Pixel(a->state.x), Pixel(a->state.y) + 4, &tile) ||
                !ActorTile(s, tile, 0x5af))
                return false;
        }
        if (!normal && !puzzle)
        {
            a->state.hitbox_width = 6;
            a->state.hitbox_height = 8;
            a->state.hitbox_y = -22;
            a->state.hitbox_state = 1;
            if (blood)
            {
                if (a->slot == 0)
                    return SDL_SetError("Lift button has no preceding Blood Skeleton slot");
                Actor *child = NULL;
                if (!ActorSpawnAt(s, a, 0x26, 0, a->slot - 1, &child))
                    return false;
                if (child == NULL)
                    return SDL_SetError("Cannot initialize lift-button Blood Skeleton");
                child->state.y = 344 * 65536;
                a->enemy_state.owner_slot = child->slot;
                a->enemy_state.owner_generation = child->state.generation;
            }
        }
        if (a->state.id == 0x48 && (a->state.parameters < 1 || a->state.parameters > 3))
            return SDL_SetError("Lift call button has an invalid stop");
    }
    bool pressed = contact != 0;
    if (!normal && !puzzle)
    {
        pressed = a->hit_params != 0;
        if (blood)
        {
            const Actor *child = &s->slots[a->enemy_state.owner_slot];
            if (child->state.generation == a->enemy_state.owner_generation &&
                SDL_abs(Pixel(a->state.x) - Pixel(child->state.x)) < 8)
                pressed = true;
        }
    }
    const Sint32 base = puzzle ? 464 : blood ? 372 : a->mechanism.base_y;
    if (blood)
    {
        ShiftY(a, pressed ? 1 : -1);
        const Sint32 y = SDL_clamp(Pixel(a->state.y), base, base + 4);
        ShiftY(a, (Sint64)y - Pixel(a->state.y));
        s->call_elevator = pressed;
    }
    else if (a->state.step == 1 && pressed)
    {
        if ((normal || puzzle) && !ActorDisplace(s, 0, 65536, 0))
            return false;
        ShiftY(a, 1);
        if (Pixel(a->state.y) > base + 4)
        {
            ShiftY(a, (Sint64)base + 4 - Pixel(a->state.y));
            a->state.step = 2;
            if (puzzle)
                s->puzzle_flags ^= a->state.parameters;
            else if (normal)
            {
                if (a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS || s->slots[a->slot + 1].state.id != 0x25)
                    return SDL_SetError("Floor button is missing its adjacent spike actor");
                s->slots[a->slot + 1].mechanism.pressed ^= true;
                s->slots[a->slot + 1].mechanism.changed = true;
            }
            else
            {
                s->elevator_target = a->state.parameters;
                if (!ActorSound(s, 0x676))
                    return false;
            }
        }
    }
    else if (puzzle && a->state.step == 2 && !pressed)
        a->state.step = 3;
    else if ((puzzle && a->state.step == 3) || (!puzzle && a->state.step == 2 && !pressed))
    {
        ShiftY(a, -1);
        if (Pixel(a->state.y) < base)
        {
            ShiftY(a, (Sint64)base - Pixel(a->state.y));
            a->state.step = 1;
        }
    }
    return Sprite(s, a, 72, 200, 16, 16, -8, -8);
}
static bool Spikes(SotnActors *s, Actor *a)
{
    const bool puzzle = a->state.id == 0x1a;
    if (puzzle && (a->state.parameters < 1 || a->state.parameters > 2))
        return SDL_SetError("Spike puzzle index is outside its modifier slots");
    if (a->state.step == 0)
    {
        if (!Initialize(s, a, 1, 0x5f))
            return false;
        a->state.hitbox_width = a->state.hitbox_height = 12;
        a->state.hitbox_state = 1;
        a->attack = 7;
        a->attack_element = 1;
        if (puzzle)
        {
            a->mechanism.pressed = (s->puzzle_flags & a->state.parameters) != 0;
            a->state.y = (a->mechanism.pressed ? 480 : 452) * 65536;
        }
        else
        {
            Uint32 tile = 0;
            if (!TileAt(s, Pixel(a->state.x) - 4, Pixel(a->state.y) - 4, &tile) ||
                !ActorTile(s, tile, 0x102) || !ActorTile(s, tile + 1, 0x103))
                return false;
            ShiftY(a, -28);
        }
    }
    const bool down = puzzle ? (s->puzzle_flags & a->state.parameters) != 0 : a->mechanism.pressed;
    const Sint32 target = puzzle ? (down ? 480 : 452) : a->mechanism.base_y - (down ? 0 : 28);
    if (puzzle)
    {
        if (a->mechanism.pressed != down && !ActorSound(s, 0x69d))
            return false;
        a->mechanism.pressed = down;
    }
    else
    {
        a->state.hitbox_state = 1;
        if (a->mechanism.changed && !ActorSound(s, 0x69d))
            return false;
        a->mechanism.changed = false;
    }
    ShiftY(a, down ? 1 : -1);
    if ((down && Pixel(a->state.y) > target) || (!down && Pixel(a->state.y) < target))
    {
        ShiftY(a, (Sint64)target - Pixel(a->state.y));
        if (!puzzle && down)
            a->state.hitbox_state = 0;
    }
    if (puzzle)
        s->puzzle_height[a->state.parameters] = (Sint16)(480 - Pixel(a->state.y));
    return Sprite(s, a, 40, 200, 32, 32, -16, -16);
}
static bool Lift(SotnActors *s, Actor *a)
{
    const bool main = a->state.id == 0x47;
    Uint16 contact = 0;
    if (!ActorObjectCollision(s, a, 16, 5, 4, &contact))
        return false;
    SotnDataView positions = {0};
    if (main && !Slice(s->nz0, 0x13a8, 8, &positions))
        return false;
    if (a->state.step == 0)
    {
        if (!Initialize(s, a, main ? 32 : 3, 0x72))
            return false;
        a->state.hitbox_y = 68;
        if (main)
        {
            if (a->state.parameters < 1 || a->state.parameters > 3 || s->elevator_target > 3)
                return SDL_SetError("Laboratory lift stop is outside 1..3");
            a->state.hitbox_width = 16;
            a->state.hitbox_height = 5;
            a->state.hitbox_state = 1;
            if (s->elevator_target)
                a->state.y = Signed16(positions.bytes + s->elevator_target * 2) * 65536;
            else
                s->elevator_target = a->state.parameters;
            a->mechanism.target = s->elevator_target;
        }
        else
            s->call_elevator = false;
    }
    if (main)
    {
        if (s->elevator_target < 1 || s->elevator_target > 3)
            return SDL_SetError("Laboratory lift lost its target stop");
        if (a->state.step == 1)
        {
            if (contact)
            {
                if (s->buttons & SOTN_BUTTON_UP)
                    s->elevator_target = (Uint16)SDL_max(1, s->elevator_target - 1);
                if (s->buttons & SOTN_BUTTON_DOWN)
                    s->elevator_target = (Uint16)SDL_min(3, s->elevator_target + 1);
            }
            if (s->elevator_target != a->mechanism.target)
            {
                const Sint32 target = Signed16(positions.bytes + s->elevator_target * 2);
                a->state.step = target < Pixel(a->state.y) ? 2 : 3;
                if (!ActorSound(s, 0x6e6))
                    return false;
            }
            a->mechanism.target = s->elevator_target;
        }
        else
        {
            const Sint32 target = Signed16(positions.bytes + a->mechanism.target * 2);
            const Sint32 dy = a->state.step == 2 ? -1 : 1;
            ShiftY(a, dy);
            if ((dy < 0 && Pixel(a->state.y) < target) || (dy > 0 && Pixel(a->state.y) > target))
            {
                ShiftY(a, (Sint64)target - Pixel(a->state.y));
                a->state.step = 1;
            }
            else if (contact && !ActorDisplace(s, 0, dy * 65536, 0))
                return false;
        }
    }
    else
    {
        const Sint32 target = s->call_elevator ? 96 : 216, dy = s->call_elevator ? -1 : 1;
        ShiftY(a, dy);
        if ((dy < 0 && Pixel(a->state.y) < target) || (dy > 0 && Pixel(a->state.y) > target))
            ShiftY(a, (Sint64)target - Pixel(a->state.y));
        else if (contact && !ActorDisplace(s, 0, dy * 65536, 0))
            return false;
    }
    if (!Sprite(s, a, 8, 128, 32, 72, -16, 0))
        return false;
    for (Uint16 i = 1; i < a->state.primitive_count; ++i)
        if (!ActorSprite(s, a, i, a->state.x - 3 * 65536, a->state.y - (56 + (i - 1) * 64) * 65536,
                         0, 128, 8, 64, 15, 9))
            return false;
    return true;
}
static bool Box(SotnActors *s, Actor *a)
{
    Uint16 contact = 0, collision = 0;
    if (!ActorObjectCollision(s, a, 16, 16, 5, &contact))
        return false;
    if (a->state.step == 0 && !Initialize(s, a, 1, 112))
        return false;
    a->vx = a->vy = 0;
    if (contact & 1)
    {
        if (Side(a, s->player) && s->player.velocity_x > 0)
            a->vx = 32768;
        if (!Side(a, s->player) && s->player.velocity_x < 0)
            a->vx = -32768;
        if (a->vx && (s->ticks & 7) == 0 && !ActorSound(s, 0x608))
            return false;
    }
    if (!ActorWalk(s, a, 0xeb8, &collision))
        return false;
    if (a->state.parameters == 0)
    {
        const Sint32 x = Pixel(a->state.x);
        Uint16 modifier = SDL_abs(x - 192) < 24 ? 1 : 0;
        if (SDL_abs(x - 256) < 24)
            modifier = 2;
        // The PS1 code reads one element before its two-item array for modifier 0.
        // Native slot 0 is an explicit zero sentinel, never an out-of-bounds read.
        if (a->mechanism.target == 0 && s->puzzle_height[modifier])
        {
            a->state.x -= a->vx;
            modifier = 0;
        }
        a->mechanism.target = modifier;
        if (modifier)
            a->state.y = (448 - s->puzzle_height[modifier]) * 65536;
    }
    return Sprite(s, a, 8, 200, 32, 32, -16, -16);
}
static bool Secret(SotnActors *s, Actor *a)
{
    const bool wall = a->state.id == 0x17;
    const Uint32 flag = wall ? SOTN_LAB_SECRET_WALL : SOTN_LAB_SECRET_FLOOR;
    const Uint32 index = wall ? 0x260 : 0x2e7, rows = wall ? 4 : 2;
    const size_t data = wall ? 0xe54 : 0xe94, bytes = wall ? 16 : 8;
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xbf8))
            return false;
        a->state.hitbox_width = 16;
        a->state.hitbox_height = wall ? 32 : 16;
        a->state.hitbox_state = 2;
        if (!ActorTilePatch(s, index, rows, 2, 16, data + (s->world->flags[flag] ? 3 : 0) * bytes))
            return false;
        if (s->world->flags[flag])
        {
            a->remove = true;
            return true;
        }
    }
    if (a->state.step == 1 && a->hit_params)
    {
        if (!ActorSound(s, 0x644))
            return false;
        a->state.step = 2;
    }
    else if (a->state.step == 2)
    {
        if (++a->mechanism.count > 3)
            return SDL_SetError("Secret obstacle break count is invalid");
        if (!ActorTilePatch(s, index, rows, 2, 16, data + a->mechanism.count * bytes) ||
            !ActorExplosion(s, a, wall ? 0x13 : 0x11))
            return false;
        a->timer = 32;
        a->state.step = 3;
        if (a->mechanism.count == 3)
        {
            s->world->flags[flag] = 1;
            if (wall)
                for (Uint32 i = 0; i < 8; ++i)
                {
                    Actor *piece = NULL;
                    if (!ActorSpawn(s, a, 0x22, 0, 160, 192, &piece))
                        return false;
                    if (piece != NULL)
                    {
                        piece->state.x += (Sint32)(ActorRandom(s) & 15) * 65536;
                        ShiftY(piece, (Sint32)(ActorRandom(s) & 63) - 32);
                    }
                }
            a->remove = true;
        }
    }
    else if (a->state.step == 3 && --a->timer == 0)
        a->state.step = 1;
    return true;
}
static bool Debris(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xc34))
            return false;
        a->state.rotated = true;
        a->state.frame = (ActorRandom(s) & 1) ? 1 : 2;
        Sint32 range = (Sint32)(ActorRandom(s) & 31) + 16,
               angle = (Sint32)(ActorRandom(s) & 63) * 16 + 0xc00;
        if (a->state.parameters)
        {
            a->state.frame = 3;
            range = (Sint32)(ActorRandom(s) & 31) + 16;
            angle = (Sint32)ActorRandom(s) * 6 + 0x900;
        }
        a->vx = range * ActorSin(angle + 1024);
        a->vy = range * ActorSin(angle);
        a->state.facing_left = a->vx < 0;
    }
    a->state.x += a->vx;
    a->state.y += a->vy;
    a->state.rotation = (Sint16)Wrap16(a->state.rotation + (a->state.parameters ? 64 : 32));
    a->vy += 8192;
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + 6, &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        ShiftY(a, c.top);
        if (!a->state.parameters)
        {
            if (!ActorSound(s, 0x644))
                return false;
            for (Uint32 i = 0; i < 2; ++i)
            {
                Actor *piece = NULL;
                if (!ActorSpawn(s, a, 0x22, 1, 160, 192, &piece))
                    return false;
            }
            a->remove = true;
        }
        else if (a->vy < 32768)
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 6, 16, 160, 192, &effect))
                return false;
            a->remove = true;
        }
        else
            a->vy = (Sint32)(-(Sint64)a->vy * 2 / 3);
    }
    return true;
}
bool ActorMechanism(SotnActors *s, Actor *a)
{
    switch (a->state.id)
    {
    case 0x22:
        return Debris(s, a);
    case 0x19:
    case 0x20:
    case 0x24:
    case 0x48:
        return Button(s, a);
    case 0x1a:
    case 0x25:
        return Spikes(s, a);
    case 0x21:
    case 0x47:
        return Lift(s, a);
    case 0x1b:
        return Box(s, a);
    case 0x17:
    case 0x18:
        return Secret(s, a);
    default:
        return SDL_SetError("Unknown Laboratory room mechanism");
    }
}
