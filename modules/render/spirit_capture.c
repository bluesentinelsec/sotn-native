// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/84B88.c:EntitySummonSpirit MoveImage boundary, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No original assets are embedded.
#include <sotn/spirit_capture.h>
bool SotnSpiritCapture_Frame(SotnActorGraphics *graphics, const Uint16 *frame, size_t count,
                             Sint32 x, Sint32 y)
{
    if (!graphics || !frame || count < SOTN_FRAME_PIXELS || x < 0 || y < 0 ||
        x > SOTN_FRAME_WIDTH - 63 || y > SOTN_FRAME_HEIGHT - 63)
        return SDL_SetError("Spirit capture requires its preceding frame and an on-frame63x63 "
                            "source (%d,%d); off-frame PS1 VRAM is not integrated",
                            x, y);
    Uint16 pixels[63 * 63] = {0};
    for (size_t row = 0; row < 63; ++row)
        SDL_memcpy(pixels + row * 63, frame + ((size_t)y + row) * SOTN_FRAME_WIDTH + (size_t)x,
                   63 * sizeof(*pixels));
    SotnActorSurface target = {0};
    if (!SotnActorGraphics_BeginTarget(
            graphics, (SotnActorTarget){.x = 512, .y = 448, .width = 63, .height = 63}, &target))
        return false;
    SDL_memcpy(target.pixels, pixels, sizeof(pixels));
    return SotnActorGraphics_EndTarget(graphics, true);
}
