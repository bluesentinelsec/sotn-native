// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/5D5BC.c func_800FE044, dra/5F60C.c and menu.c, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private data is embedded.
#include <SDL3/SDL_error.h>
#include <sotn/familiar_stats.h>
bool SotnFamiliarProgress_Init(SotnFamiliarProgress *p)
{
    if (!p)
        return SDL_InvalidParamError("familiar progress");
    *p = (SotnFamiliarProgress){0};
    for (Uint16 i = 0; i < SOTN_FAMILIAR_COUNT; ++i)
        p->records[i].level = 1;
    return true;
}
bool SotnFamiliarProgress_Validate(const SotnFamiliarProgress *p)
{
    if (!p)
        return SDL_InvalidParamError("familiar progress");
    for (Uint16 i = 0; i < SOTN_FAMILIAR_COUNT; ++i)
    {
        const SotnFamiliarStats *s = &p->records[i];
        if (s->experience < 0 || s->experience > 9899 || s->level != s->experience / 100 + 1 ||
            s->loads < 0 || s->loads > 9999)
            return SDL_SetError("Familiar progress exceeds original level/EXP/load bounds");
    }
    return true;
}
bool SotnFamiliarProgress_Load(SotnFamiliarProgress *p, Uint16 familiar)
{
    if (!familiar || familiar > SOTN_FAMILIAR_COUNT || !SotnFamiliarProgress_Validate(p))
        return SDL_SetError("Familiar load requires an actual selected identity1..7");
    SotnFamiliarStats *s = &p->records[familiar - 1U];
    if (s->loads < 9999)
        ++s->loads;
    return true;
}
bool SotnFamiliarProgress_Reward(SotnFamiliarProgress *p, Uint16 familiar, Uint16 experience,
                                 Uint16 player_level)
{
    if (familiar > SOTN_FAMILIAR_COUNT || !player_level || player_level > 99 ||
        !SotnFamiliarProgress_Validate(p))
        return SDL_SetError("Familiar reward requires actual progress, selection and player level");
    if (!familiar || !experience || player_level == 99)
        return true;
    SotnFamiliarStats *s = &p->records[familiar - 1U];
    Uint32 quotient = experience / (Uint32)s->level;
    Sint32 reward = 0;
    while (quotient)
    {
        ++reward;
        quotient >>= 1;
    }
    if (!reward)
        reward = 1;
    s->experience = SDL_min(9899, s->experience + reward);
    s->level = s->experience / 100 + 1;
    return true;
}
