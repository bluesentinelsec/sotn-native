# Hellfire beam component

The core reconstructs Hellfire's original beam controller and retained quad.
The [selected playable owner](player-hellfire.md) now supplies actual casting,
factories, row leases, private gradients, palette feedback, rendering and combat.
This component owns none of those resources itself and grants no MP, learned
progress or ordinary save changes. Full Hellfire/Alucard/console parity remains
outside its scoped instruction comparisons.

An actual successful one-row allocation starts the beam and emits two private
LoadImage requests, sound 0x636 and signal zero. Exhaustion retires the child and
reports original signal 0xFFFF; a pending factory never supplies a ready signal.
A player that leaves Hellfire retires the beam immediately. Retirement alone does
not invent a palette reset: actual beam feedback and the original player recovery/
status-flash helper retain their separate ownership.
The service neither reserves rows nor uploads substitute gradient pixels.

The controller preserves the eight original phases: initialization, vertical growth, widening,
flicker, disappearance/shrink, hidden wait, reappearance and final narrowing.
It retains width 24 rather than clamping at 22, the negative hidden width, original
pre/post-decrement waits, signal1/2/3 handshakes and distinct palette requests.
Geometry/flicker uses the resulting phase after each update. Entering the final
phase immediately reduces brightness; the last retiring update returns before
another geometry/color pass. It follows only the player's X high word, fixes the
Y high word at 120 and retains both original fractions. Dimensions/timers and screen
geometry wrap as signed words. The game clock selects the original gradient row.

Requests retain their original update points: disappearance emits blueprint 38;
signal3 also requests dust blueprint 4 with parameter 10. The later fireball request
samples the **current** Up input, selecting blueprint 34 or 35. Up at birth is not a
latched fireball choice. The component produces these requests without claiming
factory capacity, child identity, row ownership, attack initialization or readiness.
Its owner must bind actual player state and preserve normal finishing/cancellation.

Public tests cover both fireball choices, full lifecycle, request/signal ordering,
one-time upload/sound, hidden palette/geometry, allocation failure, leaving the
cast, retained fractions, signed-word wrap and malformed/removed-state rollback.
They contain no gradient images or original tables. Private comparisons match 216
supplied allocation/cancel/live-Up/fraction/word-boundary contexts and 11,256 full
original controller/quad/palette/signal/request snapshots. The complete original
controller executes; primitive allocation/destruction, LoadImage, factory, sound
and diagnostic calls use explicit boundary adapters. Supplied near-boundary state
is distinct from an ordinary cast. Debug/ASan snapshots match. These are component
comparisons, not playable Hellfire, native teleport/projectile combat, full-frame,
Alucard, castle or console parity evidence. Private inputs/traces remain ignored.

Provenance: selected `dra/84B88.c:EntityHellfire`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original gradient pixels and their descriptors remain private runtime inputs.
See [normal commands](normal-spell-commands.md) for the recognizer's separate scope.
