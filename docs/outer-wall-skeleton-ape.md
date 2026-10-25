# Outer Wall Skeleton Apes

The Wall profile runs the selected walking, front/back punch and carried-barrel
variants. Barrels follow their owner, launch after the original delay, bounce,
produce smoke, and break into four independently bounded fragments. The special
barrel variant breaks on its first landing. Apes retain their separate punch actor
and selected staged death bursts. Actor slot/generation checks prevent detached
children from following a reused owner. Exhausted optional pools preserve the
original fallback rather than creating unbounded actors.

Original animation, collision, hitbox, fragment and motion tables remain in the
private NO1 overlay. Public synthetic tests exercise the encounter variants,
throws, fragments, owner reuse, pool exhaustion and malformed metadata. Private
native checks are separate from uninterrupted route and console parity evidence.
Full effects/audio/timing parity and other Wall encounters remain incomplete.

Behavior is adapted from `src/st/e_skeleton_ape.h` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later. No original
game tables or assets are distributed.
