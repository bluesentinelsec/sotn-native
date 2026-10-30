#ifndef SOTN_WATER_VOLUME_H
#define SOTN_WATER_VOLUME_H
#include <sotn/game_data.h>

/** @brief Actual original player/scroll words and private ordinary water-volume records. */
typedef struct SotnWaterVolumeInput
{
    SotnDataView volumes,
        aspects; /**< Borrowed eight-word rows and eight private slope divisors. */
    Sint16 player_x, player_y, scroll_x, scroll_y;
    Uint16 parameters; /**< Original low-byte first row, high-byte nonzero row count. */
    Uint32 status;
} SotnWaterVolumeInput;
/** @brief Original selected volume and three distinct depth globals; zero when outside water. */
typedef struct SotnWaterVolume
{
    Sint16 x, y, left,
        right; /**< Wrapped world sampling point and selected horizontal distances. */
    Uint16 raw, feet, waist, head, row, body_height, foot_offset;
    bool
        contact; /**< Original encoded result is nonzero; depth zero can still contact a surface. */
} SotnWaterVolume;
/** @brief Query original ordinary-stage water geometry and form-dependent depths.
 * @param input Actual copied words/status and bounded private table views.
 * @param out Receives selected first row and derived depths, unchanged on failure.
 * @return Success or SDL error for malformed selectors, tables or slope references.
 * Preserves signed-word intermediate wrapping, truncating slope division, strict right/bottom
 * edges, first-match order, crouch/form precedence and the original high-bit contact encoding.
 * Outside water, unused row/distances initialize to zero instead of exposing unwritten original
 * scratch. They confer no selection/readiness. The owner still supplies actual placed actors,
 * birth/update order, currents, splash factories, native player feedback and damage behavior;
 * this query advances no clocks/RNG and enables no form controls by itself.
 */
bool SotnWaterVolume_Query(SotnWaterVolumeInput input, SotnWaterVolume *out);
#endif
