#ifndef SOTN_WOLF_CONTROLLER_H
#define SOTN_WOLF_CONTROLLER_H
#include <sotn/player_factory.h>
#include <sotn/wolf_motion.h>

/** @brief Copied active Wolf controller state, before animation and physical integration.
 * Shared player timers must already be decremented. Actual body/root and collider
 * snapshots belong to the native owner; this component allocates no entities. */
typedef struct SotnWolfControllerFrame
{
    SotnWolfJumpFrame motion;
    Sint32 x, y;
    Sint16 root_y;
    Uint16 tapped, mp, coyote_ticks, drop_ticks, priority;
    Uint32 game_timer, raw_contacts, previous_contacts, entity_flags;
    Uint32 idle_timer, turn_timer, run_window, swim_lock;
    Uint32 floor_effects[4], wall_effects[14];
    Sint32 water_level, fall_water_level, joint_distance;
    bool direction_locked, skill_enabled, power_enabled, holy_symbol_enabled;
    bool quarter_circle_complete;
    SotnWolfCadence cadence;
} SotnWolfControllerFrame;
/** @brief Original one-shot request kinds, retaining relative order in one list. */
typedef enum SotnWolfRequestKind
{
    SOTN_WOLF_FACTORY = 1,
    SOTN_WOLF_SOUND,
    SOTN_WOLF_SHAKE
} SotnWolfRequestKind;
/** @brief One original factory, sound or shake request; success is never manufactured. */
typedef struct SotnWolfRequest
{
    SotnPlayerFactorySource source;
    Uint32 parameter;
    Uint8 kind;
} SotnWolfRequest;
/** @brief Current requests for the actual player/body/factory/progress owners. */
typedef struct SotnWolfControllerEvents
{
    SotnWolfRequest requests[8];
    Uint16 mp_spent, input_hold;
    Uint8 count;
    bool attack_requested, learn_charge, turned, return_requested;
} SotnWolfControllerEvents;
/** @brief Advance selected active Wolf locomotion substates after the form resource gate.
 * @param frame Owned copied substates 1..7 or 9; atomic on malformed state/data.
 * @param dra Borrowed private spell definitions, needed only for an eligible charge cast.
 * @param out Receives ordered requests, unchanged on failure.
 * @return Success or SDL error. Preserves original US sprint quirks, contacts, word timers,
 * landing/swim transitions and exact-cost charge MP. Entry/return, actual body/attack/presentation
 * ownership and the form resource/priority gate remain with the native owner; no live controls
 * are enabled by this service. It neither advances animation nor integrates position. */
bool SotnWolfController_Tick(SotnWolfControllerFrame *frame, SotnDataView dra,
                             SotnWolfControllerEvents *out);
#endif
