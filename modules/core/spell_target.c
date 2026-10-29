// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7879C.c:func_80118970, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include <SDL3/SDL_error.h>
#include <sotn/spell_target.h>
static bool Candidate(SotnSpellTarget t)
{
    return t.identity && t.hitbox && !(t.flags & 0x200000U) && t.x >= -16 && t.x <= 272 &&
           t.y >= 0 && t.y <= 240 && t.hp < 0x7000;
}
bool SotnSpellTarget_Select(SotnSpellTarget slots[SOTN_SPELL_TARGETS], Uint16 *cursor,
                            Uint16 *selected)
{
    if (!slots || !cursor || !selected || *cursor >= SOTN_SPELL_TARGETS)
        return SDL_SetError(
            "Spell target selection requires actual stage snapshots and its bounded cursor");
    for (Uint16 i = 0; i < SOTN_SPELL_TARGETS; ++i)
        if (Candidate(slots[i]) && !(slots[i].flags & 0x80000U))
        {
            slots[i].flags |= 0x80000U;
            *selected = i;
            return true;
        }
    for (Uint16 i = 0; i < SOTN_SPELL_TARGETS; ++i)
    {
        const Uint16 at = (Uint16)((*cursor + i) % SOTN_SPELL_TARGETS);
        if (Candidate(slots[at]))
        {
            *cursor = (Uint16)((at + 1U) % SOTN_SPELL_TARGETS);
            *selected = at;
            return true;
        }
    }
    *selected = SDL_MAX_UINT16;
    return true;
}
