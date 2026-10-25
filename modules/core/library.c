// SPDX-License-Identifier: AGPL-3.0-or-later
// Shared reconstructed LIB actor identities from the pinned st/lib/e_init.c.
#include "actors_internal.h"
bool ActorLibrary(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2)
        return SDL_SetError("Library actor dispatch requires its original stage service");
    if (a->state.id == 1 || a->state.id == 0x3f || a->state.id == 0x47)
        return ActorLibraryBreakable(s, a);
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
