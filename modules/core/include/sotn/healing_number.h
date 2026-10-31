#ifndef SOTN_HEALING_NUMBER_H
#define SOTN_HEALING_NUMBER_H
#include <sotn/player_blink.h>
#include <sotn/player_factory.h>
enum
{
    SOTN_HEALING_NUMBER_ROWS = 20
};
/** @brief Original entity19 healing number and its sixteen one-pixel trail tiles.
 * Storage belongs to an actual shared child; this component allocates no entity or rows. */
typedef struct SotnHealingNumber
{
    SotnPlayerEffectQuad digits[4];
    struct
    {
        Sint16 x, y;
        Uint8 timer;
        bool started, hidden;
    } tiles[16];
    Sint32 x, y;
    Sint16 half_width, half_height, timer, lower_x, angle, distance, swirl;
    Uint16 number;
    Uint8 phase, extra_digits;
    bool removed;
} SotnHealingNumber;
/** @brief Advance the original green healing number once, including its motionless birth.
 * @param number Zeroed actual child or retained live state. Set number before first update.
 * @param executable Borrowed private SDK tables. @param source Captured screen-space player
 * position, used only at birth. @param allocated Actual twenty-row reservation outcome.
 * @return Success or SDL error preserving state. Exhaustion retires only this optional visual.
 * Deferred healing and contact absorption use this same type1 visual, including full HP and
 * stone. Original unsigned-word amounts above9999 display their four least significant digits. */
bool SotnHealingNumber_Tick(SotnHealingNumber *number, SotnDataView executable,
                            SotnPlayerFactorySource source, bool allocated);
/** @brief Translate retained coordinates/packets without updating animation or tile timers.
 * @param number Actual live child. @param x Signed screen offset. @param y Signed screen offset.
 * @return Success or SDL error; fresh children retain their source until birth. */
bool SotnHealingNumber_Rebase(SotnHealingNumber *number, Sint16 x, Sint16 y);
#endif
