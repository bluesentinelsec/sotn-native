# Outer Wall Sword Lord

The native Wall profile now runs Sword Lord’s approach, slash and charging stab,
with a separate animation-driven weapon hitbox. Original dust recipes, the staged
death animation, expanding death pulse and falling/fading fragments have bounded
native lifetimes. Fragments and the weapon actor clean up after their owner dies.
The pulse reserves the original pair of primitive slots and preserves the original
allocation-failure fallback.

Animation streams, delays, hitboxes, gravity, fragment positions and particle data
remain in private stage files. Public synthetic tests cover both attack transitions,
weapon ownership, dust expiry, death pulse/fragment cleanup, initial fragment fade
opacity, malformed variants and effect expiry. A private native LLDB fixture checks
both attacks, rendering and death against original room data. It uses an explicit
room and protected player; it does not establish ordinary combat, continuous-route
or console parity. Full effects, dithering, priority and audio/timing parity remain
incomplete.

Adapted from `st/e_sword_lord.h`, selected `st/e_misc.h` particle routines and the
axis-aligned primitive helper at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, under AGPL-3.0-or-later.
No original definitions, graphics, audio or game binaries are included publicly.
