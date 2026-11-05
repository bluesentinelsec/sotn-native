#ifndef SOTN_PLAYER_TIMERS_H
#define SOTN_PLAYER_TIMERS_H

/** @brief Original player timer indices; independent of the HUD stat-buff counters.
 * Values are signed sixteen-bit words in the source, retained as raw word bits by
 * native snapshots. Producers and expiry helpers determine their actual availability. */
typedef enum SotnPlayerTimerIndex
{
    SOTN_PLAYER_TIMER_POISON = 0,
    SOTN_PLAYER_TIMER_CURSE,
    SOTN_PLAYER_TIMER_HIT_EFFECT,
    SOTN_PLAYER_TIMER_PALETTE,
    SOTN_PLAYER_TIMER_COLOR,
    SOTN_PLAYER_TIMER_COYOTE,
    SOTN_PLAYER_TIMER_FALL,
    SOTN_PLAYER_TIMER_DROP,
    SOTN_PLAYER_TIMER_IMPACT,
    SOTN_PLAYER_TIMER_WEAPON,
    SOTN_PLAYER_TIMER_SUBWEAPON,
    SOTN_PLAYER_TIMER_DARK,
    SOTN_PLAYER_TIMER_SPELL,
    SOTN_PLAYER_TIMER_INVINCIBLE,
    SOTN_PLAYER_TIMER_INVINCIBLE_CONSUMABLES,
    SOTN_PLAYER_TIMER_AFTERIMAGE,
    SOTN_PLAYER_TIMERS
} SotnPlayerTimerIndex;

#endif
