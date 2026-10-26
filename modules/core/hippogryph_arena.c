// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO5 EntityRbo3Door and func_us_801A4494, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime initialization stays private.
#include "actors_internal.h"

static void Whole(Sint32 *position, Sint32 pixels)
{
    *position = (Sint32)((Sint64)pixels * 65536 + ((Uint32)*position & 65535U));
}
static bool MoveY(Actor *a, Sint32 speed)
{
    const Sint64 y = (Sint64)a->state.y + speed;
    if (y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Hippogryph arena door exceeds fixed-point motion bounds");
    a->state.y = (Sint32)y;
    return true;
}
static bool Tiles(SotnActors *s, const Actor *a, bool open)
{
    const bool right = a->state.parameters != 0;
    const Uint32 first = right ? 222U : 193U;
    const Uint16 tile = open ? 0 : 0x4b3;
    if (s->map.width != 32 || first + 96U >= (Uint64)s->map.width * s->map.height ||
        !s->mutable_tiles || tile >= s->map.types.size)
        return SDL_SetError(
            "Hippogryph arena seal requires its original room and tile definitions");
    for (Uint32 i = 0; i < 4; ++i)
        if (!ActorTile(s, first + i * 32U, tile))
            return false;
    return true;
}
static bool Door(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 2)
        return SDL_SetError("Hippogryph arena door has an invalid original variant");
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x4cc))
            return false;
        a->state.priority = 92;
        a->state.frame = (Uint16)(12U + a->state.parameters);
        if (a->state.parameters == 2)
            return true;
        Whole(&a->state.x, a->state.parameters ? 464 : 16);
        Whole(&a->state.y, 32);
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x21, 2, (Uint32)a->slot + 1U, &child))
            return false;
        child->state.x = (Pixel(a->state.x) + (a->state.parameters ? 16 : 0)) * 65536;
        child->state.y = (160 + s->camera_y) * 65536;
        child->state.facing_left = false;
        return true;
    }
    case 1:
        if (!s->hippogryph_encounter)
            return true;
        if (a->state.parameters == 2)
            a->state.step = 8;
        else
        {
            if (!Tiles(s, a, false))
                return false;
            a->state.step = 2;
        }
        return true;
    case 2:
        if (!MoveY(a, 73728))
            return false;
        if ((Sint64)a->state.y > ((Sint64)70 + s->camera_y) * 65536)
        {
            a->state.y = (70 + s->camera_y) * 65536;
            ++a->state.step;
        }
        return true;
    case 3:
        if (!s->hippogryph_encounter)
        {
            if (!Tiles(s, a, true))
                return false;
            ++a->state.step;
        }
        return true;
    case 4:
        if (!MoveY(a, -49152))
            return false;
        if (Pixel(a->state.y) < 32)
        {
            Whole(&a->state.y, 32);
            ++a->state.step;
        }
        return true;
    case 8:
        if (!MoveY(a, -73728))
            return false;
        if ((Sint64)a->state.y < ((Sint64)129 + s->camera_y) * 65536)
        {
            a->state.y = (129 + s->camera_y) * 65536;
            ++a->state.step;
        }
        return true;
    case 9:
        if (!s->hippogryph_encounter)
            ++a->state.step;
        return true;
    case 10:
        if (!MoveY(a, 49152))
            return false;
        if (Pixel(a->state.y) >= 161)
        {
            Whole(&a->state.y, 160);
            ++a->state.step;
        }
        return true;
    default:
        return true; // The original remaining states are inert.
    }
}
static bool Music(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x484))
            return false;
        SDL_FALLTHROUGH;
    case 1:
        if (s->hippogryph_encounter)
        {
            s->hippogryph_stop_music = true;
            s->hippogryph_music = 0x334;
            ++a->state.step;
        }
        return true;
    case 2:
        if (!s->music_playing && !s->death_event.cd_busy)
        {
            s->hippogryph_stop_music = false;
            s->combat.music_change = true;
            s->combat.music = s->hippogryph_music;
            ++a->state.step;
        }
        return true;
    case 3:
        if (!s->hippogryph_scene)
        {
            s->combat.cd_command = 0x91;
            ++a->state.step;
        }
        return true;
    default:
        return true;
    }
}
bool ActorHippogryphArena(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 25 || !a->state.generation || s->camera_x < 0 ||
        s->camera_x > 16384 || s->camera_y < 0 || s->camera_y > 16384)
        return SDL_SetError("Hippogryph arena requires its original stage and an active actor");
    if (a->state.id == 0x21)
        return Door(s, a);
    if (a->state.id == 0x29)
        return Music(s, a);
    return SDL_SetError("Hippogryph arena received an unrelated original actor");
}
