# Outer Wall Blue Axe Knight

The Wall profile now runs Blue Axe Knight’s armored/damaged animation variants,
approach/retreat, standing and crouched throws, charge/slice and arcing throw.
Its separate weapon actor follows original per-frame hitboxes and validates owner
generation. Thrown axes preserve the original spin, reversal/gravity and break rules.
The attack decision uses a signed timer: decrementing below zero is intentional.

Both original death choices run: bouncing sprite fragments and the expanding fire
pulse. Shared green/blue fragment deaths now reserve the original hidden companion
primitive per visible piece, with the original pool-exhaustion fallback. Particle
and primitive owners release all slots after death. The US armor-break routine’s
single overwritten explosion allocation and initial adjacent-table death offset read
are retained rather than silently corrected. The latter reads checked private bytes.

Public synthetic tests cover attack selection, arc/reversal, signed timer wrapping,
armor loss, charge, weapon ownership, alternate death and exhausted primitive pools.
The green knight regression also checks hidden companion ownership/cleanup. Private
LLDB checks use original room data and a protected player, including both selected
death branches. Those fixtures do not establish continuous-route or console parity.
Full effects, priority/dithering, elemental damage and timing/audio parity remain
incomplete.

Adapted from `st/e_blue_axe_knight.h`, `st/entity_axe_knight.h`, selected
`st/e_misc.h` effects and primitive helpers at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
Original animation, motion, hitbox, sprite, graphics and audio tables stay private.
