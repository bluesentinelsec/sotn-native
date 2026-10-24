#ifndef SOTN_COMBAT_H
#define SOTN_COMBAT_H

#include <SDL3/SDL.h>

/** @brief Whole-pixel hitbox; dimensions are original half extents. */
typedef struct SotnCombatBox
{
    Sint32 x, y;
    Uint16 width, height;
} SotnCombatBox;

/** @brief Copied enemy combat attributes decoded from a private enemy definition. */
typedef struct SotnCombatEnemy
{
    Sint16 defense;
    Uint16 weaknesses, strengths, immunes, absorbs;
} SotnCombatEnemy;

/** @brief Original hit-test arithmetic, including the target's one-pixel inset.
 * @param attacker Whole-pixel attacking box. @param target Whole-pixel target box.
 * @return True for an intersection; zero-sized or oversized boxes never intersect. */
bool SotnCombat_Overlap(SotnCombatBox attacker, SotnCombatBox target);
/** @brief Advance the PS1 libc random sequence without host-global state.
 * @param state Initialized owned random state, updated on return. @return Original 15-bit value. */
Uint16 SotnCombat_Random(Uint32 *state);
/** @brief Resolve original elemental/critical/defense damage for an unbuffed player.
 * @param enemy Original target attributes. @param attack Bounded attack power, 0..999.
 * @param element Original attack element bits. @param critical_rate Effective 0..255 rate.
 * @param stats Four base stats (STR, CON, INT, LCK), each 1..99.
 * @param rooms Visited room count for INT criticals, 0..1024.
 * @param random Owned PS1 libc random state. @param out Receives original packed damage
 * flags/value.
 * @return Success, or false with SDL_GetError() and unchanged random/output on invalid arguments.
 * @note Equipment modifiers must be applied before calling; no host rand() is used. */
bool SotnCombat_Damage(SotnCombatEnemy enemy, Uint16 attack, Uint16 element, Uint8 critical_rate,
                       const Uint16 stats[4], Uint16 rooms, Uint32 *random, Uint16 *out);

#endif
