// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c:func_801093C4 environment and dra/4B758.c bank15 captures,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork remains private.
#include <SDL3/SDL.h>
#include <sotn/wolf_capture.h>
bool SotnWolfCapture_Render(SotnActorGraphics *graphics, const SotnWolfDrawPacket *packets,
                            size_t count, bool clear)
{
    if (!graphics || (count && !packets) || count > SOTN_WOLF_FRAME_PACKETS)
        return SDL_SetError(
            "Wolf capture requires an actual texture owner and bounded packet list");
    Uint8 order[SOTN_WOLF_FRAME_PACKETS] = {0};
    size_t draws = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (!packets[i].capture)
            continue;
        const Uint16 priority = packets[i].quad.priority;
        if (priority < 0x190 || priority >= 0x1b0)
            return SDL_SetError("Wolf capture packet escapes its original environment interval");
        size_t at = draws;
        while (at && packets[order[at - 1U]].quad.priority >= priority)
        {
            order[at] = order[at - 1U];
            --at;
        }
        order[at] = (Uint8)i;
        ++draws;
    }
    SotnActorSurface surface = {0};
    if (!SotnActorGraphics_BeginTarget(
            graphics,
            (SotnActorTarget){.x = 512, .y = 448, .width = 63, .height = 63, .clear = clear},
            &surface))
        return false;
    for (size_t i = 0; i < draws; ++i)
        if (!SotnActorGraphics_PlayerEffect(graphics, &packets[order[i]].quad, 0, 0, surface))
        {
            (void)SotnActorGraphics_EndTarget(graphics, false);
            return false;
        }
    return SotnActorGraphics_EndTarget(graphics, true);
}
