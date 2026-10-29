#ifndef SOTN_HELLFIRE_EFFECTS_H
#define SOTN_HELLFIRE_EFFECTS_H
#include <sotn/player_blink.h>
/** @brief Actual original circle28 or flash29, borrowing one GT4/G4 row respectively.
 * Zero except kind and actual source x/y; retains fractions while following high words. */
typedef struct SotnHellfireEffect
{
    SotnPlayerEffectQuad quad;
    Sint32 x, y;
    Uint32 flags;
    Sint16 width, height, timer;
    Uint16 kind;
    Uint8 phase;
    bool removed;
} SotnHellfireEffect;
/** @brief Current actual player/clock and real initial row allocation. */
typedef struct SotnHellfireEffectInput
{
    Sint16 player_x, player_y;
    Uint16 player_priority;
    Uint32 game_timer;
    bool casting, facing_left, allocated;
} SotnHellfireEffectInput;
/** @brief Only original flash allocation failure writes the player's signal word. */
typedef struct SotnHellfireEffectEvents
{
    Uint16 signal;
    bool signal_changed;
} SotnHellfireEffectEvents;
/** @brief Advance one original circle/flash and its retained packet once.
 * @param effect Actual selected child. @param input Real copied owner context.
 * @param out Receives one-update intents. @return Success or SDL error preserving state/output.
 * Circle continues outside Hellfire; flash retires when casting ends. Failed row requests never
 * substitute storage; flash failure reports FFFF. Caller owns real leases, white palette/texture,
 * G4 versus GT4 rendering and actual player finishing. This enables no native spell controls. */
bool SotnHellfireEffect_Tick(SotnHellfireEffect *effect, SotnHellfireEffectInput input,
                             SotnHellfireEffectEvents *out);
/** @brief Selected original factory36/parameter-high-one trailing sprite, owning no pool. */
typedef struct SotnHellfireTrail
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    Uint16 parameter, priority, bank, palette, blend_mode, scale_x, scale_y;
    Uint8 phase, draw_flags;
    bool facing_left, removed;
} SotnHellfireTrail;
/** @brief Actual birth RNG/player priority and preceding private animation duration. */
typedef struct SotnHellfireTrailInput
{
    Uint16 player_priority, random;
    Sint16 remaining;
    bool random_ready;
} SotnHellfireTrailInput;
typedef struct SotnHellfireTrailEvents
{
    Uint32 animation_offset;
    bool initialize, random_used;
} SotnHellfireTrailEvents;
/** @brief Initialize/move the actual selected trailing sprite with original single-RNG birth.
 * @param trail Zero except full parameter/source/actual factory-facing and velocity metadata.
 * @param input Actual owner context. @param out Receives animation/initialization intents.
 * @return Success or SDL error preserving state/output. Allocation belongs to the real factory;
 * only actual birth consumes RNG. Negative pre-animation duration retires the actual child.
 * Native ownership/animation/artwork/rendering remain required before Hellfire availability. */
bool SotnHellfireTrail_Tick(SotnHellfireTrail *trail, SotnHellfireTrailInput input,
                            SotnHellfireTrailEvents *out);
#endif
