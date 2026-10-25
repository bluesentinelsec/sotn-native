# Marble Gallery linked enemies

Skelerang and Diplocephalus now run in the native Marble profile, alongside the
[other selected Marble enemies](marble-gallery-enemies.md). This closes two more
encounter gaps; the area and connected route still need broader validation.

Skelerang reserves its original helper and two boomerangs. It wakes, cowers,
throws, steers the returning weapons toward their captured target and then its
owner, catches them, and follows the original selected death paths. Slot-generation
checks prevent a reused actor slot from becoming a boomerang's owner.

Diplocephalus reserves its original nineteen-slot span. Its body updates the feet
in the original gait order; legs follow the body/feet, and seven tail segments
retain their independent poses and rotating hitbox. The head takes ordinary damage;
impervious parts keep their original response. Cooldowns and lethal signals follow
the original linked chain. The torso provides the original player platform contact.
Selected fireball acceleration, steering, twenty owned trail primitives, limb death
motion, tail destruction and explosion trails run from private stage tables.

The new effects use their original selected actor pools and integer spawn positions.
Private motion/position and animation reads are bounded. Complete dust/drool,
camera shake, hit flashes, drops and audiovisual/timing parity remain incomplete.

## Evidence and scope

Public synthetic coverage checks boomerang throw/return and owner reuse, fragment
lifetimes, gait transitions, lethal head damage through an existing stun, linked
death cleanup, fireball speed and primitive cleanup, and invalid reserved ranges.
All tests use original synthetic fixtures rather than game data.

Private LLDB fixtures render both families, exercise boomerangs and a scripted
fireball attack, and check body/limb cleanup after forced lethal hits. Those checks
use explicit room starts and a protected development player. They are not a
continuous route, ordinary combat playthrough or console parity replay.

Behavior is adapted from `st/e_skelerang.h`, `st/no0/e_diplocephalus.c`,
`st/no0/e_diplocephalus_tail.c`, and selected shared collision, spawn and primitive
helpers at sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under
AGPL-3.0-or-later. Game definitions, frames, sensor arrays, motion tables, graphics
and audio remain in ignored user-supplied runtime files.
