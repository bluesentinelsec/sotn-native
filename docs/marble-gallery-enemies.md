# Selected Marble Gallery enemies

The native Marble profile now runs Flea Men, Slingers and their ribs/death parts,
Ouija Tables and their linked objects, and Stone Roses with their segmented stems,
head, seeds and selected death effects. Their original entity identities, private
initializers, animation streams, motion tables and collision sensors remain separate.
Other Marble actors still emit the development checkpoint's unsupported diagnostics.

## Behavior and ownership

Ouija Tables reserve their original three child slots. Stone Roses reserve twelve
linked segments. Child references include slot generations so a reused slot cannot
be mistaken for the original owner. The Rose head receives normal damage; impervious
stem parts retain their original behavior. Hit cooldowns and head-death signals
propagate through the segment ring before behavior updates.

Actor collision detection now precedes the stage actor update pass, following the
original engine's ordering. This lets a linked root observe a later segment's hit
flags during the same tick. It does not establish complete engine timing parity.
Lethal hits preserve the original death flags and clear stale hit-stun suppression.

The selected direct animation reader handles bounded 16-bit duration/pose streams,
loops and termination. Unsupported animation-switch opcodes fail explicitly. Large
vector angle calculations use the private SDK lookup table and its original ratio
normalization; no public game tables are embedded.

## Validation and limits

Public synthetic tests cover Flea hit reactions, Slinger projectiles and death parts,
Ouija waking/throwing and child cleanup, linked Rose lethal damage despite prior
hit stun, bounded direct animations, seed recipes/pool exhaustion and vector angle
normalization. Private LLDB fixtures exercise all four families against the user's
stage data, including rendering and selected death paths. Those fixtures start in
explicit rooms and protect the player; they do not prove an uninterrupted route,
unassisted combat balance or console parity. Dummy audio checks decoding only.

Full effects, drops, audio/timing parity, clock mechanisms and the remaining enemy
families are still incomplete. Do not call Marble Gallery complete.

## Provenance

Adapted from `st/no0/e_slinger.c`, `st/no0/e_ouija_table.c`,
`st/no0/e_stone_rose.c`, shared Flea/seed helpers, `dra/alu_anim.c`, and the
collision/update ordering in `dra/5087C.c` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
The vector normalization follows the pinned PSY-Z SDK implementation. Original
assets, tables and game binaries stay in ignored private runtime inputs.
