#ifndef SOTN_HUD_RESOURCES_H
#define SOTN_HUD_RESOURCES_H
#include <SDL3/SDL_stdinc.h>
/** @brief Original HUD lifetime walking-heal counter; full HUD initialization resets it. */
typedef struct SotnHudResources
{
    Uint16 healing_mail_timer;
} SotnHudResources;
/** @brief Actual post-player/stage resource, status, equipped-item and gameplay-clock snapshot. */
typedef struct SotnHudResourcesInput
{
    Uint32 game_timer, player_status;
    Uint16 mp, mp_max;
    bool healing_mail, mystic_pendant;
} SotnHudResourcesInput;
/** @brief Proposed actual MP and original kind2/amount1 deferred-heal request. */
typedef struct SotnHudResourcesEvents
{
    Uint16 mp;
    bool heal;
} SotnHudResourcesEvents;
/** @brief Reconstruct original MP/Mystic Pendant and Healing Mail work once per HUD update.
 * @param state Actual HUD owner; counter0..127. @param input Actual current snapshot.
 * @param out Receives MP and an unconditional kind2 heal request on the128th qualifying walk.
 * @return Success or SDL error preserving state/output. MP uses original status20000000 and
 * clock phases0/31, without a health gate or substitute cooldown. Mail uses status04000000
 * and excludes form bits1/2/4, not velocity. Menus/map/draws must not tick; original frozen
 * and release HUD frames do tick, even at a repeated gameplay clock. Commit real resources
 * and pending-heal ownership before accepting the copied counter. Enables no item by itself. */
bool SotnHudResources_Tick(SotnHudResources *state, SotnHudResourcesInput input,
                           SotnHudResourcesEvents *out);
#endif
