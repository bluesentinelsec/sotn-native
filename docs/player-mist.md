# Selected playable Mist

Owned, enabled Mist now connects original entry checks, movement, collision,
capture/cloud effects and restoration to the native session. Q and Xbox LB toggle
it. Power of Mist removes the short duration limit; both relics can be enabled or
disabled through the relic panel. Ordinary starts grant neither relic. Ordinary
acquisition and a complete authentic Alucard remain unfinished.

## Simulation and ownership

Entry uses the [transformation resource rules](transformation-resources.md), the
absolute simulation clock and original 16-tick startup/24-tick normal-return
lockouts. The [movement cursor](mist-motion.md) steers and charges MP. The player
reports each cost once to character progress; transformed states suppress original
MP recovery. Water, cancellation, disabled ownership, depleted MP or ordinary
Mist's duration initiate restoration. Inverted Gravity Boots entry restores the
original capture pose and facing first. Weapons and shields retire on entry.

Compact floor, ceiling and wall sensors pass through Mist-only tiles and selected
one-way surfaces. Restoration grows the original sensors, retains the old ceiling
probe for squeezed-space detection and uses the [barrier/edge helpers](form-restoration-collision.md).
Slope handling tests bit `0x8000`. A successful zero-pixel floor-edge escape still
stops horizontal motion. Cramped returns enter the original stuck/crouch recovery.

The player issues a monotonically increasing child generation. The session binds
that actual child's phase before each player tick; stale or missing owners fail
explicitly. Restoration waits for the [real visual phase five](mist-effect.md),
then releases that child. A timer never substitutes for readiness.

The session captures the real private ARC pose through [owned textures/palettes](mist-presentation.md).
Character-start palette job 22 initializes DRA's player/Mist source colors; raw
common artwork contains placeholder rows. Current cape colors feed restoration
without resetting live fades. Cloud primitives use additive texture pages,
vertex modulation, original priorities and the active target/clip. Their four
initial random values come from the existing saved actor RNG stream.

Camera motion and room rebases translate retained ring fractions even without a
live child. Live anchors and quads move separately; cached capture geometry stays
unchanged. Flying forms use the original lower room-crossing window. A full stage
initialization destroys form entities and restores normal Alucard, as the original
engine does; global ring/color state survives that entity reset. Fresh Continue
restores ordinary progress, not transient form entities. Save format nine is unchanged.

## Validation and limits

All 129 public test groups pass in Debug, Release and macOS ASan/UBSan;
`make package` passes. Public authored fixtures exercise owned/disabled relics, strict MP eligibility,
motion/cost, real effect contraction/restoration, generation failures, water,
lockouts, Mist barriers, cramped recovery and the slope-bit distinction. Rendering
fixtures cover palette initialization, capture and command bounds. The separate
component documents retain their original-instruction comparison scope.

Private muted LLDB checks use this project's CMake libraries and executable.
Two native cycles cover manual cancellation and natural MP exhaustion; simulation
snapshots agree across two render cadences and common drawn checkpoints agree.
A connected Laboratory room crossing retains the cloud generation. An isolated
save-room UI write and fresh byte-identical version-nine load retain relics and
ordinary progress, heal MP and create no transient cloud. Existing user slots
remain untouched. Debug/sanitizer native traces agree, and both actual CMake app
builds return normally after transformation through muted LLDB. Relics are
explicit fixture grants, with ordinary development health/stats/MP; these checks
do not establish ordinary acquisition or uninterrupted castle traversal.

Another component comparison covers 2,880 original restoration snapshots across
actual visual phases, sensor growth, terrain, contact and velocity conditions.
Original controller/helpers execute against an adapter to this project's collision
decoder. It does not independently validate PS1 collision decoding or an entire
simulation tick. Capture/palette comparisons initialize the real source palette
before checking 1,578 captures and 1,492 palette records. Runtime inputs and all
authoring traces/images remain private.

[Gas Cloud combat](player-gas-cloud.md) now runs within its selected scope.
[Selected ordinary Bat flight](player-bat.md) now runs, including real cross-form
conversion. [Selected Wing Smash](player-wing-smash.md) now connects its commands
and owned attacks/effects. Fire/Echo of Bat, Wolf, complete restoration flashes,
original shared factory/palette
contention, complete water-volume behavior and full audio/raster/timing parity
remain incomplete. The current rasterizer does not reproduce requested PS1
dithering. Whole-pixel camera/rebase checks do not establish fractional stage
correction parity. None of these checks prove console, whole-route or full-game
parity.

Provenance: selected `dra/692E8.c`, `71830.c`, `6DF70.c`, `7E4BC.c`,
`7879C.c`, `4A538.c`, `4B758.c` and `5087C.c`, sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original runtime tables and artwork remain checked private inputs.
