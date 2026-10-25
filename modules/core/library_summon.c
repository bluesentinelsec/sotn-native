// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c:func_us_801BB8DC at the pinned reconstruction.
#include "actors_internal.h"
bool ActorLibrarySummon(SotnActors *s, Actor *a, size_t sensors)
{
    if (!s || !a || s->stage_id != 2)
        return SDL_SetError("Library summon requires initialized original stage owners");
    switch (a->substep)
    {
    case 0:
        if (a->state.priority < 16)
            return SDL_SetError("Library summon priority underflow");
        a->state.frame = 0;
        a->state.hitbox_state = 0;
        a->state.priority = (Uint16)(a->state.priority - 16);
        a->summon.palette = a->state.palette;
        a->state.opacity = 2;
        a->state.opacity_set = true;
        a->state.flags = (a->state.flags | 0x2000U) & ~0xc00U;
        a->substep = sensors ? 1 : 2;
        break;
    case 1: {
        Uint16 collision = 0;
        if (!ActorGround(s, a, sensors, &collision))
            return false;
        if (collision & 1U)
        {
            a->state.frame = 1;
            ++a->substep;
        }
        break;
    }
    case 2:
        a->state.opacity = (Uint16)(a->state.opacity + 4);
        if (a->state.opacity > 160)
        {
            a->state.opacity_set = false;
            a->state.opacity = 0;
            a->state.scale_set = a->state.rotated = false;
            a->state.scale_x = a->state.scale_y = 0;
            a->state.blend_mode = 0;
            a->summon.timer = 32;
            ++a->substep;
        }
        break;
    case 3:
        if (!a->summon.timer)
            return SDL_SetError("Library summon blink timer underflow");
        a->state.palette = (a->summon.timer & 1U) ? a->summon.palette : 0x819f;
        if (--a->summon.timer == 0)
        {
            a->state.palette = a->summon.palette;
            a->state.hitbox_state = 3;
            SetStep(a, 1);
        }
        break;
    default:
        return SDL_SetError("Library summon entered an invalid substep");
    }
    return true;
}
