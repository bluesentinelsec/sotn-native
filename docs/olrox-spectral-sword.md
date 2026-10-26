# Olrox's Quarters Spectral Sword

The internal NO2 checkpoint now runs its original placed Spectral Sword in room 2,
fifteen distinct poltergeist weapons and owned aura. This is NO2's encounter,
with different formations, attack ordering, hitbox tables and identities from the
Chapel version. Stage 4 remains disconnected while mechanisms, Olrox and travel
are integrated. This is not a complete authentic area or game.

## Behavior

The body waits for the player's horizontal and vertical approach, gathers its
weapons into a circle, patrols with the original projected formation, lines up
its surviving weapons and launches staggered attacks. Each weapon retains its
private frame, angle adjustment, rotating hitbox and independent HP/damage.
Destroyed weapons reduce the body's live count once, fall with selected effects
and remain hidden until their owner is cleaned up. The body's physical defeat
releases all remaining weapons and runs its original falling death. Selected
attack/death sounds, palette alternation and expanding aura run on simulation ticks.

Preserve fixed child slots and generation ownership. Check every reservation
before replacement; never overwrite another placed actor or unrelated child.
Re-entry recreates matching owned survivors after the room owner releases the
previous body's presentation. Stale weapons and auras cannot affect a replacement
body. Native cleanup retires orphaned children when their body disappears.

The original 3D formation projects all fifteen positions, then assigns consecutive
positions to surviving weapons. Its assignment cursor advances only for survivors;
do not change that ordering during authenticity work. Position-high-word writes
retain existing fractional parts. Screen anchors, offscreen thresholds and the
attack's screen-center target must include the current camera translation.

## Data and native boundaries

All original initialization, artwork, frame hitboxes, weapon-angle adjustments,
layout and SDK tables remain private. NO2 identities are body `0x3E`, aura `0x3F`
and poltergeist `0x40`; Chapel's adjacent values identify different actors.

The selected native orbit projection uses the existing fixed matrix, saturated
coordinates and GTE divider direction. It retains separate matrix-pair and
quarter-wave trigonometric tables. Native code validates ranges and owns its
projection outputs; it neither executes console instructions during gameplay nor
stores private lookup arrays in source or packages.

A full primitive pool retains the original body's failed-allocation removal;
optional aura/death-effect allocation can be omitted when the effect pool is full.
These cases do not overwrite another owner. Full hit palettes, elemental reactions,
contact ordering, GPU effects/audio and console presentation parity remain incomplete.
Transient weapons do not change ordinary save version nine or write existing slots.

## Validation

All 107 Debug, macOS AddressSanitizer/UndefinedBehaviorSanitizer and Release
test groups pass, along with asset-free `make package` output.

Public tests use authored synthetic profiles, hitboxes, SDK functions and empty
room layouts. They cover all three formations, wake-up thresholds, weapon gather/
activation, staggered throws, independent weapon HP/contact cooldown, count-once
deaths, zero survivors, parent death, aura ownership, re-entry, stale generations,
conflicting reservations, primitive exhaustion and malformed states/counts.
Selected projection checks also verify cardinal axes and unchanged output on error.

Private comparisons cover 124,800 body/weapon snapshots over 7,800 family ticks.
Original US instructions run with isolated initialization, allocation and sound
adapters; selected GTE operations use the pinned PSY-Z SDK reference. Inputs cover
ordinary state sequencing, injected individual weapon deaths, zero survivors and
injected body death. Compared fields include positions, velocities, rotations,
formations, attack cursor/timers, weapon targets, priority, hitboxes, attack profile,
RNG, sound requests and effect counts. This is component evidence, not a console
or uninterrupted encounter replay.

A separate 40,000-point comparison checks native orbit coordinates and depth
against that isolated SDK reference using the private original matrix-pair table.

A native LLDB sequence starts original room 2 and protects the ordinary development
character without granting stats/equipment or enemy damage. Normal movement,
jumps and Gladius attacks hurt and defeat the placed 90-HP body, award 243 total
EXP including defeated weapons, and complete cleanup at tick 1726 plus 180 effect
ticks with 477 rendered frames. Debug and macOS ASan/UBSan both pass.
The omitted room actor `0x24` still warns; this
sequence does not validate that mechanism, ordinary balance, connected traversal,
whole-area or console parity. Private inputs and diagnostics stay ignored.

Provenance: `st/no2/e_spectral_sword.c`, `st/e_spectral_sword_aura.h`,
`st/step_towards.h`, selected `st/st_common.h` and NO2 identities at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). The selected
matrix/divider adapter follows the pinned PSY-Z implementation documented in
[third-party notices](../THIRD_PARTY.md) (MPL-2.0). Runtime arrays remain private.
