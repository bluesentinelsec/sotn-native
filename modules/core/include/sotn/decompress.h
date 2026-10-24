#ifndef SOTN_DECOMPRESS_H
#define SOTN_DECOMPRESS_H

#include <sotn/game_data.h>

/**
 * @brief Decode the game's dictionary/RLE graphics stream into packed 4-bit pixels.
 * @param source Borrowed compressed bytes, including the eight-byte dictionary.
 * @param pixels Borrowed writable output; must not overlap source.
 * @param capacity Available output bytes, from 1 byte through 64 MiB.
 * @param written Receives decoded byte count, or zero on failure; must not alias pixels.
 * @return True after a terminating opcode, false with SDL_GetError() on invalid input.
 * @note Reads compressed nibbles high-first and writes pixels low-first. A validation
 * pass checks the whole stream before modifying output. Trailing source bytes are ignored.
 */
bool SotnGraphics_Decompress(SotnDataView source, Uint8 *pixels, size_t capacity, size_t *written);

#endif
