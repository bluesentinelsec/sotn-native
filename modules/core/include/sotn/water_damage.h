#ifndef SOTN_WATER_DAMAGE_H
#define SOTN_WATER_DAMAGE_H
#include <SDL3/SDL_stdinc.h>

/** @brief Actual DRA-lifetime water pain voice counter; no ordinary save representation. */
typedef struct SotnWaterDamage
{
    Uint8 pain_timer;
} SotnWaterDamage;
/** @brief Original pre-dispatch water, queued-contact, timer, resource and equipment snapshot. */
typedef struct SotnWaterDamageInput
{
    Uint32 game_timer, status;
    Uint16 hp, hp_max, waist, hit_params;
    bool holy_symbol, invincible, grabbed, fury_plate;
} SotnWaterDamageInput;
/** @brief Proposed original HP/contact/response and commands, not committed gameplay/effects. */
typedef struct SotnWaterDamageEvents
{
    Uint16 hp, hit_params, damage, defense_buff_min;
    Uint8 response, blink;
    bool scheduled, delegated, voice, scrape;
} SotnWaterDamageEvents;
/** @brief Reconstruct selected ordinary water contact and response once, after player timers.
 * @param state Copied original voice counter; zero on original DRA initialization.
 * @param input Actual alive-player snapshot. Invincible combines the original two damage timers;
 * grabbed includes original start/held/damage gates. Ownership flags must reflect actual equipped
 * armor and active relics, never development defaults.
 * @param out Receives proposed HP/contact, response9 or lethal6, optional Blink4E and sound/buff
 * intents. Stone and other queued contacts delegate to their actual damage owner without HP work.
 * @return Success or SDL error preserving state/output on invalid bounds. Injection precedes
 * protection/grab gates and replaces high-only pending words. Ordinary damage keeps priority.
 * Modulo10 HP loss uses the actual wrapping game clock, not a new cooldown. Continuous exposure
 * refreshes the32-update voice counter. No actor/factory/row, sound, buff, player HP or death state
 * is committed here. Before live use, connect actual contact/resource/effect owners and original
 * water death state13; ordinary physical death is not a substitute. Menus/freeze must not call it.
 */
bool SotnWaterDamage_Tick(SotnWaterDamage *state, SotnWaterDamageInput input,
                          SotnWaterDamageEvents *out);
#endif
