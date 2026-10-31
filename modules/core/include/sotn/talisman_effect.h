#ifndef SOTN_TALISMAN_EFFECT_H
#define SOTN_TALISMAN_EFFECT_H
#include <sotn/player_blink.h>

/** @brief Original entity46, retaining its own motion and eight real Blink26 packets. */
typedef struct SotnTalismanEffect
{
    SotnPlayerBlink blink;
    Sint32 x, y, velocity_x, velocity_y;
    Uint8 red, gray;
    bool facing_left;
} SotnTalismanEffect;

/** @brief Advance original Talisman child46 using actual private frame9F geometry.
 * @param effect Owned motion/packets; initialize x/y/facing from the real factory child.
 * @param shared Actual global Blink angles and transformation locks.
 * @param input Player snapshot with frame9F geometry; birth retains child fractions.
 * @param parameter Original factory115 child parameter0/1. @param clock Gameplay clock.
 * @param out Copied afterimage-reset request. @return Success; all outputs hold on failure.
 * Caller owns eight-row allocation; birth exhaustion retires this optional child.
 */
bool SotnTalismanEffect_Tick(SotnTalismanEffect *effect, SotnPlayerBlinkShared *shared,
                             SotnPlayerBlinkInput input, Uint16 parameter, Uint32 clock,
                             SotnPlayerBlinkEvents *out);
/** @brief Translate retained screen-space motion/packets without advancing a clock.
 * @param effect Owned child. @param x Signed horizontal camera/room translation in pixels.
 * @param y Signed vertical translation in pixels. @return Success or SDL error for null owner.
 * Full motion and packet coordinates retain original signed wrapping. */
bool SotnTalismanEffect_Rebase(SotnTalismanEffect *effect, Sint16 x, Sint16 y);
#endif
