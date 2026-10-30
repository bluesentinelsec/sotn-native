// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/5D5BC.c stat-buff operations, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private tables are embedded.
#include <SDL3/SDL_error.h>
#include <sotn/stat_buffs.h>
bool SotnStatBuffs_Mask(const SotnStatBuffs *s, Uint16 *mask)
{
    if (!s)
        return SDL_InvalidParamError("stat buffs");
    Uint16 bits = 0;
    for (Uint16 i = 0; i < SOTN_STAT_BUFFS; ++i)
    {
        if (s->timers[i] < 0 || s->timers[i] > SOTN_STAT_BUFF_DURATION)
            return SDL_SetError("Stat buff%u must retain its original0..4096 counter", i);
        if (s->timers[i])
            bits |= (Uint16)(1U << i);
    }
    if (mask)
        *mask = bits;
    return true;
}
bool SotnStatBuffs_Tick(SotnStatBuffs *s)
{
    if (!SotnStatBuffs_Mask(s, NULL))
        return false;
    for (Uint16 i = 0; i < SOTN_STAT_BUFFS; ++i)
        if (s->timers[i])
            --s->timers[i];
    return true;
}
bool SotnStatBuffs_Minimum(SotnStatBuffs *s, Uint16 id, Uint16 minimum)
{
    if (id >= SOTN_STAT_BUFFS || minimum > SOTN_STAT_BUFF_DURATION || !SotnStatBuffs_Mask(s, NULL))
        return SDL_SetError(
            "Stat buff refresh requires actual counters, word0..15 and duration0..4096");
    if (s->timers[id] < minimum)
        s->timers[id] = minimum;
    return true;
}
bool SotnStatBuffs_Give(SotnStatBuffs *s, Uint16 id)
{
    if (id >= SOTN_STAT_BUFFS || !SotnStatBuffs_Mask(s, NULL))
        return SDL_SetError("Stat buff producer requires actual counters and word0..15");
    s->timers[id] = SOTN_STAT_BUFF_DURATION;
    return true;
}
bool SotnStatBuffs_Clear(SotnStatBuffs *s, Uint16 id)
{
    if (!s || id >= SOTN_STAT_BUFFS)
        return SDL_SetError("Stat buff clear requires an actual owner and word0..15");
    s->timers[id] = 0;
    return true;
}
bool SotnStatBuffs_Reset(SotnStatBuffs *s)
{
    if (!s)
        return SDL_InvalidParamError("stat buffs");
    *s = (SotnStatBuffs){0};
    return true;
}
