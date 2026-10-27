# Mist movement component

The internal Mist movement cursor now reproduces the original initialization,
steering, deceleration, duration, MP reduction and restoration request. It remains
outside player/input dispatch until collision, transformation effects and the real
unmorph controller are integrated. Mist is not playable yet; no ordinary-start
relic grant or public control binding is introduced.

The caller first applies [original transformation eligibility](transformation-resources.md),
then supplies copied movement, relic, MP, direction and simulation-clock inputs.
Initialization clamps each nonzero velocity to the original signed orthogonal
speed, requests original animation/weapon/palette/effect setup and sets the source
duration. Active movement preserves the slower diagonal speeds, gradual reduction
of excess speed, immediate directional reversals and neutral/opposed-direction
coasting. The original facing helper’s turn signal is distinct from active steering.

Water or a fresh cancellation bypasses MP reduction. Otherwise MP is checked and
charged before ordinary Mist’s duration decreases; even the expiration tick can
cost MP. Power of Mist bypasses that duration decrement. Disabling it retains the
long existing duration and resumes decrementing instead of resetting it. Found
but disabled primary ownership ends active movement. Initial control substep zero
precedes cancellation/charge checks, just as in the original controller; it is not
an alternative to entry eligibility.

The original settling offset consumes one whole pixel per tick while preserving
the caller’s coordinate fraction. A restoration request stops this cursor and
clears its control substep. It never means restoration has completed: the owning
player must still restore sensors, escape a Mist-only barrier if necessary, and
wait for the actual transformation effect. Malformed input leaves the cursor and
output unchanged. No graphics readiness is synthesized.

## Validation

Public fixtures cover initialization ordering, duration/cost ordering, diagonal
and orthogonal motion, opposing directions, turn signals, disabled relics, Power
of Mist changes, settling and failed-input atomicity. All 125 test groups pass in Debug, Release and macOS
ASan/UBSan; release packaging passes.

A private comparison matches 17,920 one-tick original `ControlMistForm` and
`MistFormFinished` snapshots: all direction combinations, excess/small/opposed
velocities, power/primary ownership states, short/long/zero durations, water,
cancellation, MP exhaustion, facing locks, settling offsets and timer wrap. Debug
and ASan/UBSan traces are byte-identical and exit successfully through muted LLDB. Original
MP/relic, direction, deceleration and player-state helpers execute. Animation,
palette, weapon-release and factory calls use explicit adapters, including checks
of the original initialization requests. This is movement-component agreement;
collision, actual visual effects, restoration completion, playable Mist and
console/whole-route parity remain separate work. Original code/data and traces
stay in ignored local paths.

Provenance: `dra/71830.c:ControlMistForm/MistFormFinished` and
`dra/6DF70.c:CheckMoveDirection`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
