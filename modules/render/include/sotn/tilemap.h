#ifndef SOTN_TILEMAP_H
#define SOTN_TILEMAP_H

#include <sotn/stage.h>

/** @brief Original frame dimensions used by the stage presentation slice. */
enum
{
    SOTN_FRAME_WIDTH = 256,
    SOTN_FRAME_HEIGHT = 240,
    SOTN_FRAME_PIXELS = 256 * 240
};

/** @brief Borrowed private tile graphics and common graphics/palettes. */
typedef struct SotnTileSources
{
    SotnDataView stage;  /**< F_NZ0.BIN, exactly 256 KiB. */
    SotnDataView common; /**< F_GAME.BIN, exactly 264 KiB. */
} SotnTileSources;

/**
 * @brief Rasterize one original tile layer into a 256x240 RGB555 framebuffer.
 * @param layer Borrowed decoded stage layer, with immutable views valid during the call.
 * @param sources Borrowed verified graphics resources, alive during the call.
 * @param scroll_x Horizontal scroll in pixels relative to the layer origin.
 * @param scroll_y Vertical scroll in pixels relative to the layer origin.
 * @param frame Borrowed writable RGB555 framebuffer; must not overlap input data.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return True on success, false with SDL_GetError() on invalid references or unsupported pages.
 * @note Validates before writing. Hidden/absent layers leave the frame unchanged.
 * Retains transparent texels, 4/8-bit addressing, layer wrapping, and PS1 blend arithmetic.
 * This does not draw entities, animate palettes, or emulate the complete PS1 GPU.
 */
bool SotnTilemap_Draw(const SotnStageLayer *layer, SotnTileSources sources, Sint32 scroll_x,
                      Sint32 scroll_y, Uint16 *frame, size_t count);

/**
 * @brief Expand a 256x240 RGB555 framebuffer to opaque top-to-bottom RGBA8.
 * @param frame Borrowed SOTN_FRAME_PIXELS RGB555 values; alive during the call.
 * @param count Available input pixels, at least SOTN_FRAME_PIXELS.
 * @param rgba Borrowed writable output, not overlapping frame.
 * @param capacity Available output bytes, at least SOTN_FRAME_PIXELS * 4.
 * @return True on success, false with SDL_GetError() for invalid buffers.
 */
bool SotnFrame_ToRgba(const Uint16 *frame, size_t count, Uint8 *rgba, size_t capacity);

#endif
