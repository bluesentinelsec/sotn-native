#ifndef SOTN_ANIMATION_H
#define SOTN_ANIMATION_H

#include <sotn/game_data.h>

/** @brief Original Alucard animation cursor and the current frame's collision box. */
typedef struct SotnAnimation
{
    Uint16 id, pose, sprite;
    Sint32 remaining;
    Sint16 hitbox_x, hitbox_y;
    Uint8 hitbox_width, hitbox_height;
} SotnAnimation;

/**
 * @brief Select an original Alucard animation, resetting its pose and timer.
 * @param animation Borrowed writable cursor, unchanged on error.
 * @param id US animation table ID, less than 247.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 * @note Call SotnAnimation_Tick to resolve the first sprite from private DRA data.
 */
bool SotnAnimation_Set(SotnAnimation *animation, Uint16 id);

/**
 * @brief Advance one original simulation tick using runtime-loaded animation tables.
 * @param dra Borrowed immutable US DRA image; no tables are embedded in this library.
 * @param animation Borrowed cursor, updated atomically on success and unchanged on failure.
 * @return True on success, false with SDL_GetError() on invalid tables or cursor state.
 * @note Retains original frame durations, loops, animation transfers, subanimations,
 * and end holds. Traversal is bounded to reject malformed or unterminated data.
 */
bool SotnAnimation_Tick(SotnDataView dra, SotnAnimation *animation);

#endif
