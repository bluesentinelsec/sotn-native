#ifndef SOTN_WATER_DEATH_H
#define SOTN_WATER_DEATH_H
#include <SDL3/SDL_stdinc.h>

/** @brief Selected original state13 controller, without player, animation or capture ownership. */
typedef struct SotnWaterDeath
{
    Sint32 velocity_x, velocity_y;
    Uint16 step, phase, draw_flags, rotation, blend;
    Uint8 red, blue;
    bool color_blend;
} SotnWaterDeath;
/** @brief Actual preceding animation, global fade and cramped-space snapshots. */
typedef struct SotnWaterDeathInput
{
    Sint16 remaining;
    Uint16 fade;
    bool tight_space;
} SotnWaterDeathInput;
/** @brief Calls required in their original order, not completed native operations. */
typedef enum SotnWaterDeathActionKind
{
    SOTN_WATER_DEATH_SOUND,
    SOTN_WATER_DEATH_RESET_NORMAL,
    SOTN_WATER_DEATH_ANIMATION,
    SOTN_WATER_DEATH_GLOBAL_EFFECT,
    SOTN_WATER_DEATH_FACTORY,
    SOTN_WATER_DEATH_CAPTURE
} SotnWaterDeathActionKind;
typedef struct SotnWaterDeathAction
{
    SotnWaterDeathActionKind kind;
    Uint32 value;
} SotnWaterDeathAction;
/** @brief One-update requests. Capture must finish before the common death controller can use it.
 */
typedef struct SotnWaterDeathEvents
{
    SotnWaterDeathAction actions[6];
    Uint8 count;
    bool begin_disintegration;
} SotnWaterDeathEvents;
/** @brief Reconstruct one original PlayerStepKillWater update, without moving or animating.
 * @param state Actual state13/phase0 or1. Initialize other fields from the player; birth resets
 * velocity, colors, blend and rotation through the original normal helper boundary.
 * @param input Actual owner snapshots. Fade0xFFF preempts any initialized phase.
 * @param out Ordered original helper/sound/factory/capture commands, not successful allocations.
 * @return Success or SDL error preserving state/output. The owner must invoke real normal reset,
 * animationC1, global effect1, Blink59, factory49/parameter6, framebuffer capture and common
 * state16/17 controllers before exposing live water death. Birth rotation flags clear through
 * the normal helper and return next update. Phase1 velocity wraps before the strict positive
 * reset. Motion/collision/animation follow this function in their original engine boundaries.
 * This component grants no player control availability and never substitutes physical death.
 */
bool SotnWaterDeath_Tick(SotnWaterDeath *state, SotnWaterDeathInput input,
                         SotnWaterDeathEvents *out);
#endif
