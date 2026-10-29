# Hellfire supporting effects

The core reconstructs selected original expanding circle 28, red flash 29, trailing
sprite 24 and rising tile-particle profile 10. These components enable no Hellfire
controls. Actual player casting, shared factories/rows, gradient uploads, private
animation/artwork, spell attacks and native rendering/combat still need to connect.
Selected Dark Metamorphosis, Summon Spirit and Tetra Spirit remain playable.

The circle borrows one actual GT4 row, follows the live player's high position
words/facing while retaining its own fractions, expands by two and fades only
after width 40. It continues after casting ends. Width above 56 retires before
packet/color updates. Its original white palette and texture remain private.

The red flash borrows one actual G4 row, follows only X, uses original alternating
clock visibility and retires on the original post-increment timer test. Failed
allocation reports FFFF; successful birth does not clear the player's signal.
Leaving Hellfire immediately retires the actual child. The original missing C
argument to DestroyEntity is replaced by explicit owned-child retirement; private
instruction checks verify that the original binary passes that same child.

The selected trail accepts full parameters whose high byte is one, preserving
factory-facing, X velocity and blend metadata. It consumes one fifteen-bit rand
value only at actual birth, shifts the Y high word, retains fractions and requests
the private animation. Supplied negative pre-animation duration retires the child.
The native owner must clear it; retained component fields are not a live entity.
Other trail producers remain outside this selected component.

Profile 10 reads its bounded private profile, requests the actual count plus a
sentinel row and initializes at the live player rather than the factory origin.
It consumes no RNG, retains the original per-particle upward velocities and signed
delay, and performs the original double lifetime decrement. Leaving Hellfire
retires before that decrement; a birth still initializes before a later cancellation
check. Existing profiles 1, 9 and 13 retain their behavior.

Shared particle off-camera retirement now checks the original KEEP_ALIVE_OFFCAMERA
bit, 0x04000000. Previously it tested an unrelated flag. Actual keep-alive children
now retain their rows offscreen; ordinary children still initialize and consume
birth RNG before culling. No substitute pool or default ability grant is added.

Public authored fixtures cover both facings, growth/fade, cancellation, exhausted
rows, timers/word wrap, trail metadata/RNG, particle lifetime and shared-runtime
culling. Private instruction comparisons exercise supplied player clocks/positions,
allocation results and animation durations with explicit allocator/RNG/destruction
adapters. They cover 72 circle/flash contexts and 526 snapshots, 18 supplied-animation trail
contexts and 216 snapshots, plus 96 private particle contexts, 2,817 updates and
26,826 particle records across profiles 1/9/10/13. Debug and ASan/UBSan snapshots
match. They are component evidence, not a native spell, full-frame replay or
Alucard/castle/console parity. Private profiles, artwork and traces stay local.
Ordinary starts and version-nine saves remain unchanged.

Provenance: selected `dra/84B88.c:{EntityExpandingCircle,func_80127CC8}`,
`dra/7879C.c:EntityUnkId24` and `dra/7E4BC.c:func_8011E4BC`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
See [beam lifecycle](hellfire-beam.md) and [fireball motion](hellfire-fireballs.md).
