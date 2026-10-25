// SPDX-License-Identifier: AGPL-3.0-or-later
// DAI e_tower_exit.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Private initialization/layout records remain runtime inputs.
#include "actors_internal.h"

bool ActorChapelExit(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 6 || a->state.id != 0x16 ||
        a->state.parameters > 14 || a->state.step > 5)
        return SDL_SetError("Chapel tower exit requires its original owner, variant and state");
    if (a->state.step == 0)
        return ActorInitialize(s, a, 0x98c);
    if (a->state.step == 5 || SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) >= 48)
        return true;
    if (s->player.status & 0x40000U)
    {
        s->player.velocity_x = 0;
        s->motion.stop_horizontal = true;
        return true;
    }
    Sint32 threshold = 0;
    bool left = false, shift_origin = false, shrink_right = false;
    switch (a->state.parameters)
    {
    case 0:
    case 11:
        threshold = 640;
        shrink_right = true;
        break;
    case 1:
    case 10:
    case 12:
        threshold = 384;
        left = shift_origin = true;
        break;
    case 2:
    case 4:
    case 13:
    case 14:
        threshold = 768;
        shrink_right = true;
        break;
    case 3:
        threshold = 256;
        left = shift_origin = true;
        break;
    case 5:
    case 8:
        threshold = 128;
        left = true;
        break;
    case 6:
    case 7:
        threshold = 1152;
        break;
    case 9:
        threshold = 640;
        break;
    default:
        return SDL_SetError("Chapel tower exit has an invalid threshold variant");
    }
    const SotnRoomWindow old = s->window;
    if (old.origin_cell < 0 || old.origin_cell > 64 || old.last_cell < -1 || old.last_cell > 63 ||
        old.start_x < 0 || old.end_x < 256 || old.start_x >= old.end_x ||
        old.end_x > (Sint32)s->map.width * 16 || threshold > (Sint32)s->map.width * 16)
        return SDL_SetError("Chapel tower exit has invalid room limits");
    // dra/5087C.c saves the screen X before player motion, then updates stage actors
    // before adding that motion to g_PlayerX and selecting the neighboring room.
    // The reference's left++ / g_PlayerX-=256 pair preserves world position.
    const Sint32 previous = s->frame_start_x - old.origin_cell * 256,
                 current = Pixel(s->player.x) - old.origin_cell * 256;
    if (left ? previous >= threshold || current >= threshold
             : previous <= threshold || current <= threshold)
        return true;
    SotnRoomWindow next = old;
    if (left)
        next.start_x = threshold;
    else
        next.end_x = threshold;
    if (shift_origin)
        ++next.origin_cell;
    if (shrink_right)
        --next.last_cell;
    if (next.start_x >= next.end_x || next.origin_cell > 64 || next.last_cell < -1)
        return SDL_SetError("Chapel tower exit would exceed its bounded room window");
    s->window = next;
    ++a->state.step;
    return true;
}
