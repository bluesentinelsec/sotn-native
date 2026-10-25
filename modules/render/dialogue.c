// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected US font/portrait addressing from st/no3/cutscene.c and item icon addressing
// from dra/menu.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Direct framebuffer window presentation is a native adaptation; artwork remains private.
#include <sotn/character.h>
#include <sotn/dialogue_renderer.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}

bool SotnDialogueRenderer_Background(Uint8 fade, Uint16 *frame, size_t count)
{
    if (frame == NULL || count < SOTN_FRAME_PIXELS || fade > 240)
        return SDL_SetError("Dialogue transition requires a bounded shade and full frame");
    if (fade)
        for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        {
            Uint16 color = 0;
            for (Uint32 shift = 0; shift < 15; shift += 5)
                color |= (Uint16)((((frame[i] >> shift) & 31) * (255U - fade) / 255U) << shift);
            frame[i] = color;
        }
    return true;
}

bool SotnDialogueRenderer_Draw(const SotnActorGraphics *g, SotnDialogueState d, Uint16 *frame,
                               size_t count)
{
    if (g == NULL || frame == NULL || count < SOTN_FRAME_PIXELS || d.x > 256 || d.y > 240 ||
        d.palette >= 0x300 || d.row >= 5 || d.column > 24 || d.speaker > 1 || d.portrait_slot > 1 ||
        d.transition > d.transition_length ||
        (d.portrait_page != 0 && d.portrait_page != 0x91 && d.portrait_page != 0x94))
        return SDL_SetError("Dialogue presentation state is outside its bounded US layout");
    for (Uint32 row = 0; row < 5; ++row)
    {
        const size_t length = SDL_strnlen(d.lines[row], sizeof(d.lines[row]));
        if (length > 24)
            return SDL_SetError("Dialogue line has no bounded terminator");
        for (size_t i = 0; i < length; ++i)
            if ((Uint8)d.lines[row][i] < 32 || (Uint8)d.lines[row][i] > 126)
                return SDL_SetError("Dialogue line contains an unsupported US glyph");
    }
    if (!d.visible)
        return true;
    for (Sint32 y = d.y; y < SDL_min((Sint32)d.y + 72, 240); ++y)
        for (Sint32 x = 4; x < 248; ++x)
        {
            const bool border = x == 4 || x == 247 || y == d.y || y == (Sint32)d.y + 71;
            frame[(size_t)y * 256 + (size_t)x] = border ? 0x4210 : 0x2400;
        }
    if (d.portrait_visible && d.portraits[d.portrait_slot])
    {
        Sint32 half_width = 24, half_height = 36;
        if (d.transition_length == 6 && d.transition)
        {
            half_width = (6 - d.transition) * 4;
            half_height = (6 - d.transition) * 6;
        }
        const Sint32 center_x = (Sint32)d.x - 30, center_y = (Sint32)d.y + 36;
        const Uint16 u = (Uint16)(d.portrait_slot * 64U);
        const SotnTextureQuad portrait = {
            .x = {center_x - half_width, center_x + half_width, center_x - half_width,
                  center_x + half_width},
            .y = {center_y - half_height, center_y - half_height, center_y + half_height,
                  center_y + half_height},
            .u = {(Uint16)(u + (d.mirrored ? 47 : 0)), (Uint16)(u + (d.mirrored ? 0 : 47)),
                  (Uint16)(u + (d.mirrored ? 47 : 0)), (Uint16)(u + (d.mirrored ? 0 : 47))},
            .v = {0, 0, 72, 72},
            .page = (Uint16)(d.portrait_page ? d.portrait_page : 0x94),
            .palette = d.palette};
        if (half_width && !SotnActorGraphics_Quad(g, portrait, frame, count))
            return false;
    }
    for (Uint32 row = 0; row < 5; ++row)
        for (Uint32 column = 0; column < 24 && d.lines[row][column]; ++column)
        {
            const Uint8 c = (Uint8)d.lines[row][column];
            if (c == ' ')
                continue;
            const Sint32 x = (Sint32)d.x + 4 + (Sint32)column * 8;
            const Sint32 y = (Sint32)d.y + 20 + (Sint32)row * 12;
            const Uint16 u = (Uint16)((c & 15) * 8), v = (Uint16)((240 + ((c & 240) >> 1)) & 255);
            const SotnTextureQuad glyph = {.x = {x, x + 8, x, x + 8},
                                           .y = {y, y, y + 8, y + 8},
                                           .u = {u, (Uint16)(u + 8), u, (Uint16)(u + 8)},
                                           .v = {v, v, (Uint16)(v + 8), (Uint16)(v + 8)},
                                           .page = 0x1e,
                                           .palette = 0x1a1};
            if (!SotnActorGraphics_Quad(g, glyph, frame, count))
                return false;
        }
    return true;
}

bool SotnDialogueRenderer_Item(SotnDataView dra, SotnDataView stage, Uint16 parameter, Sint32 x,
                               Sint32 y, Uint16 width, Uint16 height, bool blink, Uint16 *frame,
                               size_t count)
{
    if (frame == NULL || count < SOTN_FRAME_PIXELS || parameter >= 6 || width > 16 || height > 16 ||
        stage.bytes == NULL || stage.size < 0x1ae0 || dra.bytes == NULL)
        return SDL_SetError("Death item presentation requires bounded dimensions and private data");
    if (width == 0 || height == 0)
        return true;
    const Uint16 id = Read16(stage.bytes + 0x1ad4 + parameter * 2U);
    SotnItem item = {0};
    if (!SotnItem_Read(dra, id >= SOTN_HAND_ITEMS,
                       (Uint16)(id >= SOTN_HAND_ITEMS ? id - SOTN_HAND_ITEMS : id), &item))
        return false;
    const size_t pixels = 0x25324 + (size_t)item.icon * 128;
    const size_t palette = 0x388d4 + (size_t)item.palette * 32;
    if (item.icon >= 320 || item.palette >= 320 || pixels > dra.size || 128 > dra.size - pixels ||
        palette > dra.size || 32 > dra.size - palette)
        return SDL_SetError("Death item artwork exceeds its private icon table");
    for (Sint32 dy = 0; dy < height; ++dy)
        for (Sint32 dx = 0; dx < width; ++dx)
        {
            const Sint32 px = x - width / 2 + dx, py = y - height / 2 + dy;
            if (px < 0 || px >= 256 || py < 0 || py >= 240)
                continue;
            const Uint32 source_x = 1U + (Uint32)dx * 14U / width;
            const Uint32 source_y = 1U + (Uint32)dy * 14U / height;
            const Uint8 index =
                (dra.bytes[pixels + source_y * 8 + source_x / 2] >> ((source_x & 1) * 4)) & 15;
            const Uint16 color = Read16(dra.bytes + palette + index * 2U);
            if (color)
            {
                Uint16 tinted = color & 0x7fff;
                if (blink)
                    tinted =
                        (Uint16)(SDL_min(31, (color & 31) * 3 / 2) |
                                 (((color >> 5) & 31) / 2 << 5) | (((color >> 10) & 31) / 2 << 10));
                frame[(size_t)py * 256 + (size_t)px] = tinted;
            }
        }
    return true;
}
