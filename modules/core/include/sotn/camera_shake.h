#ifndef SOTN_CAMERA_SHAKE_H
#define SOTN_CAMERA_SHAKE_H
#include <sotn/game_data.h>
/** @brief Original single shared shake request; zero-initialize at stage initialization.
 * Pattern offsets refer to borrowed immutable DRA data. X/Y survive request replacement:
 * the original service writes only the selected axis, including its final zero. */
typedef struct SotnCameraShake
{
    Uint32 pattern;
    Uint16 type, index;
    Sint16 amount;
    Sint32 x, y;
} SotnCameraShake;
/** @brief Replace the original shared request using its private pattern pointer table.
 * @param shake Owned initialized cursor. @param dra Borrowed immutable private image.
 * @param type Original identity 0..6; zero and five retain their original no-op updates.
 * @return Success or SDL error without mutation. Replacement restarts the index, preserving
 * current amount and both presentation offsets; it never moves camera follow or collision. */
bool SotnCameraShake_Request(SotnCameraShake *shake, SotnDataView dra, Uint16 type);
/** @brief Advance one original simulation update, retaining last-request-wins behavior.
 * @param shake Owned cursor. @param dra Borrowed immutable private image used at request time.
 * @return Success or SDL error without mutation on malformed cursor or table references.
 * The next-sample terminator clears the selected axis on that same update, discarding the
 * last pattern sample. Other-axis offsets survive. Rendering must only read this cursor. */
bool SotnCameraShake_Tick(SotnCameraShake *shake, SotnDataView dra);
#endif
