#ifndef SOTN_PALETTE_H
#define SOTN_PALETTE_H
#include <SDL3/SDL_stdinc.h>
/** @brief Copied original stage palette interpolation command; contains no palette bytes.
 * An inactive command does nothing. Active commands interpolate sixteen RGB555 colors from
 * source into steps consecutive destination palettes, preserving transparency/mask bits. */
typedef struct SotnPaletteBlend
{
    Uint16 source, destination, steps;
    Uint8 red, green, blue;
    bool active;
} SotnPaletteBlend;
/** @brief One original NO2 red-channel increment for up to seven owned palette rows.
 * Colors 1..15 gain one saturated RGB555 red unit; color zero and other bits remain unchanged. */
typedef struct SotnPaletteRedShift
{
    Uint16 palettes[7];
    Uint8 count;
} SotnPaletteRedShift;
#endif
