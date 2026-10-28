#ifndef SOTN_PLAYER_SMOKE_H
#define SOTN_PLAYER_SMOKE_H
#include <sotn/player_factory.h>
/** @brief Selected original entity-two ground/slide puff, with private animation cursor. */
typedef struct SotnPlayerSmoke
{
    Sint32 x, y, velocity_x, velocity_y;
    Sint16 remaining;
    Uint16 pose, sprite, scale, priority, parameter;
    Uint8 phase;
    bool facing_left, removed;
} SotnPlayerSmoke;
/** @brief Reconstruct selected original entity-two profiles zero/one and animation.
 * @param smoke Actual zero-initialized child's copied state. @param dra Private tables/timeline.
 * @param source Actual child source projected into screen space on birth.
 * @param parameter Child parameter, high byte0/1.
 * @param priority Current player priority on birth. @param contacts Current original contact word.
 * @param status Current original player status. The US uninitialized-parameter branch is rejected
 * explicitly rather than reading unwritten memory or inventing a deterministic original result.
 * @return Success or SDL error, preserving state on invalid input. Allocates no entity or rows.
 * Owner advances once at the actual child slot and retires removed children. */
bool SotnPlayerSmoke_Tick(SotnPlayerSmoke *smoke, SotnDataView dra, SotnPlayerFactorySource source,
                          Uint16 parameter, Uint16 priority, Uint32 contacts, Uint32 status);
/** @brief Rebase retained screen-space fractions without advancing the animation.
 * @param smoke Actual child. @param x Horizontal pixels. @param y Vertical pixels.
 * @return Success or SDL error without mutation on invalid child metadata. */
bool SotnPlayerSmoke_Rebase(SotnPlayerSmoke *smoke, Sint16 x, Sint16 y);
#endif
