// SPDX-License-Identifier: AGPL-3.0-or-later
// nz0/bossfight.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "actors_internal.h"
bool ActorBossBox(SotnActors *s, Actor *a, size_t offset, Uint32 count, size_t lookup,
                  Uint32 frames)
{
    SotnDataView index = {0}, box = {0};
    if (a->state.frame >= frames || !Slice(s->nz0, lookup + a->state.frame, 1, &index) ||
        index.bytes[0] >= count || !Slice(s->nz0, offset + (size_t)index.bytes[0] * 4, 4, &box))
        return SDL_SetError("Boss animation references an invalid private hitbox");
    if (box.bytes[2] > 127 || box.bytes[3] > 127)
        return SDL_SetError("Boss hitbox dimensions exceed original signed bytes");
    a->state.hitbox_x = (Sint16)(box.bytes[0] < 128 ? box.bytes[0] : (Sint32)box.bytes[0] - 256);
    a->state.hitbox_y = (Sint16)(box.bytes[1] < 128 ? box.bytes[1] : (Sint32)box.bytes[1] - 256);
    a->state.hitbox_width = box.bytes[2];
    a->state.hitbox_height = box.bytes[3];
    return true;
}
static void Music(SotnActors *s, Uint16 music)
{
    s->combat.music_change = true;
    s->combat.music = music;
}
static bool Manager(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (s->world->time_attack[SOTN_TIME_LAB_BOSSES])
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, 0xbf8))
            return false;
        s->boss_flags = 0;
        SotnDataView positions = {0};
        if (!Slice(s->nz0, 0x1014, 72, &positions))
            return false;
        if ((Uint32)a->slot + 12 >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Boss manager lacks its reserved block slots");
        for (Uint32 i = 0; i < 12; ++i)
        {
            const Uint8 *p = positions.bytes + i * 6;
            Actor *block = NULL;
            if (!ActorSpawnAt(s, a, 0x3f, Read16(p + 4), (Uint32)a->slot + i + 1, &block))
                return false;
            if (block == NULL)
                return SDL_SetError("Boss room block slot could not be created");
            block->state.x = Signed16(p) * 65536;
            block->state.y = Signed16(p + 2) * 65536;
        }
        Actor *slogra = NULL, *gaibon = NULL;
        if (!ActorSpawnAt(s, a, 0x40, 0, 16, &slogra) || !ActorSpawnAt(s, a, 0x43, 0, 24, &gaibon))
            return false;
        if (slogra == NULL || gaibon == NULL)
            return SDL_SetError("Boss pair reserved slots could not be created");
        slogra->state.x = 640 * 65536;
        slogra->state.y = 416 * 65536;
        gaibon->state.x = 672 * 65536;
        gaibon->state.y = 352 * 65536;
    }
    switch (a->state.step)
    {
    case 1:
        if (Pixel(s->player.x) > 24 && Pixel(s->player.x) < 968)
        {
            s->boss_flags |= 1;
            Music(s, 0);
            ++a->state.step;
        }
        break;
    case 2:
        if (Pixel(s->player.x) > 544 && Pixel(s->player.x) < 832)
            s->boss_flags |= 4;
        if (s->boss_flags & 4)
        {
            if (!SotnWorld_VisitEvent(s->world, SOTN_TIME_LAB_BOSSES))
                return false;
            ++a->state.step;
        }
        break;
    case 3:
        if (!s->music_playing)
        {
            Music(s, 0x31d);
            ++a->state.step;
        }
        // Victory can be observed during the music transition, as in the original.
        if ((s->boss_flags & 24) == 24)
        {
            if (!SotnWorld_RecordTime(s->world, SOTN_TIME_LAB_BOSSES, s->game_seconds))
                return false;
            if (s->music_playing)
                Music(s, 0);
            a->state.step = 5;
        }
        break;
    case 4:
        if ((s->boss_flags & 24) == 24)
        {
            if (!SotnWorld_RecordTime(s->world, SOTN_TIME_LAB_BOSSES, s->game_seconds))
                return false;
            if (s->music_playing)
                Music(s, 0);
            a->state.step = 5;
        }
        break;
    case 5: {
        Actor *reward = NULL;
        if (!ActorSpawn(s, a, 0x49, 5, 96, 128, &reward))
            return false;
        if (reward == NULL)
            return true;
        reward->state.x = (s->camera_x + 128) * 65536;
        reward->state.y = 384 * 65536;
        s->boss_flags |= 2;
        s->world->flags[SOTN_LAB_BOSSES] = 1;
        ++a->state.step;
        break;
    }
    case 6:
        if (!s->music_playing)
        {
            Music(s, 0x32e);
            ++a->state.step;
        }
        break;
    case 7:
        break;
    default:
        return SDL_SetError("Laboratory boss manager step is invalid");
    }
    return true;
}
static bool Block(SotnActors *s, Actor *a)
{
    Uint16 contact = 0;
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xd00))
            return false;
        a->state.frame = 8;
    }
    switch (a->state.step)
    {
    case 1:
        if (s->boss_flags & 1)
        {
            a->boss.timer = 16;
            ++a->state.step;
        }
        break;
    case 2:
        a->vx = a->state.parameters ? -65536 : 65536;
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (!ActorObjectCollision(s, a, 8, 8, 5, &contact) ||
            ((s->ticks & 3) == 0 && !ActorSound(s, 0x608)))
            return false;
        if (--a->boss.timer == 0)
            ++a->state.step;
        break;
    case 3:
        if (!ActorObjectCollision(s, a, 8, 8, 5, &contact))
            return false;
        if (s->boss_flags & 2)
            ++a->state.step;
        break;
    case 4:
        a->state.flags |= 0x80000000U;
        a->vx = a->state.parameters ? 65536 : -65536;
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    default:
        return SDL_SetError("Boss room block step is invalid");
    }
    return true;
}
bool ActorBossEvent(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x3e)
        return Manager(s, a);
    if (a->state.id == 0x3f)
        return Block(s, a);
    return SDL_SetError("Unknown Laboratory boss event");
}
