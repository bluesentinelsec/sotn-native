#ifndef SOTN_PLAYER_OUTLINE_H
#define SOTN_PLAYER_OUTLINE_H

#include <sotn/player_blink.h>

/** @brief One original entity31 outline and its copied packet; no primitive storage is owned here.
 * Zero-initialize with the actual full child parameter; profiles occupy its high seven bits. */
typedef struct SotnPlayerOutline
{
    SotnPlayerEffectQuad quad;
    Sint16 x, y, scale_x, scale_y, pivot_x, pivot_y, rotation, brightness;
    Uint16 parameter, timer;
    Uint8 phase, draw_flags;
    bool removed;
} SotnPlayerOutline;

/** @brief Actual current player geometry and original child-allocation result. */
typedef struct SotnPlayerOutlineInput
{
    SotnDataView dra, executable;
    SotnPlayerImage image;
    Sint16 x, y, scale_x, scale_y, pivot_x, pivot_y, rotation;
    Uint16 bank, frame, palette, step, priority;
    Uint32 status;
    bool facing_left, allocation_failed;
} SotnPlayerOutlineInput;

/** @brief Original post-packet afterimage-reset request, never resource readiness. */
typedef struct SotnPlayerOutlineEvents
{
    bool reset_afterimages;
} SotnPlayerOutlineEvents;

/** @brief Reconstruct original outline profiles0..23, private colors and SDK geometry.
 * @param effect Copied actual child's state; immutable parameter identifies its private profile.
 * @param input Actual normal-image geometry; borrowed artwork stays with the texture owner.
 * @param out Receives this update's original reset request.
 * @return Success or SDL error preserving effect/output on malformed state/data/geometry.
 * The owner must allocate/retain one actual shared primitive, supply its real exhaustion result,
 * upload the live normal body artwork, render at the packet priority, and retire the lease when
 * removed. This component allocates no rows, enables no spells and invents no readiness. */
bool SotnPlayerOutline_Tick(SotnPlayerOutline *effect, SotnPlayerOutlineInput input,
                            SotnPlayerOutlineEvents *out);
/** @brief Rebase retained screen packets without advancing phases, scale, colors or timers.
 * @param effect Owned child's copied state. @param dx Whole-pixel signed-word translation.
 * @param dy Whole-pixel signed-word translation. @return Success or SDL error without mutation.
 * Original geometry wraps at signed-word boundaries. Fresh/removed children have no packets. */
bool SotnPlayerOutline_Translate(SotnPlayerOutline *effect, Sint32 dx, Sint32 dy);

#endif
