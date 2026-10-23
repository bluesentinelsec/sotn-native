#ifndef SOTN_ALUCARD_SPRITE_H
#define SOTN_ALUCARD_SPRITE_H

#include <sotn/tilemap.h>

/** @brief One original Alucard frame borrowing packed pixels from ARC_F.BIN. */
typedef struct SotnAlucardSprite
{
    SotnDataView pixels;
    Uint16 width, height;
    Sint32 pivot_x, pivot_y;
} SotnAlucardSprite;

/**
 * @brief Resolve an Alucard sprite through the private DRA and ARC pointer tables.
 * @param dra Borrowed US DRA bytes.
 * @param arc Borrowed US ARC_F.BIN bytes.
 * @param frame Original Alucard frame ID below 265; zero selects an empty sprite.
 * @param out Receives metadata and a borrowed pixel view; unchanged on failure.
 * @return True on success, false with SDL_GetError() for invalid or unsupported frames.
 * @note Returned pixels expire when arc is closed. No pixel bytes are copied.
 */
bool SotnAlucard_DecodeSprite(SotnDataView dra, SotnDataView arc, Uint16 frame,
                              SotnAlucardSprite *out);

/**
 * @brief Draw an untransformed Alucard sprite using the original base palette.
 * @param sprite Borrowed decoded sprite and immutable pixels.
 * @param common Borrowed verified F_GAME.BIN graphics and palettes.
 * @param x Horizontal player origin in viewport pixels.
 * @param y Vertical player origin in viewport pixels.
 * @param facing_left Mirror using the original pivot convention.
 * @param frame Borrowed writable RGB555 framebuffer, not overlapping inputs.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return True on success, false with SDL_GetError() for invalid data or buffers.
 * @note Clips to the original frame. Equipment palettes and effects are separate services.
 */
bool SotnAlucard_DrawSprite(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x,
                            Sint32 y, bool facing_left, Uint16 *frame, size_t count);

#endif
