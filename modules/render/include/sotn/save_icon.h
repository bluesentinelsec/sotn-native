#ifndef SOTN_SAVE_ICON_H
#define SOTN_SAVE_ICON_H

#include <sotn/game_data.h>

/** @brief Shape of each original save icon: three stacked 16x16 animation frames. */
enum
{
    SOTN_SAVE_ICON_COUNT = 16,
    SOTN_SAVE_ICON_WIDTH = 16,
    SOTN_SAVE_ICON_HEIGHT = 48,
    SOTN_SAVE_ICON_RGBA_SIZE = 16 * 48 * 4
};

/**
 * @brief Decode one US DRA save icon into top-to-bottom RGBA8 pixels.
 * @param dra Borrowed DRA file bytes; use the validated SOTN_GAME_DRA view in production.
 * @param index Icon index from zero through SOTN_SAVE_ICON_COUNT - 1.
 * @param rgba Caller-owned output, at least SOTN_SAVE_ICON_RGBA_SIZE bytes.
 * @param capacity Available output bytes.
 * @return True on success, otherwise false with an SDL error and output untouched.
 * @note Decodes the original 4-bit palette indices and RGB555 colors. Zero colors are
 * transparent; the STP bit does not implement PS1 primitive semi-transparency in this preview.
 * No game bytes are embedded in this decoder. No allocation or SDL initialization is needed.
 */
bool SotnSaveIcon_Decode(SotnDataView dra, Uint32 index, Uint8 *rgba, size_t capacity);

#endif
