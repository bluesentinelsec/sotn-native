#ifndef SOTN_DOPPLE_BODY_H
#define SOTN_DOPPLE_BODY_H
#include <sotn/animation.h>
#include <sotn/dopple_motion.h>

/** @brief Commands emitted by body controls for the owning encounter. */
typedef enum SotnDoppleRequestKind
{
    SOTN_DOPPLE_FACTORY = 1,
    SOTN_DOPPLE_SOUND,
    SOTN_DOPPLE_CLEAR_ATTACK,
    SOTN_DOPPLE_PALETTE_BEGIN,
    SOTN_DOPPLE_PALETTE_CLEAR,
    SOTN_DOPPLE_SHAKE
} SotnDoppleRequestKind;

/** @brief Original boss runtime request; the owning encounter resolves bounded factories/audio. */
typedef struct SotnDoppleRequest
{
    Uint16 kind, id, parameter;
    Sint32 x, y; /**< Original 16.16 origin at request time, before body movement. */
    bool facing_left;
} SotnDoppleRequest;
/** @brief Original body cursor, owned by one encounter. No borrowed storage or global state.
 * Requests are reset each tick. They are commands for the encounter, not proof that
 * the requested child/effect is implemented. */
typedef struct SotnDoppleBody
{
    SotnDoppleMotion motion;
    Uint32 tick; /**< Owner supplies the global simulation tick; successful ticks advance it. */
    SotnAnimation animation;
    Uint16 step, substep, previous_step, previous_substep;
    Uint16 pressed, held, tapped, jump_flags, attack_state, attack_cancel_pose;
    Uint16 timers[16], palette, damage_palette;
    Uint16 animation_set, form_pose, form_phase, form_counter, wing_timer, mist_step;
    Uint16 simulated_timer, simulated_buttons;
    Sint16 rotation, bat_offset;
    bool rotated, mist_active;
    bool facing_left, turned, busy, afterimage_disabled, afterimage_reset;
    SotnDoppleRequest requests[16];
    Uint8 request_count;
} SotnDoppleBody;
/** @brief Initialize original falling body and sensors from private BO4.
 * @param overlay Borrowed verified overlay. @param x Signed 16.16 room-local position.
 * @param y Signed 16.16 room-local position. @param facing_left Initial direction.
 * @param out Owned body, unchanged on failure. @return Success or SDL error. */
bool SotnDoppleBody_Init(SotnDataView overlay, Sint32 x, Sint32 y, bool facing_left,
                         SotnDoppleBody *out);
/** @brief Advance normal and selected bat/mist controls by one simulation tick.
 * @param overlay Borrowed BO4 data. @param map Borrowed original room collision.
 * @param buttons Original US PS1 input bits. @param knives Existing original knife count, 0..16.
 * @param body Owned cursor; unchanged on any failure.
 * @return Success or SDL error. Damage/death remain rejected until integrated.
 * @note Child/effect requests and form-phase/mist signals must be handled by the
 * owning encounter before using this for live gameplay. This component alone does not implement
 * enemy attacks. */
bool SotnDoppleBody_Tick(SotnDataView overlay, const SotnCollisionMap *map, Uint16 buttons,
                         Uint8 knives, SotnDoppleBody *body);
#endif
