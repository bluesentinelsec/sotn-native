#ifndef SOTN_PLAYER_SMOKE_H
#define SOTN_PLAYER_SMOKE_H
#include <sotn/player_factory.h>
/** @brief Actual live player/collision snapshot used by the original entity-two wall/skid births.
 */
typedef struct SotnPlayerSmokeEnvironment
{
    Sint32 x, y, velocity_x; /**< Live player screen-space position and horizontal speed. */
    struct
    {
        Uint32 effects;
        Sint16 x, y; /**< Actual relative wall sensor, distinct from the captured child origin. */
    } wall[14];
    bool wall_owned; /**< Set only after the actual player owner completes its wall queries. */
} SotnPlayerSmokeEnvironment;
/** @brief Selected original entity-two puff with its private animation cursor. */
typedef struct SotnPlayerSmoke
{
    Sint32 x, y, velocity_x, velocity_y;
    Sint16 remaining;
    Uint16 pose, sprite, scale, priority, parameter;
    Uint8 phase;
    bool facing_left, removed;
} SotnPlayerSmoke;
/** @brief Reconstruct selected original entity-two ground/slide/walking/slope profiles and
 * animation.
 * @param smoke Actual zero-initialized child's copied state. @param dra Private tables/timeline.
 * @param source Actual child source projected into screen space on birth.
 * @param parameter Child parameter, high byte0/1/5/10; walking profile5 permits low byte0..2.
 * @param priority Current player priority on birth. @param contacts Current original contact word.
 * @param status Current original player status. The US uninitialized-parameter branch is rejected
 * explicitly rather than reading unwritten memory or inventing a deterministic original result.
 * @return Success or SDL error, preserving state on invalid input. Allocates no entity or rows.
 * Owner advances once at the actual child slot and retires removed children. */
bool SotnPlayerSmoke_Tick(SotnPlayerSmoke *smoke, SotnDataView dra, SotnPlayerFactorySource source,
                          Uint16 parameter, Uint16 priority, Uint32 contacts, Uint32 status);
/** @brief Advance selected ground/slide/walking/skid/wall/slope smoke with actual live feedback.
 * @param smoke Actual copied child. @param dra Private tables and animation.
 * @param source Captured child origin projected into screen space.
 * @param parameter High byte0/1/4/5/6/8/10; low byte is the original child index.
 * @param priority Actual player priority. @param contacts Actual player contact word.
 * @param status Actual player status; the undefined US wet-smoke branch still fails explicitly.
 * @param environment Live player snapshot; required for skid/wall births. Wall births require
 * completed actual wall queries, use the private sensor order and retain the child fractions.
 * @return Success or SDL error without mutation. No matching wall retires the actual child.
 * Allocates no entity/rows and consumes no RNG. */
bool SotnPlayerSmoke_TickOwned(SotnPlayerSmoke *smoke, SotnDataView dra,
                               SotnPlayerFactorySource source, Uint16 parameter, Uint16 priority,
                               Uint32 contacts, Uint32 status,
                               const SotnPlayerSmokeEnvironment *environment);
/** @brief Rebase retained screen-space fractions without advancing the animation.
 * @param smoke Actual child. @param x Horizontal pixels. @param y Vertical pixels.
 * @return Success or SDL error without mutation on invalid child metadata. */
bool SotnPlayerSmoke_Rebase(SotnPlayerSmoke *smoke, Sint16 x, Sint16 y);
#endif
