#ifndef SOTN_SOUL_ORB_H
#define SOTN_SOUL_ORB_H
#include <sotn/game_math.h>

/** @brief One actual common stage Soul Steal orb, using screen-space16.16 positions. */
typedef struct SotnSoulOrb
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint16 angle, turn, speed, age, scale_x, scale_y, pose;
    Sint16 remaining;
    Uint8 phase, frame;
    bool hitbox, removed;
} SotnSoulOrb;
/** @brief Borrowed ordinary US stage tables and actual primitive/contact context. */
typedef struct SotnSoulOrbInput
{
    SotnDataView executable, tables; /**< Private16-byte angles/96-byte shapes/14-byte animation. */
    Sint32 x, y;                     /**< Actual birth position, including retained low words. */
    Sint16 player_x, player_y;
    bool allocated, hit, heal_pending;
} SotnSoulOrbInput;
/** @brief One original opaque GT4 and deferred heal request; no HP changes here. */
typedef struct SotnSoulOrbEvents
{
    Sint16 x[4], y[4];
    Uint8 u[4], v[4];
    Uint16 heal;
    bool contact, draw;
} SotnSoulOrbEvents;
/** @brief Advance the original ordinary-stage homing orb once.
 * @param orb Copied actual actor, initially zeroed. @param input Actual owner context.
 * @param random Owned stage Random snapshot, distinct from combat rand.
 * @param out Receives this update's quad/contact/heal intents after success.
 * @return Success or SDL error preserving state, RNG and output on malformed input.
 * Failed row allocation retires before reading tables/RNG. Birth consumes one Random;
 * pickup becomes active on update16. Contact retires even when a prior heal is pending.
 * The owner supplies one real primitive, private tables, animation/contact lifetime,
 * and the next-player-update healing owner. This enables no spell controls itself.
 */
bool SotnSoulOrb_Tick(SotnSoulOrb *orb, SotnSoulOrbInput input, Uint32 *random,
                      SotnSoulOrbEvents *out);
/** @brief Rebase retained screen coordinates without advancing motion or animation.
 * @param orb Actual owner. @param dx Signed-word pixel shift. @param dy Same for Y.
 * @return Success or SDL error without mutation; low words and timing stay unchanged. */
bool SotnSoulOrb_Translate(SotnSoulOrb *orb, Sint32 dx, Sint32 dy);
#endif
