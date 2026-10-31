#ifndef SOTN_PLAYER_BLINK_H
#define SOTN_PLAYER_BLINK_H

#include <sotn/game_math.h>
#include <sotn/wolf_motion.h>

/** @brief Actual decoded player image geometry; its pixels stay with the texture owner. */
typedef struct SotnPlayerImage
{
    Uint16 width, height;
    Sint16 pivot_x, pivot_y;
} SotnPlayerImage;

/** @brief Original player-effect textured quad, in signed-word screen coordinates. */
typedef struct SotnPlayerEffectQuad
{
    Sint16 x[4], y[4];
    Uint8 u[4], v[4], red[4], green[4], blue[4];
    Uint16 page, palette, priority, draw_mode;
} SotnPlayerEffectQuad;

/** @brief Owned original global BlinkWhite phases shared by selected live children. */
typedef struct SotnPlayerBlinkShared
{
    Uint32 angles[8];
    bool bat_lock, wolf_lock;
} SotnPlayerBlinkShared;

/** @brief One real selected BlinkWhite controller and its eight owned primitives.
 * Initialize parameter to 0x20/0x21 (Bat), 0x23/0x24 (Wolf), 5 (Fire) or 0x5C (Wing Smash).
 * Retain until removed; special flashes allocate their own real children.
 * Parameters 0x16/0x17 restore original poison/curse glows after casting.
 * Parameters 0x4E/0x59 support original water-pain/death flashes; requests remain with the owner.
 * Parameters 0x44/0x48 support original blood-healing flashes; 0x67 is the spirit fan.
 * Parameter 0x58 is the scripted reaction fan; its 0x7007 hold tests original Hit state10.
 * Parameters 0x1B/0x25 include original automatic finishing flashes, not new children. */
typedef struct SotnPlayerBlink
{
    SotnPlayerEffectQuad quads[8];
    Uint16 parameter, phase, angle, speed, wave, hold;
    Sint16 brightness, amplitude;
    bool removed;
} SotnPlayerBlink;

/** @brief Actual private tables, player image/transform and preceding owner handshake. */
typedef struct SotnPlayerBlinkInput
{
    SotnDataView dra, executable;
    SotnPlayerImage image;
    SotnWolfCaptureBounds wolf_bounds;
    Sint16 x, y, rotation;
    Uint16 priority, player_step, player_substep;
    Uint16 poison_ticks, curse_ticks; /**< Actual ailment timer0/1, held through scene freeze. */
    Uint16 water_waist, player_animation; /**< Actual preceding depth and current animation ID. */
    Uint32 status;
    Uint8 signal;
    bool facing_left, rotated, image_present;
    bool wolf_form; /**< Actual bank15. Entry23 before signal2 uses the saved ordinary/Bat image. */
    bool allocation_failed; /**< Real eight-row exhaustion at phase zero; the US child retries. */
    bool holy_symbol;       /**< Actual found-and-active relic snapshot, not a default grant. */
} SotnPlayerBlinkInput;

/** @brief Original side effects for the player owner; zero signal means no new handshake. */
typedef struct SotnPlayerBlinkEvents
{
    Uint8 signal;
    bool reset_afterimages;
} SotnPlayerBlinkEvents;

/** @brief Advance selected original Bat/Wolf transformation and Wing/Fire flashes once.
 * @param effect Owned actual child and primitives. @param shared Owned shared phase/lock state.
 * @param input Borrowed verified private tables and actual decoded player snapshot.
 * @param out Copied original handshake/afterimage requests.
 * @return Success or SDL error, preserving effect/shared/output on malformed input.
 * No readiness timer is supplied: signals two/three come from the real effect's geometry
 * phases. US phase-zero allocation failures retain the pending child without packets/signals.
 * The caller owns allocation, duplicate suppression, texture upload and update order. */
bool SotnPlayerBlink_Tick(SotnPlayerBlink *effect, SotnPlayerBlinkShared *shared,
                          SotnPlayerBlinkInput input, SotnPlayerBlinkEvents *out);

/** @brief Apply original rotation-only player quad geometry using private SDK math tables.
 * @param executable Borrowed original SDK tables. @param x Signed-word pivot X.
 * @param y Signed-word pivot Y. @param rotation Original signed rotation.
 * @param facing_left Reverses rotation, as in the PS1 renderer.
 * @param quad Owned quad, unchanged on failure. @return Success or SDL error.
 * This does not apply scale, opacity, perspective or framebuffer rasterization. */
bool SotnPlayerEffect_Rotate(SotnDataView executable, Sint16 x, Sint16 y, Sint16 rotation,
                             bool facing_left, SotnPlayerEffectQuad *quad);

#endif
