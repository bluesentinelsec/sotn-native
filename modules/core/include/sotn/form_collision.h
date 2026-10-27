#ifndef SOTN_FORM_COLLISION_H
#define SOTN_FORM_COLLISION_H

#include <sotn/collision.h>

/** @brief Original Mist-only barrier escape decision; apply velocity only when blocked. */
typedef struct SotnFormBarrier
{
    bool blocked;
    Sint32 velocity_x;
} SotnFormBarrier;

/** @brief Original floor-edge nudge; displaced can be true even when shift_x is zero. */
typedef struct SotnFormEdge
{
    bool displaced;
    Sint32 shift_x;
} SotnFormEdge;

/** @brief Probe the original two Mist-only barrier sensors before restoring normal Alucard.
 * @param map Borrowed immutable room collision data. @param x Whole-pixel player X.
 * @param y Whole-pixel player Y. Coordinates must fit an original signed position word.
 * @param facing_left Original direction for escape when both sensors touch a barrier.
 * @param out Receives the escape decision; unchanged on failure.
 * @return Success or SDL error for invalid coordinates, map data or output.
 * This does not complete restoration or bypass actual transformation-effect readiness. */
bool SotnFormCollision_Barrier(const SotnCollisionMap *map, Sint32 x, Sint32 y, bool facing_left,
                               SotnFormBarrier *out);

/** @brief Evaluate the original side-of-floor ejection used during form restoration.
 * @param map Borrowed immutable room collision data. @param x Whole-pixel player X.
 * @param y Whole-pixel player Y. Coordinates fit an original signed position word.
 * @param floor_sensor_y Current original second floor sensor's signed Y offset.
 * @param ignore_mist True for the original Mist reset path, false for the shared normal path.
 * @param out Receives a whole-pixel nudge; preserve the owner's coordinate fraction on apply.
 * @return Success or SDL error with unchanged output for invalid data.
 * The original second-probe Mist quirk is retained: its solid result still blocks ejection. */
bool SotnFormCollision_FloorEdge(const SotnCollisionMap *map, Sint32 x, Sint32 y,
                                 Sint16 floor_sensor_y, bool ignore_mist, SotnFormEdge *out);

#endif
