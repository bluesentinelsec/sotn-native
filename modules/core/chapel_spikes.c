// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected DAI/NO2 st/e_spikes.h and st_common.h at sotn-decomp 7bc54d8157b6.
// Initialization, orientation, animation, artwork and sine data remain private.
#include "actors_internal.h"
static bool Move(Actor *a, Sint32 gravity)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 vy = (Sint64)a->vy + gravity;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        vy < SDL_MIN_SINT32 || vy > SDL_MAX_SINT32)
        return SDL_SetError("Chapel spike debris exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy = (Sint32)vy;
    return true;
}
static Actor *Parent(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || !s->slots[slot].state.generation ||
        s->slots[slot].state.id != (s->stage_id == 4 ? 0x21 : 0x2e) ||
        s->slots[slot].state.generation != a->enemy_state.owner_generation)
        return NULL;
    return &s->slots[slot];
}
static bool Damage(SotnActors *s, Actor *a)
{
    if (!Parent(s, a))
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, ActorStageOffset(s, 0xbf8)))
            return false;
        a->attack_element = s->stage_id == 4 ? 0x50 : 0x51;
        a->attack = 15;
        a->state.hitbox_state = 1;
        a->state.hitbox_width = a->state.hitbox_height = 4;
        a->remaining = 4;
    }
    else if (s->stage_id == 4 && a->state.step == 1)
        a->remove = true;
    return a->state.step == 1 || SDL_SetError("Chapel spike damage entered an invalid state");
}
static bool Dust(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        Sint16 angle = 0;
        if (!ActorInitialize(s, a, ActorStageOffset(s, 0xbe0)) ||
            !SotnMath_Atan2(s->executable, Wrap16((Sint64)Pixel(s->player.y) - Pixel(a->state.y)),
                            Wrap16((Sint64)Pixel(s->player.x) - Pixel(a->state.x)), &angle))
            return false;
        const Uint8 phase = (Uint8)(ActorShift(angle, 4) + 64);
        SotnDataView x = {0}, y = {0};
        const size_t sine = s->stage_id == 4 ? 0x142c : 0x1474;
        if (!Slice(s->nz0, sine + phase * 2U, 2, &x) ||
            !Slice(s->nz0, sine + (Uint8)(phase - 64) * 2U, 2, &y))
            return false;
        const Sint32 sx = Signed16(x.bytes), sy = Signed16(y.bytes);
        if (sx < -4096 || sx > 4096 || sy < -4096 || sy > 4096)
            return SDL_SetError("Chapel spike dust sine exceeds its private amplitude");
        a->vx = sx * 40;
        a->vy = sy * 40;
        a->state.priority = 160;
        a->state.animation_set = 8;
        a->state.frame = 1;
        a->state.palette = 0x8161;
        return true;
    }
    Uint8 animation = 0;
    if (a->state.step != 1 || !Move(a, 0) ||
        !ActorAnimate(s, a, s->stage_id == 4 ? 0xd38 : 0xea4, &animation))
        return SDL_SetError("Chapel spike dust has invalid animation or motion state");
    a->remove = animation == 0;
    return true;
}
static bool Parts(SotnActors *s, Actor *a)
{
    const bool quarters = s->stage_id == 4;
    const Uint16 direction = quarters ? a->state.parameters : a->state.parameters >> 8,
                 orientation = quarters ? 0 : a->state.parameters & 255U;
    if (direction > 2 || orientation > 15)
        return SDL_SetError("Chapel spike debris has invalid orientation or launch parameters");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, quarters ? 0x8a4 : 0x9b0))
            return false;
        if (!quarters)
            a->state.frame = 15;
        a->state.rotated = true;
        a->state.flags |= 0xc0000000U;
        a->state.priority = 160;
        a->vx = a->vy = 0;
        if (quarters)
            a->vy = 49152;
        a->state.rotation = a->enemy_state.angle_delta = 0;
        if (direction & 1U)
        {
            a->vx = 32768;
            a->state.rotation += 16;
            a->enemy_state.angle_delta += 8;
        }
        if (direction & 2U)
        {
            a->vx = -32768;
            a->state.rotation -= 16;
            a->enemy_state.angle_delta -= 8;
        }
        if (orientation & 2U)
        {
            a->vx -= 49152;
            a->state.rotation -= 1024;
            a->enemy_state.angle_delta += 64;
        }
        if (orientation & 1U)
        {
            a->vx += 49152;
            a->state.rotation += 1024;
            a->enemy_state.angle_delta -= 64;
        }
        if (orientation & 4U)
        {
            a->vy += 49152;
            a->state.rotation -= 2048;
        }
        if (orientation & 8U)
            a->vy -= 163840;
        a->vx += (Sint32)(ActorRandom(s) & 3U) * 8192 - 12288;
        a->vy += (Sint32)(ActorRandom(s) & 3U) * 8192 - 12288;
        a->enemy_state.angle_delta += (Sint16)((Sint32)(ActorRandom(s) & 3U) * 16 - 24);
        return true;
    }
    if (a->state.step != 1 || !Move(a, 10240))
        return SDL_SetError("Chapel spike debris has invalid motion state");
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + a->enemy_state.angle_delta);
    SotnCollider c = {0};
    if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y), &c))
        return false;
    if (c.effects)
    {
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            a->vy = -a->vy / 2;
            a->enemy_state.angle_delta = (Sint16)Wrap16((Sint32)a->enemy_state.angle_delta * 4);
        }
        if (c.effects & SOTN_EFFECT_SIDE)
        {
            if (a->vx == SDL_MIN_SINT32)
                return SDL_SetError("Chapel spike debris cannot reverse its minimum velocity");
            a->vx = -a->vx;
        }
        a->remove = SDL_abs(a->vy) < 12288;
    }
    return true;
}
static bool TileType(SotnActors *s, Sint64 index, Uint8 *out)
{
    if (index < 0 || (Uint64)index >= (Uint64)s->map.width * s->map.height ||
        (Uint64)index * 2U + 2U > s->map.tiles.size)
        return SDL_SetError("Chapel spike scan exceeds the owned tile map");
    const Uint16 tile = Read16(s->map.tiles.bytes + (size_t)index * 2U);
    if (tile >= s->map.types.size)
        return SDL_SetError("Chapel spike tile has no private collision definition");
    *out = s->map.types.bytes[tile];
    return true;
}
static bool Orientation(SotnActors *s, Uint32 index, Uint16 *out)
{
    SotnDataView recipe = {0};
    if (!Slice(s->nz0, 0xeb4, 9, &recipe))
        return false;
    Uint16 orientation = 0;
    for (Uint16 row = 0; row < 3; ++row)
        for (Uint16 column = 0; column < 3; ++column)
        {
            Uint8 collision = 0;
            if (recipe.bytes[row * 3U + column] > 15 ||
                !TileType(s, (Sint64)index + ((Sint32)row - 1) * 80 + column - 1, &collision))
                return SDL_SetError("Chapel spike orientation exceeds its private neighborhood");
            if (collision == 3)
                orientation |= recipe.bytes[row * 3U + column];
        }
    *out = orientation;
    return true;
}
static bool Spawn(SotnActors *s, const Actor *a, Uint16 id, Uint16 parameters, Uint32 index)
{
    Actor *effect = NULL;
    const bool damage = s->stage_id == 4 && id == 0x31;
    const Uint32 width = s->stage_id == 4 ? 96 : 80;
    const Sint64 x = ((Sint64)(index % width) * 16 + 8) * 65536,
                 y = ((Sint64)(index / width) * 16 + 8) * 65536;
    if (x > SDL_MAX_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Spike effect tile %u exceeds fixed-point position bounds", index);
    if (!ActorSpawn(s, a, id, parameters, damage ? 96 : 160, damage ? 128 : 192, &effect))
        return false;
    if (effect)
    {
        effect->state.x = (Sint32)x;
        effect->state.y = (Sint32)y;
        effect->state.facing_left = false;
        if (damage)
        {
            effect->enemy_state.owner_slot = a->slot;
            effect->enemy_state.owner_generation = a->state.generation;
        }
    }
    return true;
}
static bool Spikes(SotnActors *s, Actor *a)
{
    if (s->map.width != 80 || !s->mutable_tiles)
        return SDL_SetError("Chapel spikes require their original eighty-tile-wide mutable room");
    if (!a->state.step)
    {
        Actor *child = NULL;
        if (!ActorInitialize(s, a, 0x95c) ||
            !ActorOwnedSpawnAt(s, a, 0x31, 0, (Uint32)a->slot + 1U, &child))
            return false;
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Pixel(a->state.y) * 65536;
        child->state.facing_left = false;
        a->chapel.child_generation = child->state.generation;
        return true;
    }
    if (a->state.step != 1 || (Uint32)a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Chapel spikes entered an invalid state or child reservation");
    Actor *child = &s->slots[a->slot + 1U];
    if (child->state.id != 0x31 || child->state.generation != a->chapel.child_generation ||
        Parent(s, child) != &s->slots[a->slot])
        return SDL_SetError("Chapel spikes lost their owned damage actor");
    child->state.x = (s->camera_x - 16) * 65536;
    child->state.y = (s->camera_y - 16) * 65536;
    const Sint32 x = Pixel(s->player.x), y = Pixel(s->player.y);
    // Room crossings may overshoot the map; never reproduce an out-of-bounds PS1 read.
    if (x < 0 || x >= 80 * 16 || y < 0 || y >= (Sint32)s->map.height * 16)
        return true;
    SotnCharacterState progress = {0};
    if (s->character && !SotnCharacter_Get(s->character, &progress, NULL))
        return false;
    const bool breaker = s->character && progress.equipment[SOTN_EQUIP_ARMOR] == 14;
    Uint32 indices[3] = {0};
    Uint16 orientation[3] = {0};
    Uint16 count = 0;
    for (Sint32 row = y / 16 - 1; row <= y / 16 + 1; ++row)
    {
        if (row < 0 || (Uint32)row >= s->map.height)
            continue;
        const Uint32 index = (Uint32)row * 80U + (Uint32)x / 16U;
        Uint8 collision = 0;
        if (!TileType(s, index, &collision))
            return false;
        if (collision > 243 && collision < 248)
        {
            indices[count] = index;
            if (breaker && !Orientation(s, index, &orientation[count]))
                return false;
            ++count;
        }
    }
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint32 index = indices[i];
        if (!breaker)
        {
            child->state.x = (Sint32)(index % 80U * 16U + 8U) * 65536;
            child->state.y = (Sint32)(index / 80U * 16U + 8U) * 65536;
            continue;
        }
        if (!ActorTile(s, index, 0))
            return false;
        for (Uint16 part = 0; part < 3; ++part)
            if (!Spawn(s, a, 0x2f, (Uint16)(orientation[i] + part * 256U), index))
                return false;
        if (!Spawn(s, a, 6, 16, index) || !Spawn(s, a, 0x30, 0, index) || !ActorSound(s, 0x652))
            return false;
    }
    return true;
}
static bool QuartersSpikes(SotnActors *s, Actor *a)
{
    if (!s->map.width || !s->map.height || !s->mutable_tiles)
        return SDL_SetError("NO2 spikes require an owned mutable tile map");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x844))
            return false;
        s->backdrop_color = 2U | (2U << 5) | (2U << 10);
        // NO2 scans on initialization; DAI deliberately waits until the next tick.
    }
    if (a->state.step != 1)
        return SDL_SetError("NO2 spikes entered an invalid scan state");
    const Sint32 x = Pixel(s->player.x), y = Pixel(s->player.y);
    if (x < 0 || x >= (Sint32)s->map.width * 16 || y < 0 || y >= (Sint32)s->map.height * 16)
        return true;
    SotnCharacterState progress = {0};
    if (s->character && !SotnCharacter_Get(s->character, &progress, NULL))
        return false;
    const bool breaker = s->character && progress.equipment[SOTN_EQUIP_ARMOR] == 14;
    Uint32 indices[3] = {0};
    Uint16 replacements[3] = {0}, count = 0;
    // Original initial indexing uses actual hSize, then a fixed 96-tile scan stride.
    // Room 15 is 112 tiles wide. Neither scanning nor spawn placement may normalize it.
    const Sint64 first = x / 16 + (Sint64)(y / 16) * s->map.width - 96;
    for (Uint16 i = 0; i < 3; ++i)
    {
        const Sint64 index = first + i * 96U;
        // Safe native concession for room-edge overshoot: omit out-of-map reads.
        if (index < 0 || (Uint64)index >= (Uint64)s->map.width * s->map.height)
            continue;
        Uint8 collision = 0;
        if (!TileType(s, index, &collision))
            return false;
        if (collision <= 243 || collision >= 248)
            continue;
        indices[count] = (Uint32)index;
        if (breaker)
        {
            const Uint16 tile = Read16(s->map.tiles.bytes + (size_t)index * 2U);
            // The original switch leaves unknown replacements uninitialized.
            // Preflight every tile before changing any room-owned data.
            if (tile < 0x6ae || tile > 0x6b0 || (size_t)tile + 3U >= s->map.types.size)
                return SDL_SetError("NO2 spike tile %u has no valid original broken replacement",
                                    tile);
            replacements[count] = (Uint16)(tile + 3U);
        }
        ++count;
    }
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint32 index = indices[i];
        if (!breaker)
        {
            if (!Spawn(s, a, 0x31, 0, index))
                return false;
            continue;
        }
        if (!ActorTile(s, index, replacements[i]))
            return false;
        for (Uint16 part = 0; part < 3; ++part)
            if (!Spawn(s, a, 0x22, part, index))
                return false;
        if (!Spawn(s, a, 6, 16, index) || !Spawn(s, a, 0x23, 0, index) || !ActorSound(s, 0x652))
            return false;
    }
    return true;
}
bool ActorChapelSpikes(SotnActors *s, Actor *a)
{
    if (!s || !a || (s->stage_id != 6 && s->stage_id != 4))
        return SDL_SetError("Spike actors require their original Chapel or NO2 owner");
    if (s->stage_id == 4)
    {
        switch (a->state.id)
        {
        case 0x21:
            return QuartersSpikes(s, a);
        case 0x22:
            return Parts(s, a);
        case 0x23:
            return Dust(s, a);
        case 0x31:
            return Damage(s, a);
        default:
            return SDL_SetError("NO2 spike dispatch received an unsupported actor identity");
        }
    }
    switch (a->state.id)
    {
    case 0x2e:
        return Spikes(s, a);
    case 0x2f:
        return Parts(s, a);
    case 0x30:
        return Dust(s, a);
    case 0x31:
        return Damage(s, a);
    default:
        return SDL_SetError("Chapel spike dispatch received an unsupported actor identity");
    }
}
