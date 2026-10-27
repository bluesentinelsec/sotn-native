#ifndef SOTN_MIST_EFFECT_H
#define SOTN_MIST_EFFECT_H

#include <sotn/game_math.h>

/** @brief Copied original BlinkWhite snapshot wedge; vertex 2 is its duplicated center. */
typedef struct SotnMistWedge
{
    Sint16 x[3], y[3];
    Uint8 u[3], v[3];
} SotnMistWedge;

/** @brief Owned original fixed-point ring point, retained across effect activations. */
typedef struct SotnMistPoint
{
    Sint32 x, y;
    Sint16 angle, phase, size, target_x, target_y;
} SotnMistPoint;

/** @brief Original textured quad description; each grey value applies to R, G and B. */
typedef struct SotnMistQuad
{
    Sint16 x[4], y[4];
    Uint8 u[4], v[4], grey[4];
    Uint16 page, palette, priority;
    bool hidden;
} SotnMistQuad;

/** @brief Owned Mist visual cursor. Zero-initialize once; retain point fractions on reuse.
 * Coordinates are original screen-space signed 16.16 values. The owner must translate
 * points and the entity anchor when the original camera-lock service moves them.
 * Phase 5 is real restoration readiness, not effect destruction. */
typedef struct SotnMistEffect
{
    SotnMistPoint points[16];
    SotnMistWedge capture[8];
    SotnMistQuad quads[80];
    Sint32 x, y;
    Sint16 color_phase[4], color_base[4];
    Sint16 timer, offset_x, offset_y, target_x, target_y;
    Uint8 phase, opacity;
    bool removed;
} SotnMistEffect;

/** @brief Borrowed private inputs and copied original player snapshot for one simulation tick. */
typedef struct SotnMistEffectInput
{
    SotnDataView dra, executable;
    const SotnMistWedge *capture; /**< Eight actual snapshot wedges, mandatory in phases 0/3. */
    Sint16 x, y;
    Sint32 vx, vy;
    Uint32 game_timer;
    Uint16 priority, random[4]; /**< Original four rand results, consumed only by phase 0. */
    bool facing_left, alternate_sheet;
} SotnMistEffectInput;

/** @brief Requests for the owning palette/RNG services, not simulated completion signals. */
typedef struct SotnMistEffectEvents
{
    Uint8 random_count;
    bool cancel_palette, restore_palette;
} SotnMistEffectEvents;

/** @brief Advance the original 16-point/80-quad Mist effect and restoration lifecycle.
 * @param effect Owned cursor. Initialization inherits its entity anchor fractions and
 * retained ring fractions; the actual capture supplies the whole-pixel outline.
 * @param input Borrowed verified runtime tables and actual captured sprite geometry.
 * @param out Copied side-effect requests, unchanged on error.
 * @return Success or SDL error with effect/output unchanged for malformed data/state.
 * An owner requests restoration by changing a live phase 1/2 to 3, then supplies a fresh
 * actual capture. Only the resulting phase 5 permits the player to complete restoration.
 * This component owns no texture memory, shared primitive allocator, player or bindings. */
bool SotnMistEffect_Tick(SotnMistEffect *effect, SotnMistEffectInput input,
                         SotnMistEffectEvents *out);

/** @brief Apply an original whole-pixel camera/room translation to retained Mist state.
 * @param effect Owned initialized cursor, including retained points when no entity is alive.
 * @param x Signed whole-pixel horizontal delta, within an original position word.
 * @param y Signed whole-pixel vertical delta. @param live The actual child currently exists.
 * @return Success or SDL error without mutation. Always moves global ring points; moves
 * the entity anchor and primitive geometry only while live. Cached capture geometry and
 * UVs remain unchanged, matching the original separate snapshot lifetime. */
bool SotnMistEffect_Translate(SotnMistEffect *effect, Sint32 x, Sint32 y, bool live);

#endif
