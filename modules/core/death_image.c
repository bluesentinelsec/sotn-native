// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/71830.c:PlayerStepKill phase2, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private artwork is embedded.
#include <SDL3/SDL_error.h>
#include <sotn/death_image.h>

bool SotnDeathImage_Begin(SotnDeathImage *image, SotnDataView pixels)
{
    if (!image || !pixels.bytes || pixels.size != SOTN_DEATH_IMAGE_BYTES)
        return SDL_SetError("Death image requires the actual complete packed display capture");
    SotnDeathImage next = {.remaining = 64, .captured = true};
    SDL_memcpy(next.pixels, pixels.bytes, sizeof(next.pixels));
    *image = next;
    return true;
}
bool SotnDeathImage_Tick(SotnDeathImage *image)
{
    if (!image || !image->captured || !image->remaining || image->remaining > 64)
        return SDL_SetError("Death image requires its live captured common-death cursor");
    SotnDeathImage next = *image;
    for (Uint16 pass = 0; pass < 4; ++pass)
    {
        const Uint16 base = (Uint16)(((next.cursor >> 1) & 7U) + (next.cursor >> 4) * 64U);
        const Uint8 mask = (Uint8)((next.cursor & 1U) ? 0xf0U : 0x0fU);
        for (Uint16 band = 0; band < 40; ++band)
        {
            const size_t at = base + (band & 7U) * 8U + (band >> 3) * 1024U;
            next.pixels[at] &= mask;
        }
        next.cursor = (Uint8)(next.cursor + 35U);
    }
    --next.remaining;
    *image = next;
    return true;
}
