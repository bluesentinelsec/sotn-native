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
#endif
