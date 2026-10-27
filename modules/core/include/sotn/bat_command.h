#ifndef SOTN_BAT_COMMAND_H
#define SOTN_BAT_COMMAND_H

#include <sotn/animation.h>

/** @brief Original transient command globals; zero-initialize and retain between Bat ticks.
 * Values wrap as original 32-bit words; they are not ordinary save data. */
typedef struct SotnBatCommand
{
    Uint32 counter, timer, duration;
} SotnBatCommand;

/** @brief Copied ordinary Bat controller context after the form's resource gate.
 * Animation/motion are controller state, not visual completion or allocated children. */
typedef struct SotnBatCommandFrame
{
    SotnAnimation animation;
    Sint32 velocity_x, velocity_y;
    Sint16 rotation;
    Uint16 substep, buttons, tapped, contacts, mp;
    Uint32 game_timer;
    bool facing_left, fire_enabled, echo_enabled;
} SotnBatCommandFrame;

/** @brief Original factory/control requests; the session must fulfill actual child ownership.
 * Requests do not assert allocation success, collision hits or effect completion. */
typedef struct SotnBatCommandEvents
{
    Uint16 mp_spent;
    bool learn_wing_smash, wing_attack, wing_flash, echo, fire_flash, fireball;
    bool trail, floor_dust, ceiling_dust, return_requested, wall_impact, handled;
} SotnBatCommandEvents;

/** @brief Advance original Wing Smash command and selected Wing/Fire motion once.
 * @param command Owned transient globals. @param dra Borrowed private spell definitions.
 * @param frame Copied in/out original controller context; substeps 0..5.
 * @param out Receives original side-effect requests, zeroed on successful no-request ticks.
 * @return Success or SDL error; all outputs remain unchanged on malformed state/private data.
 * Ordinary flight substeps remain with the player controller. Cast eligibility uses >= MP,
 * unlike transformation entry. This service allocates no entities and learns no spell itself.
 */
bool SotnBatCommand_Tick(SotnBatCommand *command, SotnDataView dra, SotnBatCommandFrame *frame,
                         SotnBatCommandEvents *out);

#endif
