// SPDX-License-Identifier: AGPL-3.0-or-later
// Shared reconstructed LIB actor identities from the pinned st/lib/e_init.c.
#include "actors_internal.h"
bool ActorLibrarySound(SotnActors *s, const Actor *a, Uint16 sound)
{
    if (s == NULL || a == NULL || s->stage_id != 2)
        return SDL_SetError("Library positional sound requires initialized stage owners");
    const Sint32 dx = Pixel(a->state.x) - s->camera_x - 128;
    const Sint32 dy = Pixel(a->state.y) - s->camera_y - 128;
    const Sint32 pan = SDL_clamp((SDL_abs(dx) - 32) / 32, 0, 8);
    const Sint32 attenuation = SDL_max(0, SDL_abs(dx) - 96 + SDL_max(0, SDL_abs(dy) - 112));
    const Sint32 volume = 127 - attenuation / 2;
    return volume <= 0 ||
           ActorSoundVolumePan(s, sound, (Uint16)volume, (Sint8)(dx < 0 ? -pan : pan));
}
bool ActorLibraryBlocked(SotnActors *s, const Actor *a, size_t offset, bool facing, bool *out)
{
    if (s == NULL || a == NULL || out == NULL || offset > s->nz0.size)
        return SDL_SetError("Library collision sensors require bounded private data and owners");
    bool blocked = false;
    for (Uint16 i = 0; i < 16; ++i, offset += 4)
    {
        SotnDataView sensor = {0};
        if (!Slice(s->nz0, offset, 4, &sensor))
            return false;
        const Sint32 dx = Signed16(sensor.bytes), dy = Signed16(sensor.bytes + 2);
        if (dx == 255)
        {
            *out = blocked;
            return true;
        }
        SotnCollider collider = {0};
        if (!ActorQuery(s, Wrap16((Sint64)Pixel(a->state.x) + (facing ? dx : -dx)),
                        Wrap16((Sint64)Pixel(a->state.y) + dy), &collider))
            return false;
        blocked = blocked || (collider.effects & SOTN_EFFECT_SOLID) != 0;
    }
    return SDL_SetError("Library collision sensor list has no bounded sentinel");
}
bool ActorLibrary(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2)
        return SDL_SetError("Library actor dispatch requires its original stage service");
    if (a->state.id == 1 || a->state.id == 0x3f || a->state.id == 0x47)
        return ActorLibraryBreakable(s, a);
    if (a->state.id == 0x1b || a->state.id == 0x1c)
        return ActorLibraryFlying(s, a);
    if (a->state.id == 0x1d || a->state.id == 0x1e)
        return ActorLibraryBlood(s, a);
    if (a->state.id == 0x12)
        return ActorCamera(s, a);
    if (a->state.id >= 0x43 && a->state.id <= 0x45)
        return ActorSkeleton(s, a, s->player);
    if (a->state.id == 0x4c)
        return ActorFlea(s, a);
    if (a->state.id == 6)
        return ActorIntenseExplosion(s, a);
    // Library area enemies/scripts must never use another overlay's tables.
    a->state.unsupported = true;
    return true;
}
