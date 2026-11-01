#ifndef SOTN_FAIRY_EFFECTS_H
#define SOTN_FAIRY_EFFECTS_H
#include <sotn/player_blink.h>
#include <sotn/player_particles.h>

enum
{
    SOTN_FAIRY_ITEM_PARTICLES = 128,
    SOTN_FAIRY_ITEM_ROWS = 129,
    SOTN_FAIRY_APPLE_ROWS = 10
};
/** @brief Original DF item cursor and per-row scratch, not an alternate primitive pool.
 * Caller reserves actual count+one rows before birth, retains the sentinel and retires all rows
 * with the real entity. Coordinates are original screen-space16.16; drawing never ticks. */
typedef struct SotnFairyItemEffect
{
    SotnPlayerParticle tiles[SOTN_FAIRY_ITEM_ROWS];
    Sint32 x, y;
    Uint32 flags;
    Uint16 parameter, phase, count, priority;
    Sint16 active, activation_timer, alive;
    bool removed;
} SotnFairyItemEffect;
/** @brief Actual private profile and original entity/player snapshots at this callback. */
typedef struct SotnFairyItemEffectInput
{
    SotnDataView overlay;
    SotnPlayerFactorySource source;
    Uint16 player_priority;
    bool
        allocated; /**< Actual count+one stage-row reservation result, never predicted readiness. */
} SotnFairyItemEffectInput;
/** @brief Original DD Life Apple light with its ten untextured G4 rows. */
typedef struct SotnFairyAppleEffect
{
    SotnPlayerEffectQuad quads[SOTN_FAIRY_APPLE_ROWS];
    Sint32 x, y;
    Uint32 flags;
    Sint16 timer, width, height, opacity, fan_opacity;
    Uint16 phase;
    bool removed;
} SotnFairyAppleEffect;
/** @brief Actual original entity snapshot, private SDK tables and real ten-row result. */
typedef struct SotnFairyAppleEffectInput
{
    SotnDataView executable;
    SotnPlayerFactorySource source;
    bool allocated;
} SotnFairyAppleEffectInput;
/** @brief Read the private original item profile's count+one row requirement.
 * @param overlay Borrowed immutable TT_002. @param parameter Original DF parameter0..10.
 * @param rows Unchanged on failure. @return Success or SDL error; allocates nothing. */
bool SotnFairyItemEffect_Rows(SotnDataView overlay, Uint16 parameter, Uint16 *rows);
/** @brief Advance original DF once, including birth activation and motion fallthrough.
 * @param effect Copied actual cursor; set parameter before first callback.
 * @param input Real source/profile/allocation snapshots. @param random Copied original libc RNG.
 * @return Success or SDL error preserving cursor/RNG. Failed allocation retires before RNG.
 * Calls neither healing nor inventory/resource owners; no native familiar/effect is enabled. */
bool SotnFairyItemEffect_Tick(SotnFairyItemEffect *effect, SotnFairyItemEffectInput input,
                              Uint32 *random);
/** @brief Advance original DD once, preserving birth-only geometry and signed-word phases.
 * @param effect Copied actual cursor. @param input Real source/SDK/allocation snapshots.
 * @return Success or SDL error without mutation. Optional failure retires before SDK reads.
 * This visual restores no resources and supplies no player17 or scene handshake. */
bool SotnFairyAppleEffect_Tick(SotnFairyAppleEffect *effect, SotnFairyAppleEffectInput input);
/** @brief Apply original camera/room packet flags without advancing timers or RNG.
 * @param effect Actual initialized item cursor. @param x Signed whole-pixel translation.
 * @param y Signed whole-pixel translation. @param room Select original room packet flag100.
 * @return Success or SDL error preserving malformed cursors. Raster positions translate;
 * fixed-point scratch high words rebind from those positions on the next real callback. */
bool SotnFairyItemEffect_Rebase(SotnFairyItemEffect *effect, Sint16 x, Sint16 y, bool room);
/** @brief Apply original entity/packet camera or room flags to the real light cursor.
 * @param effect Actual light. @param x Signed pixel shift. @param y Signed pixel shift.
 * @param room Select room versus camera flags. @return Success or SDL error.
 * Its ordinary435 G4s omit camera flag2; never translate the birth fan a second time. */
bool SotnFairyAppleEffect_Rebase(SotnFairyAppleEffect *effect, Sint16 x, Sint16 y, bool room);
#endif
