// SPDX-License-Identifier: AGPL-3.0-or-later
// Shared stage actor dispatch retaining the original per-overlay entity identities.
#include "actors_internal.h"
bool ActorEarlyCastle(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x12)
        return ActorCamera(s, a);
    if (a->state.id == ActorStageId(s, 0x2e) || a->state.id == ActorStageId(s, 0x2f) ||
        a->state.id == ActorStageId(s, 0x30))
        return ActorSkeleton(s, a, s->player);
    if (s->stage_id == 0 && (a->state.id == 0x47 || a->state.id == 0x48))
        return ActorKnight(s, a, s->player);
    a->state.unsupported = true;
    return true;
}
