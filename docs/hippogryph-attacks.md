# Hippogryph attack dependencies

The internal BO5 profile now owns its original stage identity (25), initialization,
shared tables, camera metadata and bounded actor IDs. It remains disconnected and
has no public gameplay start or save room. Loading the overlay does not establish
a working boss arena.

The native attack adapters cover the body-following weapon hitbox, fire-damage
hitbox, flying objects, expanding flame and flame particles. They retain original
animation, motion, scale/opacity, sound requests and RNG consumption, including
the flying object's fall-through update and byte-sized rotation-direction wrap.
The body-following hitbox reads its private per-pose selector and only follows the
matching owned parent generation. Original effect pools remain bounded; exhaustion
omits the requested effect without replacing unrelated actors or consuming extra
random draws. Private animations, enemy definitions and selectors stay in the
user's dump.

This is a boss dependency batch. Hippogryph's main state machine, placed arena
entry/seals, defeat/reward, after-boss Maria and connected Chapel progression
remain unintegrated. Combat damage routing from the owned pose hitbox to the main
body is part of that pending integration. Full graphics/audio and encounter/console parity remain
unproven. These adapters do not make Chapel a connected, complete playable area.

Synthetic public tests cover motion and lifetime, hitbox selection/fallback,
owned-parent replacement, independent effect pools, pool exhaustion and RNG,
fall-through/timer behavior, death flags, invalid data/owners and bounded motion.
Private component checks compare 568 ticks across 22 cases with the original US
instructions for these routines. The comparison executes original animation,
movement and random helpers, with isolated initialization, allocation, factory,
destruction and sound adapters. It compares actor fields, RNG, sound requests and
effect counts. It is not a complete frame/scene replay, render/listening review,
natural encounter or console comparison.

Provenance: `src/boss/bo5/e_hippogryph.c`, its unreconstructed US routines
`func_us_801A3B88` and `func_us_801A425C`, and shared `src/st/` services at
sotn-decomp revision `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). Public code contains adapted algorithms and table addresses;
original runtime arrays, instructions, artwork and private comparison tools are
not distributed.
