#ifndef SOTN_SPELL_TARGET_H
#define SOTN_SPELL_TARGET_H
#include <SDL3/SDL_stdinc.h>
#include <sotn/game_data.h>
#define SOTN_SPELL_TARGETS 128
/** Every original stage slot 64..255; selectors only scan the first SOTN_SPELL_TARGETS. */
#define SOTN_STAGE_TARGETS 192
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
/** @brief w_041 func_123000_8017A994: Monster vial 2's bat picks a candidate whose ratan2 heading
 * from (x, y) lies within @p tolerance of @p heading, marking the first unmarked one; otherwise
 * it takes the next already-marked one in range from its program's own circular cursor.
 * @param slots Actual stage slots64..191 in order; marks commit in this snapshot only.
 * @param executable Private SDK ratan2 tables. @param x,y The bat's screen position.
 * @param cursor Owned program BSS D_123000_8017B200 (non-negative).
 * @param selected Receives the stage-relative index or SDL_MAX_UINT16 when none qualify.
 * @return Success or SDL error preserving inputs/output. Consumes no RNG. */
bool SotnSpellTarget_SelectAngle(SotnSpellTarget slots[SOTN_SPELL_TARGETS], SotnDataView executable,
                                 Sint16 x, Sint16 y, Sint16 heading, Sint16 tolerance,
                                 Sint32 *cursor, Uint16 *selected);
#endif
