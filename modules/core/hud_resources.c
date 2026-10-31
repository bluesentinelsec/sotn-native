// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/5F60C.c:DrawHudSubweapon MP and Healing Mail work, pinned
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private tables embedded.
#include <SDL3/SDL_error.h>
#include <sotn/hud_resources.h>
bool SotnHudResources_Tick(SotnHudResources *state, SotnHudResourcesInput in,
                           SotnHudResourcesEvents *out)
{
    if (!state || !out || state->healing_mail_timer >= 128 || in.mp > in.mp_max || in.mp_max > 9999)
        return SDL_SetError(
            "HUD resources require actual bounded MP and a0..127 walking-heal counter");
    SotnHudResources next = *state;
    SotnHudResourcesEvents e = {.mp = in.mp};
    if (!(in.player_status & 0x20000000U) &&
        (!(in.game_timer & 63U) || (in.mystic_pendant && (in.game_timer & 63U) == 31U)))
        e.mp = (Uint16)SDL_min(in.mp_max, (Uint32)in.mp + 1U);
    if (in.healing_mail && (in.player_status & 0x04000000U) && !(in.player_status & 7U))
    {
        ++next.healing_mail_timer;
        if (next.healing_mail_timer == 128)
        {
            e.heal = true;
            next.healing_mail_timer = 0;
        }
    }
    else
        next.healing_mail_timer = 0;
    *state = next;
    *out = e;
    return true;
}
