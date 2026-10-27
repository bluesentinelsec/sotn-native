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

/** @brief Advance Alucard with the original mutable transformation pose.
 * @param dra Borrowed verified DRA. @param form_pose Actual displayed capture pose, below 512.
 * @param animation Owned cursor, unchanged on failure. @return Success or SDL error.
 * Replaces only the original record at DRA offset FDA4 without mutating private input bytes.
 * Other timelines, controls, durations and frame properties retain ordinary semantics. */
bool SotnAnimation_TickPlayerForm(SotnDataView dra, Uint16 form_pose, SotnAnimation *animation);

/** @brief Select a supported overlay's player-format timeline without copying runtime tables.
 * @param animation Borrowed cursor. @param stage Original stage ID; currently BO4 (26).
 * @param id Original timeline ID, below 220 for BO4.
 * @return Success or SDL error, retaining the cursor on failure. */
bool SotnAnimation_SetStage(SotnAnimation *animation, Uint16 stage, Uint16 id);
/** @brief Advance an overlay player-format cursor with bounded original control semantics.
 * @param image Borrowed immutable verified overlay. @param stage Currently BO4 (26).
 * @param animation Owned cursor, unchanged on failure. @return Success or SDL error.
 * @note BO4 owns 220 timelines, 29 subanimations and 17 frame-property records. The original
 * property-19 access reads adjacent timeline-pointer bytes from the image,
 * without a native out-of-bounds array read.
 * This does not select the drawing bank or implement actor movement/AI. */
bool SotnAnimation_TickStage(SotnDataView image, Uint16 stage, SotnAnimation *animation);

/** @brief Advance BO4 animation with its original mutable form-pose record.
 * @param image Borrowed verified BO4 overlay. @param form_pose Original replacement sprite, <512.
 * @param animation Owned cursor, unchanged on failure. @return Success or SDL error.
 * @note Replaces only the original record at BO4 offset3B0C; the immutable input
 * image and all other timeline/property/control records remain unchanged. */
bool SotnAnimation_TickDopple(SotnDataView image, Uint16 form_pose, SotnAnimation *animation);

#endif
