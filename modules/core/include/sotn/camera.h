#ifndef SOTN_CAMERA_H
#define SOTN_CAMERA_H
#include <SDL3/SDL.h>
/** @brief Original room-local camera bounds and followed position, owned by one scene. */
typedef struct SotnCamera
{
    Sint32 x, y, left, top, right, bottom;
    Uint32 width, height;
} SotnCamera;
/** @brief Initialize full room bounds and the original camera at an explicit development entry.
 * @param camera Writable owner. @param width Room pixels, multiple of 256, 256..16384.
 * @param height Room pixels with the same bounds. @param player_x Whole room-local player X.
 * @param player_y Whole room-local player Y. @return Success or SDL error; unchanged on failure. */
bool SotnCamera_Enter(SotnCamera *camera, Uint32 width, Uint32 height, Sint32 player_x,
                      Sint32 player_y);
/** @brief Apply the original horizontal anchor and four-pixel upward catch-up rule once per tick.
 * @param camera Initialized writable owner. @param player_x Whole room-local player X.
 * @param player_y Whole room-local player Y. @param snap Original immediate vertical-follow flag.
 * @return Success or SDL error; unchanged on invalid bounds. Rendering must only read this state.
 */
bool SotnCamera_Tick(SotnCamera *camera, Sint32 player_x, Sint32 player_y, bool snap);
/** @brief Set bounded original tilemap constraints after a room/event update.
 * @param camera Initialized owner. @param left Minimum X. @param top Minimum Y minus four.
 * @param right Exclusive X bound. @param bottom Original exclusive Y bound (may end at 0xFB).
 * @return Success or SDL error, with unchanged state on invalid bounds. */
bool SotnCamera_Bounds(SotnCamera *camera, Sint32 left, Sint32 top, Sint32 right, Sint32 bottom);
#endif
