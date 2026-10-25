// SPDX-License-Identifier: AGPL-3.0-or-later
// Shared EntityIntenseExplosion from st/e_misc.h at the pinned reconstructed reference.
#include "actors_internal.h"
bool ActorIntenseExplosion(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, ActorTable(s, 0xbe0, 0xac4, 0xa54)))
            return false;
        a->state.palette = (a->state.parameters & 0xf0) ? 0x8195 : 0x8170;
        a->state.animation_set = 5;
        a->state.frame = 1;
        a->state.blend_mode = (a->state.parameters & 0xf0) ? 0x10 : 0x30;
        if (a->state.parameters & 0xff00)
            a->state.priority = (Uint16)(a->state.parameters >> 8);
        a->state.priority = (Uint16)(a->state.priority + 8);
        return true;
    }
    a->state.y -= 16384;
    if ((++a->timer % 2) == 0)
        ++a->state.frame;
    if (a->timer > 36)
        a->remove = true;
    return true;
}
