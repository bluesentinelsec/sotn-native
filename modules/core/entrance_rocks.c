// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_jewel_sword_puzzle.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Tile/debris recipes remain in private NO3/NP3 overlays. Inverted variants are separate work.
#include "actors_internal.h"
static bool Patch(SotnActors *s, bool background, Uint32 start, Uint32 rows, Uint32 columns,
                  size_t offset, bool write)
{
    const SotnCollisionMap map = background ? s->background : s->map;
    Uint8 *tiles = background ? s->background_tiles : s->mutable_tiles;
    SotnDataView data = {0};
    if (tiles == NULL || !rows || rows > 5 || !columns || columns > 3 ||
        !Slice(s->nz0, offset, (size_t)rows * columns * 2, &data))
        return SDL_SetError(
            "Entrance rock patch requires an owned layer and bounded private recipe");
    for (Uint32 col = 0; col < columns; ++col)
        for (Uint32 row = 0; row < rows; ++row)
        {
            const Uint32 index = start + col + row * 48;
            const Uint16 tile = Read16(data.bytes + (col * rows + row) * 2);
            if (index >= (Uint64)map.width * map.height || tile >= map.types.size)
                return SDL_SetError("Entrance rock patch exceeds its room layer or definitions");
        }
    if (write)
        for (Uint32 col = 0; col < columns; ++col)
            for (Uint32 row = 0; row < rows; ++row)
            {
                const Uint32 index = start + col + row * 48;
                const Uint16 tile = Read16(data.bytes + (col * rows + row) * 2);
                tiles[(size_t)index * 2] = (Uint8)tile;
                tiles[(size_t)index * 2 + 1] = (Uint8)(tile >> 8);
            }
    return true;
}
static bool Dust(SotnActors *s, const Actor *a, bool right)
{
    Actor *effect = NULL;
    if (!ActorSpawn(s, a, 2, 0x13, 160, 192, &effect))
        return false;
    if (effect != NULL)
    {
        effect->state.priority = 0xa9;
        effect->state.x =
            (Pixel(a->state.x) + (right ? -1 : 1) * a->enemy_state.counter * 16) * 65536;
        effect->state.y = (Pixel(a->state.y) + 16) * 65536;
        effect->state.facing_left = false;
    }
    SotnDataView recipe = {0};
    if (a->enemy_state.counter > 1 ||
        !Slice(s->nz0, EntranceTable(s, 0x1344, 0x120c) + a->enemy_state.counter * 3U, 3, &recipe))
        return SDL_SetError("Entrance rock debris recipe exceeds its private table");
    for (Uint16 i = 0; i < 3; ++i)
    {
        Actor *rock = NULL;
        if (!ActorSpawn(s, a, 0x27, recipe.bytes[i], 160, 192, &rock))
            return false;
        if (rock == NULL)
            continue;
        rock->state.x = Pixel(a->state.x) * 65536;
        rock->state.y = (Pixel(a->state.y) - 16 + (Sint32)i * 16) * 65536;
        rock->state.facing_left = right;
        const Sint32 speed = (Sint32)ActorRandom(s) * 256 + 32768;
        rock->vx = right ? speed : -speed;
        rock->vy = -(Sint32)ActorRandom(s) * 256;
    }
    return true;
}
static bool Barrier(SotnActors *s, Actor *a)
{
    const bool right = a->state.id == 0x25;
    const Uint8 bit = right ? 2 : 1;
    const Uint32 start = right ? 0x1fd : 0x1f1;
    const size_t tiles =
        right ? EntranceTable(s, 0x1288, 0x1150) : EntranceTable(s, 0x124c, 0x1114);
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, EntranceTable(s, 0xadc, 0xa6c)))
            return false;
        a->state.hitbox_state = 2;
        a->state.hitbox_width = 16;
        a->state.hitbox_height = 24;
        const size_t background =
            right ? EntranceTable(s, 0x12ac, 0x1174) : EntranceTable(s, 0x1270, 0x1138);
        const bool broken = (s->world->flags[SOTN_ENTRANCE_ROCKS] & bit) != 0;
        if (!Patch(s, true, start, 3, 2, background + 12, false) ||
            (broken && !Patch(s, false, start, 3, 2, tiles + 24, false)) ||
            !Patch(s, true, start, 3, 2, background + 12, true) ||
            (broken && !Patch(s, false, start, 3, 2, tiles + 24, true)))
            return false;
        if (broken)
        {
            a->state.hitbox_state = 1;
            a->state.step = 2;
        }
        break;
    }
    case 1:
        if (a->enemy_state.counter > 1)
            return SDL_SetError("Entrance rock strike count exceeds its original two-hit sequence");
        if (a->hit_flags)
        {
            if (!Patch(s, false, start, 3, 2, tiles + (a->enemy_state.counter + 1U) * 12, true) ||
                !ActorSound(s, 0x644) || !Dust(s, a, right))
                return false;
            if (++a->enemy_state.counter > 1)
            {
                if (!right)
                {
                    Actor *food = NULL;
                    if (!ActorSpawn(s, a, 0xa, 0x43, 96, 128,
                                    &food)) // Original pot-roast identity.
                        return false;
                    if (food != NULL)
                    {
                        food->state.x = Pixel(a->state.x) * 65536;
                        food->state.y = Pixel(a->state.y) * 65536;
                        food->state.facing_left = false;
                    }
                }
                s->world->flags[SOTN_ENTRANCE_ROCKS] |= bit;
                a->state.hitbox_state = 1;
                ++a->state.step;
            }
        }
        break;
    case 2:
        // Wolf/bat contact bits require the corresponding original player transformations.
        // Those abilities remain explicitly unavailable; this adapter never grants their bits.
        break;
    default:
        return SDL_SetError("Entrance breakable rock entered an invalid original state");
    }
    return true;
}
static bool Door(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, EntranceTable(s, 0xadc, 0xa6c)))
            return false;
        if (s->world->flags[SOTN_ENTRANCE_JEWEL_DOOR])
            a->state.step = 2;
        break;
    case 1:
        if ((s->world->flags[SOTN_ENTRANCE_ROCKS] & 12) == 12)
        {
            if (!ActorSound(s, 0x644))
                return false;
            ++a->state.step;
        }
        break;
    case 2: {
        const size_t foreground = EntranceTable(s, 0x12ac, 0x1174) + 54;
        const size_t background = EntranceTable(s, 0x1308, 0x11d0) + 22;
        if (!Patch(s, false, 0x420, 5, 3, foreground, false) ||
            !Patch(s, true, 0x420, 5, 3, background, false) ||
            !Patch(s, false, 0x420, 5, 3, foreground, true) ||
            !Patch(s, true, 0x420, 5, 3, background, true))
            return false;
        s->world->flags[SOTN_ENTRANCE_JEWEL_DOOR] |= 1;
        ++a->state.step;
        break;
    }
    case 3:
        break;
    default:
        return SDL_SetError("Entrance Jewel Sword door entered an invalid state");
    }
    return true;
}
static bool Fragment(SotnActors *s, Actor *a)
{
    const Uint16 frame = a->state.parameters & 15;
    if (frame > 3)
        return SDL_SetError("Entrance falling-rock variant exceeds its private offset table");
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, EntranceTable(s, 0xb18, 0xaa8)))
            return false;
        a->state.frame = (Uint16)(frame + 31);
        a->state.rotated = true;
        a->state.priority = 0x9f;
        break;
    case 1: {
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += 16384;
        a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation - 32);
        SotnDataView offset = {0};
        if (!Slice(s->nz0, EntranceTable(s, 0x133c, 0x1204) + frame * 2U, 2, &offset))
            return false;
        const Sint32 dy = Signed16(offset.bytes);
        if (SDL_abs(dy) > 32)
            return SDL_SetError(
                "Entrance falling-rock collision offset exceeds its private bounds");
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + dy, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            if (a->vy > 4 * 65536)
            {
                if (!ActorExplosionInPool(s, a, frame == 0 ? 0x13 : 0x11, 160, 192, 0))
                    return false;
                a->remove = true;
            }
            else
            {
                ShiftY(a, c.top);
                a->vy = -a->vy;
                a->vy -= a->vy / 8;
            }
        }
        break;
    }
    default:
        return SDL_SetError("Entrance falling rock entered an invalid state");
    }
    return true;
}
bool ActorEntranceRock(SotnActors *s, Actor *a)
{
    if (s->stage_id != 65 && s->stage_id != 7)
        return SDL_SetError("Entrance rock behavior requires its own NO3/NP3 profile");
    if (a->state.id == 0x24 || a->state.id == 0x25)
        return Barrier(s, a);
    if (a->state.id == 0x26)
        return Door(s, a);
    if (a->state.id == 0x27)
        return Fragment(s, a);
    return SDL_SetError("Unknown Entrance rock-puzzle actor identity");
}
