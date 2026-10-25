#ifndef SOTN_DOPPLE_AI_H
#define SOTN_DOPPLE_AI_H
#include <sotn/game_data.h>

/** @brief Owned original Doppleganger decision cursor and simulated PS1 buttons. */
typedef struct SotnDoppleAI
{
    Uint16 step, buttons;
    Sint32 timer, attack_timer;
} SotnDoppleAI;
/** @brief Borrowed simulation observations for one original decision tick.
 * Status/contact words and button outputs retain their original PS1 bit values;
 * coordinates are whole pixels, with world_x including camera scroll. */
typedef struct SotnDoppleObservation
{
    Sint16 boss_x, boss_y, player_x, player_y;
    Sint32 world_x, velocity_y;
    Uint32 boss_status, player_status, contact, tick;
    Uint16 step, substep;
    bool facing_left, player_weapon_starting, player_subweapon_starting, player_casting;
} SotnDoppleObservation;
/** @brief Reset decision state to the original opening backdash request.
 * @param out Writable cursor. @return Success or SDL error for NULL. */
bool SotnDoppleAI_Init(SotnDoppleAI *out);
/** @brief Evaluate the original US boss's next simulated control input.
 * @param observation Borrowed current observations; no allocation or rendering.
 * @param libc_random Owned original libc RNG stream, advanced only for original decisions.
 * @param ai Owned cursor. Cursor and RNG remain unchanged on failure.
 * @return Success or SDL error for invalid state/observations.
 * @note This produces input only; movement, attacks and form transitions belong
 * to the encounter runtime. It preserves the original crouch-attack lock behavior. */
bool SotnDoppleAI_Tick(const SotnDoppleObservation *observation, Uint32 *libc_random,
                       SotnDoppleAI *ai);
#endif
