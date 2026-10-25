#ifndef SOTN_DOPPLE_BODY_H
#define SOTN_DOPPLE_BODY_H
#include <sotn/animation.h>
#include <sotn/dopple_motion.h>

/** @brief Commands emitted by body controls for the owning encounter. */
typedef enum SotnDoppleRequestKind
{
    SOTN_DOPPLE_FACTORY = 1,
    SOTN_DOPPLE_SOUND,
    SOTN_DOPPLE_CLEAR_ATTACK
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
    SotnAnimation animation;
    Uint16 step, substep, previous_step, previous_substep;
    Uint16 pressed, held, tapped, jump_flags, attack_state, attack_cancel_pose;
    Uint16 timers[16], palette, damage_palette;
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
/** @brief Advance normal-form movement and attack-selection controls by one simulation tick.
 * @param overlay Borrowed BO4 data. @param map Borrowed original room collision.
 * @param buttons Original US PS1 input bits. @param knives Existing original knife count, 0..16.
 * @param body Owned cursor; unchanged on any failure.
 * @return Success or SDL error. Forms/damage/death remain rejected until integrated.
 * @note Child/effect requests must be handled by the encounter before calling this
 * live gameplay. This component alone does not implement enemy attacks. */
bool SotnDoppleBody_Tick(SotnDataView overlay, const SotnCollisionMap *map, Uint16 buttons,
                         Uint8 knives, SotnDoppleBody *body);
#endif
