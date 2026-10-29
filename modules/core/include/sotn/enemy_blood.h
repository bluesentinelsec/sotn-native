#ifndef SOTN_ENEMY_BLOOD_H
#define SOTN_ENEMY_BLOOD_H
#include <sotn/game_math.h>

#define SOTN_ENEMY_BLOOD_PARTICLES 12
/** @brief Original procedural tile and its independent packed control fractions. */
typedef struct SotnEnemyBloodParticle
{
    Sint32 fixed_x, fixed_y, velocity_x, velocity_y, acceleration_x, acceleration_y;
    Sint16 x, y, timer, interval;
    Uint8 width, height;
} SotnEnemyBloodParticle;
/** @brief One actual stage blood actor; its owner retains twelve real tile rows. */
typedef struct SotnEnemyBlood
{
    SotnEnemyBloodParticle particles[SOTN_ENEMY_BLOOD_PARTICLES];
    Sint32 x, y, velocity_x, acceleration;
    Uint16 timer, size;
    Uint8 phase, hitbox_width, hitbox_height;
    bool hitbox, removed;
} SotnEnemyBlood;
/** @brief Current source, actual primitive allocation and player/contact/healing context. */
typedef struct SotnEnemyBloodInput
{
    SotnDataView executable;
    Sint32 x, y; /**< Screen-space16.16 source, used only on actual birth. */
    bool facing_left, allocated, absorbing, hit, heal_pending, bloodstone;
} SotnEnemyBloodInput;
/** @brief Original deferred player-healing request and consumed blood contact. */
typedef struct SotnEnemyBloodEvents
{
    Uint16 heal;
    bool contact;
} SotnEnemyBloodEvents;
/** @brief Advance original enemy blood body, twelve particles and absorption requests once.
 * @param blood Copied actual actor, initially zeroed. @param input Actual allocation/context.
 * @param random Owned original Random global snapshot, not the separate combat rand state.
 * @param out Receives this update's deferred heal/contact request.
 * @return Success or SDL error preserving state/RNG/output on malformed input or SDK data.
 * Failed initial row allocation retires before RNG. Successful birth consumes four Random
 * values per actual tile in original descending particle order. The owner commits stage rows,
 * contact flags, first-request-wins healing and next-player-update effects; no HP is changed here.
 */
bool SotnEnemyBlood_Tick(SotnEnemyBlood *blood, SotnEnemyBloodInput input, Uint32 *random,
                         SotnEnemyBloodEvents *out);
/** @brief Rebase screen-space body/tiles without advancing packed fractions or simulation.
 * @param blood Owned actor. @param dx Signed-word whole-pixel X shift. @param dy Same for Y.
 * @return Success or SDL error without mutation. Cached control high words refresh on update. */
bool SotnEnemyBlood_Translate(SotnEnemyBlood *blood, Sint32 dx, Sint32 dy);
#endif
