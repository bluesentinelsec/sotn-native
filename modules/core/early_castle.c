// SPDX-License-Identifier: AGPL-3.0-or-later
// Shared stage actor dispatch retaining the original per-overlay entity identities.
#include "actors_internal.h"
bool ActorEarlyCastle(SotnActors *s, Actor *a)
{
    if (s->stage_id == 1 && a->state.id >= 0x59 && a->state.id <= 0x5b)
        return ActorMedusa(s, a);
    if (s->stage_id == 1 && a->state.id >= 0x4a && a->state.id <= 0x4e)
        return ActorArmorLord(s, a);
    if (s->stage_id == 1 && a->state.id >= 0x53 && a->state.id <= 0x56)
        return ActorSkeletonApe(s, a);
    if (s->stage_id == 0 && ((a->state.id >= 0x19 && a->state.id <= 0x20) || a->state.id == 0x2b ||
                             a->state.id == 0x42))
        return ActorClock(s, a);
    if (s->stage_id == 1 && (a->state.id == 0x29 || a->state.id == 0x2a || a->state.id == 0x2b ||
                             a->state.id == 0x2c || a->state.id == 0x32 || a->state.id == 0x33))
        return ActorOuterElevator(s, a);
    if (s->stage_id == 1 && a->state.id >= 0x20 && a->state.id <= 0x22)
        return ActorBlueKnight(s, a);
    if (s->stage_id == 1 && a->state.id == 0x23)
        return ActorOuterEffect(s, a);
    if (s->stage_id == 1 && (a->state.id == 6 || a->state.id == 0x14 || a->state.id == 0x15))
        return ActorOuterEffect(s, a);
    if (s->stage_id == 1 && (a->state.id == 0x48 || a->state.id == 0x49))
        return ActorSwordLord(s, a);
    if (s->stage_id == 1 && (a->state.id == 0x4f || a->state.id == 0x50 || a->state.id == 0x58))
        return ActorSpearGuard(s, a);
    if (s->stage_id == 1 && (a->state.id == 0x44 || a->state.id == 0x45))
        return ActorArcher(s, a);
    if (s->stage_id == 1 && (a->state.id == 0x46 || a->state.id == 0x47))
        return ActorMusket(s, a);
    if (a->state.id == 0x12)
        return ActorCamera(s, a);
    if (a->state.id == ActorStageId(s, 0x2e) || a->state.id == ActorStageId(s, 0x2f) ||
        a->state.id == ActorStageId(s, 0x30))
        return ActorSkeleton(s, a, s->player);
    if (s->stage_id == 0 && (a->state.id == 0x47 || a->state.id == 0x48))
        return ActorKnight(s, a, s->player);
    if (s->stage_id == 0 && (a->state.id == 0x13 || (a->state.id >= 0x21 && a->state.id <= 0x26)))
        return ActorDiplo(s, a);
    if (s->stage_id == 0 && a->state.id >= 0x2e && a->state.id <= 0x30)
        return ActorSkelerang(s, a);
    if (s->stage_id == 0 && a->state.id == 0x3d)
        return ActorRose(s, a);
    if (s->stage_id == 0 &&
        (a->state.id == 0x3e || a->state.id == 0x3f || a->state.id == 0x40 || a->state.id == 0x41))
        return ActorRoseSeed(s, a);
    if (s->stage_id == 0 && (a->state.id == 0x4a || a->state.id == 0x4b))
        return ActorOuija(s, a);
    if (s->stage_id == 0 && a->state.id == 0x4c)
        return ActorFlea(s, a);
    if (s->stage_id == 0 && a->state.id >= 0x3a && a->state.id <= 0x3c)
        return ActorSlinger(s, a);
    a->state.unsupported = true;
    return true;
}
