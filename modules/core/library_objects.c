// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_room_bg.h and e_room_fg.h at sotn-decomp 7bc54d8157b6.
// Original ObjInit/ObjInit2 records and animations remain private runtime inputs.
#include "actors_internal.h"
bool ActorRoomDecoration(SotnActors *s, Actor *a)
{
    if (!s || !a || (s->stage_id != 2 && s->stage_id != 4 && s->stage_id != 6) ||
        (a->state.id != 8 && a->state.id != 0x11))
        return SDL_SetError("Room decoration requires its own foreground/background actor");
    const bool foreground = a->state.id == 8;
    const bool chapel = s->stage_id == 6, quarters = s->stage_id == 4;
    const Uint16 count = foreground ? 6 : (chapel || quarters) ? 1 : 8;
    const size_t table = foreground ? (chapel     ? 0x1b14
                                       : quarters ? 0x1fb0
                                                  : 0x2094)
                                    : (chapel     ? 0xac8
                                       : quarters ? 0x998
                                                  : 0xa28);
    SotnDataView record = {0};
    if (a->state.parameters >= count ||
        !Slice(s->nz0, table + a->state.parameters * 20U, 20, &record))
        return SDL_SetError("Room decoration index exceeds its private initialization table");
    const Uint8 *p = record.bytes;
    const Uint32 animation = Read32(p + 16);
    const Uint16 draw_flags = Read16(p + 8);
    if (Read16(p + 2) > 511 || (Read16(p + 6) & 0x7fffU) >= 0x300U || draw_flags ||
        animation < 0x80180000 || (animation & 1U))
        return SDL_SetError(
            "Room decoration has an invalid priority, palette, draw flags or animation");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, ActorStageOffset(s, quarters || chapel ? 0xc1c : 0xbec)))
            return false;
        a->state.animation_set = Read16(p);
        a->state.priority = Read16(p + 2);
        // US LIB/DAI/NO2 have no BG_FACING_LEFT_FIX: the low word is the texture offset.
        a->state.texture_offset = Read16(p + 4);
        a->state.palette = Read16(p + 6);
        a->state.blend_mode = Read16(p + 10);
        if (Read32(p + 12))
            a->state.flags = Read32(p + 12);
        if (foreground && a->state.parameters > 4)
        {
            a->state.rotated = true;
            a->state.rotation = 2048;
        }
    }
    Uint8 animated = 0;
    return ActorAnimate(s, a, animation - 0x80180000U, &animated);
}
