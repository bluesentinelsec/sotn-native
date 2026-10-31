#ifndef SOTN_RISING_HEART_H
#define SOTN_RISING_HEART_H
#include <sotn/player_factory.h>
/** @brief Original child51 motion, owned by its actual shared factory slot; no primitive lease. */
typedef struct SotnRisingHeart
{
    Sint32 x, y;
    Sint16 angle, speed;
    Uint16 timer, priority;
    bool initialized, blink, removed;
} SotnRisingHeart;
/** @brief Advance EntitySmallRisingHeart once, including its motionless birth update.
 * @param heart Zeroed fresh child or retained live state. @param executable Private SDK trig.
 * @param source Actual factory source in screen space; used only at birth.
 * @param player_priority Live player priority at birth, at least two.
 * @param game_timer Actual wrapping gameplay clock for the original modulo-four speed change.
 * @return Success or SDL error with unchanged state. No RNG or substitute artwork is used. */
bool SotnRisingHeart_Tick(SotnRisingHeart *heart, SotnDataView executable,
                          SotnPlayerFactorySource source, Uint16 player_priority,
                          Uint32 game_timer);
/** @brief Translate retained screen coordinates without advancing motion or lifetime.
 * @param heart Actual child. @param x Signed camera/room offset. @param y Signed offset.
 * @return Success or SDL error; fresh children retain their factory source until birth. */
bool SotnRisingHeart_Rebase(SotnRisingHeart *heart, Sint16 x, Sint16 y);
#endif
