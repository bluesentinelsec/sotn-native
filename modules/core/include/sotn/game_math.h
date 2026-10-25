#ifndef SOTN_GAME_MATH_H
#define SOTN_GAME_MATH_H
#include <sotn/game_data.h>
/** @brief Evaluate the original quantized SDK direction using the user's executable table.
 * @param executable Borrowed verified US boot image; table bytes stay private.
 * @param y Signed whole-pixel difference, -32767..32767.
 * @param x Signed whole-pixel difference, -32767..32767.
 * @param out Receives signed 4096-unit angle; unchanged on failure.
 * @return Success or SDL error for missing/truncated data, invalid bounds, or corrupt table. */
bool SotnMath_Atan2(SotnDataView executable, Sint32 y, Sint32 x, Sint16 *out);
/** @brief Evaluate the SDK direction for original signed fixed-point velocity vectors.
 * @param executable Borrowed private boot table. @param y Signed vector ordinate.
 * @param x Signed vector abscissa. @param out Unchanged on error; signed 4096-unit direction.
 * @return Success or SDL error. INT32_MIN is rejected; large ratios preserve SDK normalization. */
bool SotnMath_Atan2Vector(SotnDataView executable, Sint32 y, Sint32 x, Sint16 *out);
#endif
