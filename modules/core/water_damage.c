// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c water injection/response9 and 5D5BC.c:HandleDamage kind6,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private data is embedded.
#include <SDL3/SDL_error.h>
#include <sotn/water_damage.h>

bool SotnWaterDamage_Tick(SotnWaterDamage *state, SotnWaterDamageInput in,
                          SotnWaterDamageEvents *out)
{
    if (!state || !out || state->pain_timer > 32 || !in.hp || in.hp > in.hp_max ||
        in.hp_max > 9999 || in.waist > 32767)
        return SDL_SetError("Water damage requires an alive bounded player and actual DRA timer");
    SotnWaterDamage next = *state;
    SotnWaterDamageEvents e = {.hp = in.hp, .hit_params = in.hit_params};
    if (next.pain_timer)
        --next.pain_timer;
    if (in.waist && !in.holy_symbol && !(e.hit_params & 31U))
    {
        e.hit_params = 6;
        e.scheduled = true;
    }
    if (!in.invincible && !in.grabbed && e.hit_params)
    {
        if (e.hit_params != 6 || (in.status & 0x80U))
            e.delegated = true;
        else
        {
            e.damage = in.game_timer % 10U == 0 ? 1 : 0;
            e.hp = (Uint16)(in.hp - e.damage);
            if (!e.hp)
                e.response = 6;
            else
            {
                e.response = 9;
                e.blink = 0x4e;
                e.voice = !next.pain_timer;
                e.scrape = e.damage != 0;
                e.defense_buff_min = in.fury_plate ? 512 : 0;
                next.pain_timer = 32;
            }
        }
    }
    *state = next;
    *out = e;
    return true;
}
