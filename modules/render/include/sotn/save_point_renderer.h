#ifndef SOTN_SAVE_POINT_RENDERER_H
#define SOTN_SAVE_POINT_RENDERER_H

#include <sotn/save_point.h>
#include <sotn/tilemap.h>

/** @brief Owned bounded mesh cache borrowing private DRA and common graphics. */
typedef struct SotnSavePointRenderer SotnSavePointRenderer;
/** @brief Decode selected original save-point meshes, UVs, and coffin palette.
 * @param dra Borrowed private image. @param common Borrowed F_GAME.BIN.
 * @param out Receives ownership, NULL on failure. Inputs outlive the renderer.
 * @return Success, or false with SDL_GetError() for invalid private data/allocation. */
bool SotnSavePointRenderer_Create(SotnDataView dra, SotnDataView common,
                                  SotnSavePointRenderer **out);
/** @brief Draw one priority half of the original point around the player.
 * @param renderer Borrowed cache. @param point Copied simulation state.
 * @param front False before player, true after player. @param frame Writable RGB555 frame.
 * @param count At least SOTN_FRAME_PIXELS. @return Success or false with SDL_GetError().
 * @note Selected CPU projection replaces GTE; complete lighting/effect parity is unproven. */
bool SotnSavePointRenderer_Draw(const SotnSavePointRenderer *renderer, SotnSavePoint point,
                                bool front, Uint16 *frame, size_t count);
/** @brief Draw the selected save-point world mesh with the shared presentation offset.
 * @param renderer Borrowed cache. @param point Copied simulation state.
 * @param front Original priority half. @param offset_x Shared screen offset, -256..256.
 * @param offset_y Shared screen offset, -256..256. @param frame Writable RGB555 frame.
 * @param count At least SOTN_FRAME_PIXELS. @return Success or SDL error before writes for
 * invalid input. This shifts projected vertices only, leaving simulation and dialogs fixed. */
bool SotnSavePointRenderer_DrawOffset(const SotnSavePointRenderer *renderer, SotnSavePoint point,
                                      bool front, Sint32 offset_x, Sint32 offset_y, Uint16 *frame,
                                      size_t count);
/** @brief Draw the native slot/confirmation adapter using the original font.
 * @param renderer Borrowed cache. @param point Copied state. @param gamepad Show Xbox hints.
 * @param frame Writable RGB555 frame. @param count At least SOTN_FRAME_PIXELS.
 * @return Success or false with SDL_GetError(). */
bool SotnSavePointRenderer_Dialog(const SotnSavePointRenderer *renderer, SotnSavePoint point,
                                  bool gamepad, Uint16 *frame, size_t count);
/** @brief Free cache without freeing borrowed data. @param renderer Owned cache or NULL. */
void SotnSavePointRenderer_Destroy(SotnSavePointRenderer *renderer);
#endif
