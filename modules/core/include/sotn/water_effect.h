#ifndef SOTN_WATER_EFFECT_H
#define SOTN_WATER_EFFECT_H
#include <sotn/player_blink.h>
#include <sotn/water_actor.h>

/** @brief One actual splash/surfacing/side child and its retained original GT4 packets. */
typedef struct SotnWaterEffect
{
    SotnPlayerEffectQuad quads[4];
    Sint32 x, y, velocity_x, velocity_y, gravity, top;
    Sint16 world_y, current;
    Uint16 parameter, water_row;
    SotnWaterChildKind kind;
    Uint8 phase, count, age;
    bool removed;
} SotnWaterEffect;
/** @brief Private ordinary water block, real row lease, live scroll and actor priority. */
typedef struct SotnWaterEffectInput
{
    SotnDataView tables, volumes, executable;
    Sint16 scroll_x, scroll_y;
    Uint16 priority, allocated_rows;
} SotnWaterEffectInput;
/** @brief Original sound and optional water-drop request; neither is completed playback/creation.
 */
typedef struct SotnWaterEffectEvents
{
    Sint32 drop_x, drop_y, drop_velocity_y;
    Uint8 sound_index, volume;
    Sint8 pan;
    bool sound, positional, drop;
} SotnWaterEffectEvents;
/** @brief Resolve the original row count without allocating anything or signalling readiness.
 * @param kind Actual controller-selected role. @param parameter Full original word.
 * @param out Receives2/4 main rows,2 surface rows or1 side row, unchanged on failure.
 * @return Success or SDL error for unsupported roles/private-table indices. */
bool SotnWaterEffect_Rows(SotnWaterChildKind kind, Uint16 parameter, Uint16 *out);
/** @brief Advance an actual ordinary water child once, preserving original birth/motion/fades.
 * @param effect Copied owner initialized with actual source/kind/parameter and surface metadata.
 * @param input Actual lease/context; tables borrow the private188-byte US water block.
 * @param random Actual shared libc-rand state, not stage Random; only surface birth consumes it.
 * @param out Receives this update's sound/drop requests, unchanged on failure.
 * @return Success or SDL error preserving state/RNG/output on invalid data/context. Initial
 * allocation failure retires before table reads, math, sounds or RNG. A requested drop still
 * requires its actual auxiliary slot, behavior,33 contiguous rows and rendering owner. The owner
 * initializes original stage flags and commits real rows/requests before enabling live feedback.
 * Undefined original zero-divisor slope paths fail explicitly. Saves and controls are untouched.
 */
bool SotnWaterEffect_Tick(SotnWaterEffect *effect, SotnWaterEffectInput input, Uint32 *random,
                          SotnWaterEffectEvents *out);
/** @brief Apply original camera primitive/body shifts without advancing simulation or world cache.
 * @param effect Actual child. @param dx Signed-word screen shift. @param dy Same for Y.
 * @return Success or SDL error without mutation. Cached world_y stays fixed; the next update
 * receives actual new scroll words. Repeated draws must never call this as an update. */
bool SotnWaterEffect_Translate(SotnWaterEffect *effect, Sint32 dx, Sint32 dy);
#endif
