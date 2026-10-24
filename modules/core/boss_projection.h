#ifndef SOTN_BOSS_PROJECTION_H
#define SOTN_BOSS_PROJECTION_H
#include <SDL3/SDL.h>
/** @brief Bounded original reward effect transform; angles use 4096 units and scale uses 4096. */
typedef struct SotnRewardProjection
{
    Sint16 x, y, z, radius, scale;
    Sint32 anchor_x, anchor_y;
} SotnRewardProjection;
/** @brief Copied projected opposite corners; negative scale reverses both pairs. */
typedef struct SotnProjectedRect
{
    Sint32 left, top, right, bottom;
} SotnProjectedRect;
/** @brief Project the selected reward sprite through the original two matrix/projection passes.
 * @param transform Initialized fixed-point effect state, radius within +/-128, anchor +/-16384.
 * @param out Copied screen-space corners, unchanged on failure.
 * @return Success or SDL error. No global GTE state or original tables are stored. */
bool SotnBoss_ProjectReward(SotnRewardProjection transform, SotnProjectedRect *out);
#endif
