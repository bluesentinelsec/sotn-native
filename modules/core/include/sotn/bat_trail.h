#ifndef SOTN_BAT_TRAIL_H
#define SOTN_BAT_TRAIL_H

#include <sotn/player_blink.h>

/** @brief One actual Wing Smash trail child; initialize its factory position before first tick.
 * Position remains fixed in room-local 16.16 units; the owner rebases it on room changes.
 * Current frame follows Alucard, while facing/rotation/priority are captured at birth. */
typedef struct SotnBatTrail
{
    Sint32 x, y;
    Sint16 rotation;
    Uint16 frame, bank, priority, scale;
    Uint8 opacity;
    bool initialized, removed, facing_left, rotated;
} SotnBatTrail;

/** @brief Actual current owner snapshot for a selected trail child. */
typedef struct SotnBatTrailInput
{
    Uint16 frame, bank, priority;
    Sint16 rotation;
    bool wing_smashing, facing_left, rotated;
} SotnBatTrailInput;

/** @brief Advance original trail initialization, shrink/fade or owner-state removal once.
 * @param trail Owned actual child; never a factory request or invented allocation result.
 * @param input Current copied player state.
 * @return Success or SDL error, unchanged on invalid state. A removed owner cannot tick again.
 * The caller allocates original slots and preserves entity update/render ordering. */
bool SotnBatTrail_Tick(SotnBatTrail *trail, SotnBatTrailInput input);

/** @brief Build the original scaled, rotated, additively shaded Bat trail packet.
 * @param trail Borrowed initialized live child. @param image Actual current private frame geometry.
 * @param executable Borrowed original SDK tables. @param x Signed-word screen origin.
 * @param y Signed-word screen origin. @param out Receives copied packet, unchanged on failure.
 * @return Success or SDL error. Samples the actual player display slot; uploads no texture.
 * This is geometry/attributes, not PS1 raster or shared allocation parity. */
bool SotnBatTrail_Quad(const SotnBatTrail *trail, SotnPlayerImage image, SotnDataView executable,
                       Sint16 x, Sint16 y, SotnPlayerEffectQuad *out);

#endif
