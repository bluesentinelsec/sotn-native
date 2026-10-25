#ifndef SOTN_ALUCARD_SPRITE_H
#define SOTN_ALUCARD_SPRITE_H

#include <sotn/player.h>
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
/** @brief Draw the selected physical-death tint, rotation, and original dissolve mask.
 * @param sprite Borrowed decoded sprite. @param common Borrowed verified base palettes.
 * @param x Screen anchor X. @param y Screen anchor Y. @param facing_left Original mirror flag.
 * @param death Copied physical-death cursor. @param frame Writable full RGB555 framebuffer.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return Success, or false with SDL_GetError() and unchanged output on invalid inputs.
 * @note CPU rotation replaces GTE services; complete transform/raster parity remains unproven. */
bool SotnAlucard_DrawDeath(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x, Sint32 y,
                           bool facing_left, SotnPlayerDeath death, Uint16 *frame, size_t count);

/** @brief Draw selected stone palette and pivoted wiggle from private common art.
 * @param sprite Borrowed decoded sprite. @param common Verified F_GAME graphics/palettes.
 * @param x Screen anchor X. @param y Screen anchor Y. @param facing_left Mirror flag.
 * @param stone Active copied stone cursor. @param frame Writable RGB555 framebuffer.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS. @return Success or SDL error.
 * @note CPU rotation replaces GTE; statue fragments/lighting remain separate integration work.
 */
bool SotnAlucard_DrawPetrified(const SotnAlucardSprite *sprite, SotnDataView common, Sint32 x,
                               Sint32 y, bool facing_left, SotnPlayerStone stone, Uint16 *frame,
                               size_t count);

/** @brief Apply the selected original bounded cape palette-copy operation.
 * @param dra Borrowed private DRA. @param cape Original supported cape item.
 * @param colors Four initialized RGB555 words; unaddressed colors stay intact. Unchanged on
 * failure.
 * @return Success or false with SDL_GetError() for malformed table/pointer/opcode data. */
bool SotnAlucard_CapePalette(SotnDataView dra, Uint16 cape, Uint16 colors[4]);
/** @brief Advance Twilight cloak's original two-color glow once per simulation tick.
 * @param dra Borrowed private DRA. @param colors Four owned cape colors; first two animate.
 * @param target Initialized target index (0/1). @param delay Initialized countdown (0/1).
 * @return Success or SDL error; outputs remain unchanged on invalid data. */
bool SotnAlucard_TwilightTick(SotnDataView dra, Uint16 colors[4], Uint16 *target, Uint8 *delay);
#endif
