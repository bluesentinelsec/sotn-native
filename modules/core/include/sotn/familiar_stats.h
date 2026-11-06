#ifndef SOTN_FAMILIAR_STATS_H
#define SOTN_FAMILIAR_STATS_H
#include <sotn/game_data.h>
enum
{
    SOTN_FAMILIAR_COUNT = 7
};
/** @brief Original three signed words, including menu load count, for each familiar.
 * These records grant no card or active familiar. Ordinary v11 saves encode them. */
typedef struct SotnFamiliarStats
{
    Sint32 level, experience, loads;
} SotnFamiliarStats;
typedef struct SotnFamiliarProgress
{
    SotnFamiliarStats records[SOTN_FAMILIAR_COUNT];
} SotnFamiliarProgress;
/** @brief Initialize all seven original records to level1/EXP0/loads0, without grants. */
bool SotnFamiliarProgress_Init(SotnFamiliarProgress *progress);
/** @brief Validate all seven records, including level/EXP agreement, without mutation. */
bool SotnFamiliarProgress_Validate(const SotnFamiliarProgress *progress);
/** @brief Commit one actual changed/reloaded menu selection, saturating its load count.
 * @param progress Actual progression owner. @param familiar Original identity1..7.
 * Room entry and unchanged menu selection must not call this operation. */
bool SotnFamiliarProgress_Load(SotnFamiliarProgress *progress, Uint16 familiar);
/** @brief Apply original raw enemy reward to the selected familiar, without RNG.
 * @param progress Actual seven records. @param familiar Original identity0..7;0 holds all.
 * @param experience Raw enemy reward before player scaling or equipment adjustments.
 * @param player_level Actual player level before reward;99 and zero reward hold all records.
 * @return Success or SDL error without mutation. Grants neither selection nor persistence. */
bool SotnFamiliarProgress_Reward(SotnFamiliarProgress *progress, Uint16 familiar, Uint16 experience,
                                 Uint16 player_level);
#endif
