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
    case 0x24:
    case 0x25:
    case 0x26:
        return ActorOlroxSecret(s, a);
    case 0x28:
        return ActorOlroxPrisoner(s, a);
    case 0x33:
        return ActorNo2Door(s, a);
    case 0x34:
    case 0x35:
    case 0x36:
        return ActorValhalla(s, a);
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3c:
    case 0x3d:
        return ActorGiant(s, a);
    case 0x3e:
    case 0x3f:
    case 0x40:
        return ActorNo2Spectral(s, a);
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
