# Olrox arena furnishings

BO0 now runs its placed breakable table, seats and candelabra, the original
adjacent table destruction group, falling/rotating pieces and explosion handoffs.
Both sitting points accept the original Up-only trigger, alignment and seated
pose, release on movement, and disappear when their associated furniture breaks.
Destroying the left seat also supplies the original adjacent event flag read by
Olrox’s introduction; this does not synthesize a boss victory.

Profiles, hitboxes, HP, priorities and artwork come from the user's private BO0
files. The thirteen furniture/debris variants retain their separate original
phases, fractional movement, rotation and floor sensors. Table destruction checks
its adjacent placed group before changing it. Effect-pool exhaustion omits optional
pieces without replacing unrelated actors. Explosion conversion retains the
source’s rotation/draw state. Invalid variants and motion overflow fail explicitly.

Original sitting uses the existing normal-form player service. The selected
furniture integration does not implement transformations, Alucard afterimages,
full effects/audio or whole-encounter/console parity. Connected NO2 travel remains
a separate milestone. The [encounter](olrox-encounter.md) describes both boss forms
and the actual defeat/reward flow.

## Validation

Public authored fixtures exercise all variants, private-profile bounds,
adjacent-group failure, damage transitions, sitting alignment/exit, destroyed-seat
removal, rotation-preserving explosion conversion and checked motion overflow.
No original runtime records or artwork appear in these fixtures. All 123 test
groups pass in Debug, Release and macOS ASan/UBSan; release packaging passes.

A private instruction comparison matches 8,320 complete furniture/debris state
and factory snapshots, covering thirteen variants, both facing directions and
camera offsets. Original furniture, allocation, creation and movement instructions
execute against the private dump; initialization, collision and audio use explicit
adapters. The fixture supplies a flat floor and damage flags, and does not advance
explosion children. This establishes component agreement, not console parity. Debug and sanitizer traces are byte-identical.

A separate muted native sequence walks to the placed right sitting point, sits
and stands, then breaks five placed furniture fixtures using ordinary development
Gladius attacks. It advances the original grouped destruction and debris, without
injected hit/death signals or equipment/stat grants. It uses an explicit arena
start and contact protection. Debug and ASan/UBSan pass. The actual CMake executable independently exercises
this route through LLDB and exits through normal SDL quit. Ordinary user save
slots remain untouched. These checks are not an uninterrupted castle replay.

Provenance: reconstructed BO0 `2D26C.c:func_us_801B001C` and shared `st/e_chair.h`,
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Generated analysis, original tables/assets, traces and captures remain private.
