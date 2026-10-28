// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c:func_801093C4 environment and dra/4B758.c bank15 captures,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork remains private.
#include <SDL3/SDL.h>
#include <sotn/wolf_capture.h>
static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
bool SotnWolfCapture_Snapshot(SotnDataView dra, SotnWolfCaptureBounds bounds, Sint32 x, Sint32 y,
                              bool facing_left, SotnMistWedge out[8])
{
    const size_t profile = 0xd670 + 29U * 20U + 14U;
    if (!out || !dra.bytes || dra.size < profile + 2 || dra.bytes[profile] != 0x0a ||
        dra.bytes[profile + 1] != 0x70 || x < SDL_MIN_SINT16 || x > SDL_MAX_SINT16 ||
        y < SDL_MIN_SINT16 || y > SDL_MAX_SINT16 || bounds.left < SDL_MIN_SINT16 ||
        bounds.left > SDL_MAX_SINT16 || bounds.right < SDL_MIN_SINT16 ||
        bounds.right > SDL_MAX_SINT16 || bounds.top < SDL_MIN_SINT16 ||
        bounds.top > SDL_MAX_SINT16 || bounds.bottom < SDL_MIN_SINT16 ||
        bounds.bottom > SDL_MAX_SINT16)
        return SDL_SetError(
            "Wolf snapshot requires actual word bounds and the original Mist profile");
    const Sint32 width = Word(bounds.right - bounds.left),
                 height = Word(bounds.bottom - bounds.top),
                 edge_x[8] = {0, width / 2, width, width, width, width / 2, 0, 0},
                 edge_y[8] = {0, 0, 0, height / 2, height, height, height, height / 2},
                 origin_x = Word((Sint64)x + bounds.left), origin_y = Word((Sint64)y + bounds.top),
                 texture_x = (facing_left ? 20 : 44) + bounds.left, texture_y = bounds.top - 40;
    SotnMistWedge wedges[8] = {0};
    for (Uint16 i = 0; i < 8; ++i)
    {
        for (Uint16 vertex = 0; vertex < 2; ++vertex)
        {
            const Uint16 at = (Uint16)((i + vertex) % 8U);
            wedges[i].x[vertex] = Word(origin_x + edge_x[at]);
            wedges[i].y[vertex] = Word(origin_y + edge_y[at]);
            wedges[i].u[vertex] = (Uint8)(Uint32)(texture_x + edge_x[at]);
            wedges[i].v[vertex] = (Uint8)(Uint32)(texture_y + edge_y[at]);
        }
        wedges[i].x[2] = Word(origin_x + width / 2);
        wedges[i].y[2] = Word(origin_y + height / 2);
        wedges[i].u[2] = (Uint8)(Uint32)(texture_x + width / 2);
        wedges[i].v[2] = (Uint8)(Uint32)(texture_y + height / 2);
    }
    SDL_memcpy(out, wedges, sizeof(wedges));
    return true;
}
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
