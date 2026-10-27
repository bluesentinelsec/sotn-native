// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/5D5BC.c:HandleTransformationMP at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No runtime data or assets are embedded.
#include <SDL3/SDL.h>
#include <sotn/transformation.h>

bool SotnTransformation_MP(const SotnWorldState *world, SotnTransformationForm form, Uint16 mp,
                           Uint32 game_timer, bool reduce, SotnTransformationCharge *out)
{
    if (!world || !out ||
        (form != SOTN_FORM_BAT && form != SOTN_FORM_MIST && form != SOTN_FORM_WOLF))
        return SDL_SetError("Transformation MP requires progress, a valid form and output");
    if (!SotnWorld_Validate(world))
        return false;
    Uint16 relic = SOTN_RELIC_BAT, cost = 1;
    Uint32 period = 60;
    if (form == SOTN_FORM_MIST)
    {
        relic = SOTN_RELIC_MIST;
        const bool power = (world->relics[SOTN_RELIC_POWER_MIST] & 2U) != 0;
        cost = (Uint16)(power ? 2 : 10);
        period = power ? 30 : 8;
    }
    else if (form == SOTN_FORM_WOLF)
    {
        relic = SOTN_RELIC_WOLF;
        period = 120;
    }
    const bool allowed = (world->relics[relic] & 2U) != 0 && mp > cost;
    *out = (SotnTransformationCharge){
        .allowed = allowed,
        .spent = (Uint16)(allowed && reduce && game_timer % period == 0 ? cost : 0)};
    return true;
}
