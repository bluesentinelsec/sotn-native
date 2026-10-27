# Olrox orbiting projectiles and portals

The internal BO0 encounter now advances the humanoid body's orbiting projectiles,
bat portals, homing bats and bat shadows. These use their original identities,
private definitions, animations, stage sine samples and SDK lookup tables.

Nine owned projectiles spiral toward the body's original elevated center, enable
their physical hitboxes on arrival, orbit with bounded original steering, and
launch toward Alucard on their individual delayed release. Readiness comes from
the real child updates; the body waits for every retained projectile. Positions
retain fractional words during high-word assignments, and the original stage
sine table supplies eight-bit direction velocities. The SDK quarter-wave table
is a separate input for the spiral's twelve-bit geometry. Parent death produces
the original explosion request and retires the child. Preserve the sound-order
quirk: positional attenuation uses the cleared entity after original destruction.

Portals grow through zero-to-unit scale, retain the original extra growth tick,
spawn into the original eight-slot primary-bat pool every twelve ticks, cycle
palettes and shrink on the body's close request. The zero-scale closing tick
survives until the following signed-negative test. Optional pool exhaustion
omits that birth without consuming extra RNG or queuing a birth sound.

Bats fade in before enabling their hitboxes, steer toward the original target
above Alucard, and optionally create a separate shadow. Shadows retain independent
animation and fractional coordinates while copying the primary's integer
position and opacity. Generation checks retire stale dependents without reading
reused actor slots. The first-form transition flag requests the original bat
retirement effect. Its selected [configured retirement presentation](olrox-fire-effects.md) now runs.

Sprite presentation now accepts explicit original scale/rotation/opacity enable
bits. Disabled values resolve to neutral transforms even when an actor retains
old scale or opacity words. Enabled zero axes remain invisible. Existing native
adapters retain their legacy selectors. This corrects projectile growth scaling
remaining visible after the original routine clears its enable flags.

## Validation and scope

Public authored tests cover spiral/orbit/readiness/release, delayed launches,
portal growth/closing boundaries, timed birth and pool exhaustion, bat fade and
shadows, death requests, generation reuse, malformed SDK/sine data and invalid
states. A public pixel regression compares unit-scale drawing byte-for-byte after
clearing explicit transform flags while retaining stale scale/rotation/opacity
values; independent axes, signed scale and legacy behavior also have coverage.
No original fixture tables or artwork are included.

Private comparisons match 8,400 complete body/family snapshots across 24 seeded
sequences, including actual child readiness and semantic hashes of the active
projectile/portal/bat/shadow states. They execute original US body, child, motion,
animation, steering, random and factory instructions. Isolated adapters supply
initialization, a flat floor, audio, visits, Alucard-afterimage suppression and
GTE leading-zero counting for the original square-root routine. Unintegrated
fireballs/skull columns are not advanced, and optional death/body-image effects
are cleared identically at tick boundaries. This is selected family evidence,
not full shared-capacity, effects, audio or encounter parity.

Muted native Debug and macOS ASan/UBSan LLDB runs start at the placed body and
advance its natural introduction and commands. Nine projectiles spiral, orbit
and launch, followed by portal births of at least three bats and three shadows.
Equivalent CMake-app debugger checks use normal SDL quit. Fixtures supply only
an explicit room arrival and contact protection; they do not grant states,
positions during play, HP, stats, relics, equipment or readiness. User saves remain
untouched and private inputs, scripts, traces and captures remain ignored.

Selected transformation, second form and defeat/reward now run in the
[native encounter](olrox-encounter.md). Furnishing behavior and connected
progression remain incomplete. BO0 and NO2 remain
disconnected. These component/native checks do not establish ordinary castle
entry, an uninterrupted route, console pixel/PCM parity or a complete game.

Current integration: the selected [native encounter](olrox-encounter.md) now
connects both forms, real morph/attacks and captured defeat/reward. Earlier
component checks above retain their narrower evidence scope. Furnishings, full
effects/presentation/parity and connected BO0/NO2 travel remain incomplete.

Provenance: BO0 `2D26C.c` routines `func_us_801AFAF4`, `func_us_801AF8C0` and
`func_us_801AF604`, and selected shared math/factory helpers, at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later), reconstructed
from the matching user's private US instructions. SDK algorithms retain their
existing documented provenance. All original lookup, animation, definition,
artwork and sound arrays remain private runtime inputs.
