# Olrox monster body component

The original monster controller now connects its owned legs, torso, head and arm
commands: transformation handoff, recovery/roar, alternating forward/backward
steps, turns, takeoff, leap/slam, swipe and HP-ranked laser/fireball requests.
Requests wait on real child busy/release fields. The first takeoff tick retains
its original leg-zero selection even after selecting the eventual support leg.

The body remains outside gameplay dispatch until shared combat, complete defeat
and reward flow are integrated. It does not yet make the second form playable.
No ordinary progression, relics, equipment or save state are granted by this work.

## Ownership and coordinates

One generation owns both articulated legs. The body creates original feet, head,
tail, knees, arms and independent hands in its bounded reserved block. Handoff
may replace released humanoid attack siblings only when their generation belongs
to the same live morph lineage. Unrelated placed or dynamic actors fail explicitly.
Optional packet exhaustion preserves the original initialization retry.

Room-local fixed-point state adapts the original screen/scroll coordinates.
The first eight parts acquire the original camera-lock flag only after the
morph controller finishes. Joint phases reset on a body step change; retained
angles/velocities do not. Geometry uses the existing original leg/torso helpers,
with selected palette/death-image overrides. No runtime tables are embedded.

## Validation and limitations

Public authored tests cover private-profile HP ranks, real attack release waits,
transformation completion gating, ownership protection, initial leg selection,
invalid support roles and bounded fixed-point motion. Synthetic tables do not
establish parity.

Private Debug and macOS sanitizer comparisons match 52,000 seeded snapshots from
original instructions across entry, thirteen selected starting states, both
facings and multiple camera positions. They check body motion, phases,
joint/foot/knee state, child commands,
visible geometry, RNG and sound requests. Original body, articulated movement,
geometry, SDK and factory instructions execute; initialization, allocation,
audio, shake and leading-zero boundaries use adapters. Feet advance, but head,
arm and attack completion signals are supplied by the fixture. This is component
evidence, not an encounter or console replay. Defeat, shared HP integration and
full shake/effect presentation remain pending.

All 123 tests pass in Debug, Release and macOS ASan/UBSan; Release packaging
passes. Muted actual CMake-app checks retain ordinary Entrance startup and normal
SDL quit without room, inventory, position or save fixtures. These are regression
checks, not monster gameplay evidence.

Provenance: BO0 `313A8.c:func_us_801B365C` and selected joint helper at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Authoring disassembly, generated analysis and private traces are not shipped.
