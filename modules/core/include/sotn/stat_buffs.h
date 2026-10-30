#ifndef SOTN_STAT_BUFFS_H
#define SOTN_STAT_BUFFS_H
#include <SDL3/SDL_stdinc.h>

enum
{
    SOTN_STAT_BUFFS = 16,
    SOTN_STAT_BUFF_DURATION = 4096
};
/** @brief Original twelve timed stat effects; the other four DRA words have no selected stat
 * effect. */
typedef enum SotnStatBuffId
{
    SOTN_BUFF_DEFENSE,
    SOTN_BUFF_ATTACK,
    SOTN_BUFF_LUCK,
    SOTN_BUFF_INTELLIGENCE,
    SOTN_BUFF_STRENGTH,
    SOTN_BUFF_FIRE,
    SOTN_BUFF_ICE,
    SOTN_BUFF_THUNDER,
    SOTN_BUFF_CURSE,
    SOTN_BUFF_HOLY,
    SOTN_BUFF_STONE,
    SOTN_BUFF_DARK
} SotnStatBuffId;
/** @brief Actual runtime DRA counters, independent of ordinary character/save data. */
typedef struct SotnStatBuffs
{
    Sint32 timers[SOTN_STAT_BUFFS];
} SotnStatBuffs;
/** @brief Validate actual counters and return their derived active bits without mutation.
 * @param state Actual retained owner. @param mask Optional derived mask, unchanged on error.
 * @return Success or SDL error for a missing owner or a counter outside0..4096. */
bool SotnStatBuffs_Mask(const SotnStatBuffs *state, Uint16 *mask);
/** @brief Decrement all sixteen original words once at the actual HUD update.
 * @param state Actual owner. @return Success or SDL error preserving state on invalid counters.
 * Menus/map/draws must not call this. Original spell-frozen/release HUD updates do call it. */
bool SotnStatBuffs_Tick(SotnStatBuffs *state);
/** @brief Apply an actual producer's original minimum-duration request, preserving longer buffs.
 * @param state Actual owner. @param id Original word0..15. @param minimum Bounded0..4096.
 * @return Success or SDL error without mutation. This API enables no item/control by itself. */
bool SotnStatBuffs_Minimum(SotnStatBuffs *state, Uint16 id, Uint16 minimum);
/** @brief Original GiveStatBuff4096 for an actual producer; no resource/acquisition is granted. */
bool SotnStatBuffs_Give(SotnStatBuffs *state, Uint16 id);
/** @brief Clear one original bounded word; invalid input preserves the owner. */
bool SotnStatBuffs_Clear(SotnStatBuffs *state, Uint16 id);
/** @brief Original full play initialization clears all sixteen counters, without a HUD tick. */
bool SotnStatBuffs_Reset(SotnStatBuffs *state);
#endif
