// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted selected intact/active behaviors from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: src/st/nz0/e_breakable.c,
// e_red_eye_bust.c, e_nz0_room2.c; src/st/e_bloody_zombie.h,
// animate_entity.h, st_common.h, and st_update.h. No game tables are embedded.
#include <SDL3/SDL_error.h>
#include <sotn/actors.h>

typedef struct Actor
{
    SotnActorState state;
    Sint32 vx, vy;
    Uint16 pose, remaining, timer, substep;
    Uint8 parallax;
} Actor;

struct SotnActors
{
    SotnDataView dra, nz0;
    SotnCollisionMap map;
    SotnRoomActors *spawns;
    Actor slots[SOTN_STAGE_ACTOR_SLOTS];
    Sint32 camera_x, camera_y;
    Uint32 random;
    bool entered;
};

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Read32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static Sint32 Signed16(const Uint8 *bytes)
{
    const Uint16 value = Read16(bytes);
    return value < 32768 ? value : (Sint32)value - 65536;
}

static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}

static Sint32 Wrap16(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}

static void ShiftY(Actor *actor, Sint64 delta)
{
    const Sint32 high = Wrap16((Sint64)Pixel(actor->state.y) + delta);
    actor->state.y = (Sint32)((Sint64)high * 65536 + ((Uint32)actor->state.y & 65535));
}

static bool Slice(SotnDataView image, size_t offset, size_t size, SotnDataView *out)
{
    if (image.bytes == NULL || offset > image.size || size > image.size - offset)
        return SDL_SetError("Laboratory actor table is outside its private image");
    *out = (SotnDataView){image.bytes + offset, size};
    return true;
}

static void SetStep(Actor *actor, Uint16 step)
{
    actor->state.step = step;
    actor->substep = actor->pose = actor->remaining = 0;
}

static bool Initialize(SotnActors *actors, Actor *actor, size_t offset)
{
    SotnDataView init = {0}, api = {0}, enemy = {0};
    if (!Slice(actors->nz0, offset, 10, &init) || !Slice(actors->dra, 0x98, 4, &api))
        return false;
    const Uint32 address = Read32(api.bytes), id = Read16(init.bytes + 8);
    if (address < 0x800a0000 || address % 4 != 0 || id >= 400 ||
        !Slice(actors->dra, (size_t)(address - 0x800a0000) + (size_t)id * 40, 40, &enemy))
        return SDL_SetError(
            "Laboratory actor initialization references an invalid enemy definition");
    actor->state.animation_set = Read16(init.bytes);
    actor->state.frame = Read16(init.bytes + 2);
    actor->state.texture_offset = Read16(init.bytes + 4);
    actor->state.palette = Read16(init.bytes + 6);
    actor->state.flags = Read32(enemy.bytes + 36);
    actor->state.priority = 0x74;
    ++actor->state.step;
    actor->substep = 0;
    return true;
}

static bool Animate(SotnActors *actors, Actor *actor, size_t offset, Uint8 *result)
{
    SotnDataView record = {0};
    if (actor->pose >= 512 || !Slice(actors->nz0, offset + (size_t)actor->pose * 2, 2, &record))
        return SDL_SetError("Laboratory actor animation exceeds its bounded frame stream");
    *result = 0;
    if (actor->remaining == 0)
    {
        if (record.bytes[0] == 255)
            return true;
        if (record.bytes[0] == 0)
        {
            if (!Slice(actors->nz0, offset, 2, &record) || record.bytes[0] == 0 ||
                record.bytes[0] == 255)
                return SDL_SetError("Laboratory actor animation cannot loop to a control record");
            actor->remaining = record.bytes[0];
            actor->state.frame = record.bytes[1];
            actor->pose = 1;
            return true;
        }
        actor->remaining = record.bytes[0];
        actor->state.frame = record.bytes[1];
        ++actor->pose;
        *result = 0x80;
    }
    if (actor->pose == 0 || !Slice(actors->nz0, offset + ((size_t)actor->pose - 1) * 2, 2, &record))
        return SDL_SetError("Laboratory actor animation has an invalid cursor");
    --actor->remaining;
    actor->state.frame = record.bytes[1];
    *result |= 1;
    return true;
}

static bool Query(SotnActors *actors, Sint32 x, Sint32 y, SotnCollider *out)
{
    return SotnCollision_Query(&actors->map, x, y, out);
}

static bool Ground(SotnActors *actors, Actor *actor, bool *landed)
{
    SotnDataView sensors = {0};
    if (!Slice(actors->nz0, 0x22d4, 16, &sensors))
        return false;
    actor->state.x += actor->vx;
    actor->state.y += actor->vy;
    if (actor->vy < 6 * 65536)
        actor->vy += 16384;
    *landed = false;
    if (actor->vy >= 0)
    {
        Sint32 x = Pixel(actor->state.x), y = Pixel(actor->state.y);
        for (Uint32 i = 0; i < 4; ++i)
        {
            x = Wrap16((Sint64)x + Signed16(sensors.bytes + i * 4));
            y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
            SotnCollider col = {0}, back = {0};
            if (!Query(actors, x, y, &col))
                return false;
            if ((col.effects & SOTN_EFFECT_SLOPE_FLOOR) && i == 1)
            {
                if (col.effects & SOTN_EFFECT_SOLID)
                {
                    if (!Query(actors, x, y - 8, &back))
                        return false;
                    if (!(back.effects & SOTN_EFFECT_SOLID))
                    {
                        ShiftY(actor, (Sint64)4 + col.top);
                        actor->vx = actor->vy = 0;
                        actor->state.flags &= ~0x10000000U;
                        *landed = true;
                        return true;
                    }
                }
                continue;
            }
            if ((col.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK)) && i != 1)
            {
                if (col.effects & SOTN_EFFECT_SINK)
                {
                    actor->state.flags &= ~0x10000000U;
                    return true;
                }
                if (!Query(actors, x, y - 8, &back))
                    return false;
                if (!(back.effects & SOTN_EFFECT_SOLID))
                {
                    ShiftY(actor, col.top);
                    actor->vx = actor->vy = 0;
                    actor->state.flags &= ~0x10000000U;
                    *landed = true;
                    return true;
                }
            }
        }
    }
    actor->state.flags |= 0x10000000U;
    return true;
}

static bool WalkCollision(SotnActors *actors, Actor *actor)
{
    SotnDataView sensors = {0};
    if (!Slice(actors->nz0, 0x22e4, 8, &sensors))
        return false;
    actor->state.x += actor->vx;
    ShiftY(actor, 3);
    Sint32 x = Wrap16((Sint64)Pixel(actor->state.x) + Signed16(sensors.bytes));
    Sint32 y = Wrap16((Sint64)Pixel(actor->state.y) + Signed16(sensors.bytes + 2));
    SotnCollider col = {0};
    if (!Query(actors, x, y, &col))
        return false;
    if (!(col.effects & SOTN_EFFECT_SOLID))
        return true;
    ShiftY(actor, col.top);
    if (actor->vx == 0)
        return true;
    const Sint32 sensor_x = Signed16(sensors.bytes + 4);
    x = Wrap16((Sint64)x + (actor->vx < 0 ? -sensor_x : sensor_x));
    y = Wrap16((Sint64)y + Signed16(sensors.bytes + 6) - 7);
    if (!Query(actors, x, y, &col))
        return false;
    if (col.effects & SOTN_EFFECT_SOLID)
    {
        if ((col.effects & SOTN_EFFECT_SLOPE_FLOOR) || !(col.effects & SOTN_EFFECT_SIDE))
            return true;
        actor->state.x -= actor->vx;
        actor->vx = 0;
        return true;
    }
    y = Wrap16((Sint64)y + 15);
    if (!Query(actors, x, y, &col))
        return false;
    if (!(col.effects & SOTN_EFFECT_SOLID))
    {
        actor->state.x -= actor->vx;
        actor->vx = 0;
    }
    return true;
}

static Uint8 Side(const Actor *actor, SotnPlayerState player)
{
    return (Uint8)(Pixel(actor->state.x) > Pixel(player.x));
}

static Sint64 Distance(const Actor *actor, SotnPlayerState player)
{
    const Sint64 difference = (Sint64)Pixel(actor->state.x) - Pixel(player.x);
    return difference < 0 ? -difference : difference;
}

static bool Zombie(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    Uint8 animation = 0;
    switch (actor->state.step)
    {
    case 0:
        if (!Initialize(actors, actor, 0xc7c))
            return false;
        SetStep(actor, 2);
        break;
    case 2: {
        bool landed = false;
        if (!Ground(actors, actor, &landed))
            return false;
        if (landed)
            SetStep(actor, 1);
        break;
    }
    case 1:
        if (actor->substep == 0)
        {
            actor->timer = 128;
            actor->substep = 1;
        }
        if (!Animate(actors, actor, 0x22ec, &animation) || !WalkCollision(actors, actor))
            return false;
        actor->vx = actor->state.facing_left ? 24576 : -24576;
        if (--actor->timer == 0)
        {
            actor->timer = 128;
            actor->state.facing_left = !actor->state.facing_left;
        }
        actors->random = actors->random * 0x01010101U + 1U;
        if (player.facing_left == (Side(actor, player) != 0) && Distance(actor, player) < 128)
        {
            actor->state.facing_left = Side(actor, player) == 0;
            SetStep(actor, 3);
        }
        break;
    case 3:
        if (!Animate(actors, actor, 0x237c, &animation))
            return false;
        if (animation == 0)
            actor->state.facing_left = Side(actor, player) == 0;
        if (!WalkCollision(actors, actor))
            return false;
        actor->vx = actor->state.facing_left ? 49152 : -49152;
        actors->random = actors->random * 0x01010101U + 1U;
        if (Distance(actor, player) < 40)
            SetStep(actor, 4);
        break;
    case 4:
        if (!Animate(actors, actor, 0x22fc, &animation))
            return false;
        if (animation == 0)
            SetStep(actor, 1);
        break;
    default:
        return SDL_SetError("Bloody Zombie entered an unintegrated damage/death state");
    }
    return true;
}

static bool Behavior(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    Uint8 animation = 0;
    switch (actor->state.id)
    {
    case 1: {
        const Uint32 type = actor->state.parameters >> 12;
        SotnDataView table = {0};
        if (type >= 3 || !Slice(actors->nz0, 0xe04 + type * 4, 4, &table))
            return SDL_SetError("Breakable actor references an unsupported original subtype");
        if (actor->state.step != 0)
        {
            const Uint32 address = Read32(table.bytes);
            if (address < 0x80180000)
                return SDL_SetError("Breakable actor animation address is invalid");
            return Animate(actors, actor, address - 0x80180000, &animation);
        }
        if (!Initialize(actors, actor, 0xbc8) || !Slice(actors->nz0, 0xe34 + type * 2, 2, &table))
            return false;
        actor->state.animation_set = Read16(table.bytes);
        if (!Slice(actors->nz0, 0xe44 + type, 1, &table))
            return false;
        actor->state.blend_mode = table.bytes[0];
        actor->state.priority = 0x6c;
        if (type == 2)
        {
            actor->state.texture_offset = 0x4b;
            actor->state.palette = 0x219;
        }
        return true;
    }
    case 0x23:
        if (actor->state.step == 0)
        {
            if (!Initialize(actors, actor, 0xc34))
                return false;
            actor->state.frame = 7;
            actor->state.priority = 0x70;
        }
        return true;
    case 0x34:
        if (actor->state.step == 0)
        {
            if (!Initialize(actors, actor, 0xcc4))
                return false;
            actor->state.priority = 0x6a;
        }
        return Animate(actors, actor, 0xef0, &animation);
    case 0x2b:
        return Zombie(actors, actor, player);
    default:
        actor->state.unsupported = true;
        return true;
    }
}

bool SotnActors_Create(SotnDataView dra, SotnDataView nz0, SotnActors **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (dra.bytes == NULL || dra.size < 0x9c || dra.size > 0x200000 || nz0.bytes == NULL ||
        nz0.size < 40 || nz0.size > 0x80000)
        return SDL_SetError("Laboratory actors require bounded private US DRA/NZ0 images");
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    if (actors == NULL)
        return false;
    actors->dra = dra;
    actors->nz0 = nz0;
    if (!SotnRoomActors_Create(&actors->spawns))
    {
        SotnActors_Destroy(actors);
        return false;
    }
    *out = actors;
    return true;
}

bool SotnActors_Enter(SotnActors *actors, const SotnActorLayout *layout, SotnCollisionMap map,
                      Sint32 camera_x, Sint32 camera_y)
{
    if (actors == NULL || map.width == 0 || map.height == 0 || map.width > 1024 ||
        map.height > 1024)
        return SDL_SetError("Laboratory actors require a bounded room collision map");
    SotnCollider probe = {0};
    if (!SotnCollision_Query(&map, 0, 0, &probe) ||
        !SotnRoomActors_Enter(actors->spawns, layout, camera_x, camera_y))
        return false;
    actors->map = map;
    actors->camera_x = camera_x;
    actors->camera_y = camera_y;
    actors->entered = true;
    SDL_memset(actors->slots, 0, sizeof(actors->slots));
    return true;
}

bool SotnActors_Tick(SotnActors *actors, SotnPlayerState player, Sint32 camera_x, Sint32 camera_y)
{
    if (actors == NULL || !actors->entered || camera_x < 0 || camera_x > 16384 || camera_y < 0 ||
        camera_y > 16384)
        return SDL_SetError("Laboratory actor tick requires an entered room and bounded camera");
    const Sint64 min_position = -256LL * 65536;
    const Sint64 max_x = ((Sint64)actors->map.width * 16 + 256) * 65536;
    const Sint64 max_y = ((Sint64)actors->map.height * 16 + 256) * 65536;
    if (player.x < min_position || player.x > max_x || player.y < min_position || player.y > max_y)
        return SDL_SetError("Laboratory actors require a player within the bounded current room");
    const Sint32 dx = camera_x - actors->camera_x, dy = camera_y - actors->camera_y;
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        Actor *actor = &actors->slots[i];
        if (actor->state.generation == 0 || actor->state.step == 0 || actor->state.unsupported)
            continue;
        const Sint64 scale = actor->parallax != 0                ? 65536 - actor->parallax * 256
                             : (actor->state.flags & 0x08000000) ? 0
                                                                 : 65536;
        const Sint64 x = actor->state.x + dx * scale, y = actor->state.y + dy * scale;
        if (x < -1024LL * 65536 || x > 17408LL * 65536 || y < -1024LL * 65536 ||
            y > 17408LL * 65536)
            return SDL_SetError("Laboratory actor left the bounded room region");
        actor->state.x = (Sint32)x;
        actor->state.y = (Sint32)y;
    }
    if (!SotnRoomActors_Scroll(actors->spawns, camera_x, camera_y))
        return false;
    actors->camera_x = camera_x;
    actors->camera_y = camera_y;
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        SotnActorPlacement placement = {0};
        Uint32 generation = 0;
        if (!SotnRoomActors_Get(actors->spawns, i, &placement, &generation))
            return false;
        Actor *actor = &actors->slots[i];
        if (generation == 0)
        {
            *actor = (Actor){0};
            continue;
        }
        if (actor->state.generation != generation)
        {
            *actor = (Actor){0};
            actor->state.generation = generation;
            actor->state.id = placement.id;
            actor->state.parameters = placement.parameters;
            actor->state.x = (Sint32)placement.x * 65536;
            actor->state.y = (Sint32)placement.y * 65536;
            actor->parallax = placement.variant;
        }
        const Sint32 x = Pixel(actor->state.x) - camera_x, y = Pixel(actor->state.y) - camera_y;
        if (actor->state.step != 0)
        {
            const bool barely = (actor->state.flags & 0x40000000) != 0;
            const Sint32 near = barely ? -64 : -128, far_x = barely ? 320 : 384,
                         far_y = barely ? 288 : 352;
            if ((actor->state.flags & 0x80000000) &&
                (x < near || x > far_x || y < near || y > far_y))
            {
                if (!SotnRoomActors_Release(actors->spawns, i, false))
                    return false;
                *actor = (Actor){0};
                continue;
            }
            if ((actor->state.flags & 0x20000000) && !(actor->state.flags & 0x10000000) &&
                (x < -64 || x > 320 || y < -64 || y > 288))
                continue;
        }
        Actor next = *actor;
        if (!next.state.unsupported && !Behavior(actors, &next, player))
            return false;
        if ((Sint64)next.state.x < -1024LL * 65536 || (Sint64)next.state.x > 17408LL * 65536 ||
            (Sint64)next.state.y < -1024LL * 65536 || (Sint64)next.state.y > 17408LL * 65536)
            return SDL_SetError("Laboratory actor motion left the bounded room region");
        *actor = next;
    }
    return true;
}

bool SotnActors_Get(const SotnActors *actors, Uint32 slot, SotnActorState *out)
{
    if (actors == NULL || out == NULL || slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Laboratory actor slot or output is invalid");
    *out = actors->slots[slot].state;
    return true;
}

void SotnActors_Destroy(SotnActors *actors)
{
    if (actors != NULL)
    {
        SotnRoomActors_Destroy(actors->spawns);
        SDL_free(actors);
    }
}
