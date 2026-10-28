#ifndef SOTN_WOLF_TRANSITION_H
#define SOTN_WOLF_TRANSITION_H
#include <sotn/wolf_controller.h>
/** @brief Copied original Wolf entry/return state; actual entities remain with the session. */
typedef struct SotnWolfTransition
{
    SotnWolfControllerFrame player;
    Sint16 floor_y[4], ceiling_y[4], rotation;
    Uint16 bank, palette, texture, saved_bank, saved_sprite, form_pose;
    Uint16 movement[3];
    Uint16 signal;
    Sint16 timer14;
    Uint8 draw_flags;
    bool returning, cramped;
} SotnWolfTransition;
/** @brief Actual, generation-checked child/factory results at the original transition boundary. */
typedef struct SotnWolfTransitionInput
{
    Uint16 center_priority;
    bool mist_child, mist_ready, entry_factory_created;
} SotnWolfTransitionInput;
/** @brief Requests for the actual body, factory and normal-player owners; never readiness signals.
 */
typedef struct SotnWolfTransitionEvents
{
    bool entry_flash, finish_flash, replace_body, clear_weapon, normal_return, stuck_return;
} SotnWolfTransitionEvents;
/** @brief Reconstruct original Wolf entry 0/8 or return 0/1/2, after the resource gate.
 * @param transition Copied current player/sensor words; unchanged on failure.
 * @param input Actual child/factory results, not elapsed-time approximations.
 * @param out Current requests, unchanged on failure. @return Success or SDL error.
 * Entry stays pending until its actual factory succeeds (or the actual Mist child exists).
 * The session must perform requested slot replacements, finishing factory and normal/stuck
 * return helpers in original order. This component allocates no entities, changes no resource
 * ownership and enables no Wolf controls. Timer14 is the original player timer, not a sensor. */
bool SotnWolfTransition_Tick(SotnWolfTransition *transition, SotnWolfTransitionInput input,
                             SotnWolfTransitionEvents *out);
#endif
