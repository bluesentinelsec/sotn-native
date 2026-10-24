#ifndef SOTN_INVENTORY_RENDERER_H
#define SOTN_INVENTORY_RENDERER_H
#include <sotn/inventory_menu.h>
#include <sotn/tilemap.h>
enum
{
    SOTN_MENU_WIDTH = 384,
    SOTN_MENU_HEIGHT = 240,
    SOTN_MENU_RGBA_BYTES = 384 * 240 * 4
};
/** @brief Owned menu frame borrowing private image/common artwork. */
typedef struct SotnInventoryRenderer SotnInventoryRenderer;
/** @brief Allocate the selected original 384-pixel menu presentation.
 * @param dra Borrowed DRA. @param common Borrowed F_GAME.BIN. @param out Owned output, NULL on
 * failure.
 * @return Success or false with SDL_GetError(). Borrowed bytes outlive this object. */
bool SotnInventoryRenderer_Create(SotnDataView dra, SotnDataView common,
                                  SotnInventoryRenderer **out);
/** @brief Draw supported original status/equipment panels with private font/portrait/icons.
 * @param renderer Borrowed renderer. @param menu Copied valid cursor. @param character Borrowed
 * state.
 * @param ticks Elapsed gameplay ticks. @param gamepad Show Xbox hints. @param rgba Output buffer.
 * @param capacity At least SOTN_MENU_RGBA_BYTES. @return Success or false with SDL_GetError(). */
bool SotnInventoryRenderer_Draw(SotnInventoryRenderer *renderer, SotnInventoryMenu menu,
                                const SotnCharacter *character, Uint64 ticks, bool gamepad,
                                Uint8 *rgba, size_t capacity);
/** @brief Draw an original item icon into a gameplay RGB555 frame, clipped to its bounds.
 * @param dra Borrowed image. @param item Decoded icon metadata. @param x Left edge. @param y Top
 * edge.
 * @param frame Writable gameplay frame. @param count At least SOTN_FRAME_PIXELS.
 * @return Success or false with SDL_GetError(), unchanged frame on invalid metadata. */
bool SotnItemIcon_Draw(SotnDataView dra, SotnItem item, Sint32 x, Sint32 y, Uint16 *frame,
                       size_t count);
/** @brief Free owned frame, retaining borrowed data. @param renderer Owned or NULL. */
void SotnInventoryRenderer_Destroy(SotnInventoryRenderer *renderer);
#endif
