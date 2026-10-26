// SPDX-License-Identifier: AGPL-3.0-or-later
// NO2 original identities/shared algorithms, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All runtime tables remain private.
#include "actors_internal.h"
bool ActorOlroxQuarter(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4)
        return SDL_SetError("Olrox's Quarters dispatch requires its original stage owner");
    switch (a->state.id)
    {
    case 1:
    case 0x41:
        return ActorChapelBreakable(s, a);
    case 8:
    case 0x11:
        return ActorRoomDecoration(s, a);
    case 0x12:
        return ActorCamera(s, a);
    case 0x14:
    case 0x15:
        return ActorOuterEffect(s, a);
    case 0x16:
    case 0x17:
    case 0x18:
        return ActorSkelerang(s, a);
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x31:
        return ActorChapelSpikes(s, a);
    case 0x28:
        return ActorOlroxPrisoner(s, a);
    case 0x2e:
    case 0x2f:
        return ActorLibraryBlood(s, a);
    case 0x2d:
        return ActorBloodyZombie(s, a, s->player);
    default:
        a->state.unsupported = true;
        return true;
    }
}
