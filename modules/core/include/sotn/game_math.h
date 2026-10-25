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
/** @brief Original SDK sine/cosine values, in units of 1/4096. */
typedef struct SotnMathTrig
{
    Sint16 sine, cosine;
} SotnMathTrig;
/** @brief Resolve periodic SDK trigonometry from the private quarter-wave table.
 * @param executable Borrowed US boot image. @param angle Signed 4096-unit angle.
 * @param out Receives both values atomically. @return Success or SDL error. */
bool SotnMath_Trig(SotnDataView executable, Sint32 angle, SotnMathTrig *out);
/** @brief Evaluate the original SDK table-based twelve-bit square root.
 * @param executable Borrowed US boot image, retaining the private 192-word table.
 * @param value Nonnegative original signed-word input (<=INT32_MAX).
 * @param out Receives sqrt(value)*64 using original quantization, unchanged on error.
 * @return Success or SDL error for missing/corrupt data or invalid input. */
bool SotnMath_Sqrt12(SotnDataView executable, Uint32 value, Uint32 *out);
/** @brief Resolve the SDK matrix sine/cosine pair, distinct from rsin/rcos.
 * @param executable Borrowed verified US boot image, retaining the original 4096 pairs.
 * @param angle Original signed-word angle. @param out Receives both values atomically.
 * @return Success or SDL error. Preserves rcossin's absolute-angle/sign order;
 * never substitutes the numerically different quarter-wave table. */
bool SotnMath_MatrixTrig(SotnDataView executable, Sint16 angle, SotnMathTrig *out);
#endif
