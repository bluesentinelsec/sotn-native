#ifndef SOTN_PLAYER_GLOW_H
#define SOTN_PLAYER_GLOW_H
#include <sotn/game_data.h>
/** @brief Original retained one-color player glow cursor; owner reserves real palette job23. */
typedef struct SotnPlayerGlow
{
    Uint16 color, targets[2];
    Uint16 interval, delay;
    Uint8 phase;
    bool absorbing, initialized;
} SotnPlayerGlow;
/** @brief Initialize from the private original job23 and normal absorption palette pair.
 * @param glow Receives a new cursor. @param dra Borrowed validated main image.
 * @return Success or SDL error preserving output. This does not reserve a palette job. */
bool SotnPlayerGlow_Init(SotnPlayerGlow *glow, SotnDataView dra);
/** @brief Apply an actual absorption timer edge without restarting the retained glow job.
 * @param glow Owned initialized cursor. @param dra Borrowed private pairs.
 * @param absorbing Current timer presence. @return Success or SDL error without mutation. */
bool SotnPlayerGlow_Set(SotnPlayerGlow *glow, SotnDataView dra, bool absorbing);
/** @brief Advance the original palette job once, retaining mask bit and arrival wait.
 * @param glow Owned initialized cursor with its actual job lease. @return Success or SDL error.
 * Render calls must not advance this cursor. Palette jobs continue through engine pause. */
bool SotnPlayerGlow_Tick(SotnPlayerGlow *glow);
#endif
