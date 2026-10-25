// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/unk_3B200.c and unk_3B53C.c, pinned sotn-decomp 7bc54d8157b6.
#include "actors_internal.h"
static void Music(SotnActors *s, Uint16 music)
{
    s->combat.music_change = true;
    s->combat.music = music;
}
static bool Victory(SotnActors *s, Actor *a)
{
    if (s->library_enemy_flags & 2U)
    {
        if (!SotnWorld_RecordTime(s->world, SOTN_TIME_LESSER_DEMON, s->game_seconds))
            return false;
        if (s->music_playing)
            Music(s, 0);
        a->state.step = 5;
    }
    return true;
}
static bool Manager(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0x830))
            return false;
        if (s->world->time_attack[SOTN_TIME_LESSER_DEMON])
        {
            a->remove = true;
            return true;
        }
        Actor *body = NULL, *camera = NULL;
        if (!ActorLibrarySpawnAt(s, a, 0x1f, 0, 16, &body) ||
            !ActorLibrarySpawnAt(s, a, 0x12, 6, 15, &camera))
            return false;
        body->state.x = 512 * 65536;
        body->state.y = 672 * 65536;
        camera->state.x = 512 * 65536;
        camera->state.y = 624 * 65536;
    }
    switch (a->state.step)
    {
    case 1:
        if (Pixel(s->player.x) > 368 && Pixel(s->player.x) < 656 && Pixel(s->player.y) > 544 &&
            Pixel(s->player.y) < 704)
        {
            s->library_arena_active = true;
            s->library_enemy_flags |= 1U;
            if (!SotnWorld_VisitEvent(s->world, SOTN_TIME_LESSER_DEMON))
                return false;
            ++a->state.step;
        }
        break;
    case 2:
        if (s->music_playing)
            Music(s, 0);
        ++a->state.step;
        break;
    case 3:
        if (!s->music_playing)
        {
            Music(s, 0x31d);
            ++a->state.step;
        }
        return Victory(s, a);
    case 4:
        return Victory(s, a);
    case 5:
        if (s->library_enemy_flags & 4U)
        {
            Actor *reward = NULL;
            if (!ActorSpawn(s, a, 0x42, 7, 96, 128, &reward))
                return false;
            if (reward == NULL)
                return true;
            reward->state.x = (s->camera_x + 128) * 65536;
            reward->state.y = (s->camera_y + 128) * 65536;
            reward->state.facing_left = false;
            s->library_arena_active = false;
            s->library_camera_unlocked = true;
            s->library_enemy_flags &= ~1U;
            ++a->state.step;
        }
        break;
    case 6:
        if (!s->music_playing)
        {
            Music(s, 0x309);
            ++a->state.step;
        }
        break;
    case 7:
        break;
    default:
        return SDL_SetError("Library arena manager step is invalid");
    }
    return true;
}
static bool Tile(SotnActors *s, const Actor *a, bool closed)
{
    const Uint16 low = a->state.parameters & 255U, side = a->state.parameters >> 8;
    const Uint32 index = (side ? 0xc0dU : 0xbf2U) + (3U - low) * 80U;
    SotnDataView tile = {0};
    if (s->map.width != 80 ||
        !Slice(s->nz0, 0x1aa8 + ((closed ? 7U : 3U) - low) * 4U + side * 2U, 2, &tile))
        return SDL_SetError("Library arena seal requires its original room and bounded tile table");
    return ActorTile(s, index, Read16(tile.bytes));
}
static bool Seal(SotnActors *s, Actor *a)
{
    const Uint16 low = a->state.parameters & 255U, side = a->state.parameters >> 8;
    if (low > 3 || side > 1)
        return SDL_SetError("Library arena seal parameter is invalid");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0x95c))
            return false;
        a->state.frame = (Uint16)(low + 15);
        a->state.priority = 0x69;
        a->state.facing_left = side != 0;
        a->arena.triggered = false;
        if (a->state.parameters == 0)
        {
            if ((Uint32)a->slot + 7U >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Library sealing books lack their reserved child slots");
            for (Uint16 i = 1; i < 8; ++i)
            {
                Actor *child = NULL;
                const Uint16 parameters = (Uint16)(i < 4 ? i : 0x100U + i - 4U);
                if (!ActorLibrarySpawnAt(s, a, 0x40, parameters, (Uint32)a->slot + i, &child))
                    return false;
            }
        }
        a->state.x = (side ? 736 : 288) * 65536;
        a->state.y = 600 * 65536;
        return true;
    }
    switch (a->state.step)
    {
    case 1:
        if ((s->library_arena_active && low == 0) || a->arena.triggered)
            a->state.step = 2;
        break;
    case 2:
        a->vy = 4 * 65536;
        ++a->state.step;
        SDL_FALLTHROUGH;
    case 3: {
        const Sint32 target = (3 - low) * 16 + 616;
        const Sint64 y = (Sint64)a->state.y + a->vy, vy = (Sint64)a->vy + 8192;
        if (y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 || vy > SDL_MAX_SINT32)
            return SDL_SetError("Library falling seal exceeds fixed-point bounds");
        a->state.y = (Sint32)y;
        a->vy = (Sint32)vy;
        if (target < Pixel(a->state.y))
        {
            const Uint32 root_slot = a->state.parameters == 0 ? a->slot : a->enemy_state.owner_slot;
            const Uint32 generation =
                a->state.parameters == 0 ? a->state.generation : a->enemy_state.owner_generation;
            if ((Uint32)a->slot + 1U < SOTN_STAGE_ACTOR_SLOTS)
            {
                Actor *next = &s->slots[a->slot + 1U];
                // The original writes an unused field after the final segment. Never alter an
                // unrelated owner.
                if (next->state.generation != 0 && next->state.id == 0x40 &&
                    next->enemy_state.owner_slot == root_slot &&
                    next->enemy_state.owner_generation == generation)
                    next->arena.triggered = true;
            }
            a->state.y = (Sint32)((Sint64)target * 65536 + ((Uint32)a->state.y & 65535U));
            a->vy = 0;
            if (!Tile(s, a, true) || (side != 0 && !ActorSound(s, 0x650)))
                return false;
            ++a->state.step;
        }
        break;
    }
    case 4:
        if (!s->library_arena_active)
            ++a->state.step;
        break;
    case 5:
        if (!Tile(s, a, false) || !ActorExplosionInPool(s, a, 3, 160, 192, 0))
            return false;
        a->state.frame = 0;
        ++a->state.step;
        break;
    case 6:
        break;
    default:
        return SDL_SetError("Library arena seal step is invalid");
    }
    return true;
}
bool ActorLibraryEvent(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2 || s->world == NULL)
        return SDL_SetError("Library arena event requires initialized stage and progress owners");
    if (a->state.id == 0x41)
        return Manager(s, a);
    if (a->state.id == 0x40)
        return Seal(s, a);
    return SDL_SetError("Unknown Library arena event actor");
}
