#ifndef SOTN_SPELL_TARGET_H
#define SOTN_SPELL_TARGET_H
#include <SDL3/SDL_stdinc.h>
#define SOTN_SPELL_TARGETS 128
/** @brief Copied actual original stage slot, in screen coordinates; contains no asset data. */
typedef struct SotnSpellTarget
{
    Uint32 flags;
    Uint16 identity, hitbox;
    Sint16 hp, x, y;
} SotnSpellTarget;
/** @brief Select an original spirit target, marking only the first unmarked candidate.
 * @param slots Actual stage slots64..191 in order; selected marks commit in this snapshot only.
 * @param cursor Owned original circular global,0..127, retained across stage changes.
 * @param selected Receives stage-relative index or SDL_MAX_UINT16 when none qualify.
 * @return Success or SDL error preserving inputs/output. The native owner commits copied
 * flags against actual slot identity/generation. This consumes no RNG, allocates no child,
 * and confers no attack readiness. A retained original pointer follows its actual slot. */
bool SotnSpellTarget_Select(SotnSpellTarget slots[SOTN_SPELL_TARGETS], Uint16 *cursor,
                            Uint16 *selected);
#endif
