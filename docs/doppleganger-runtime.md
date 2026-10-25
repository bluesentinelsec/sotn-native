# Doppleganger runtime foundation

BO4 now has independent native collision and decision services in addition to its
private stage, sprite and animation adapters. These services prepare the encounter;
they do not yet instantiate the boss, play the intro, resolve its attacks/death,
award its reward or open ordinary travel into the arena.

The collision service reads the boss's original sensors from private BO4 data.
It retains separate floor, ceiling and fourteen wall probes, transformed geometry,
slope/one-way/mist handling, contact history, fixed-point fractions and the original
four-step fast movement. BO4 uses five-pixel wall probes rather than Alucard's
seven-pixel probes. The source's short-sensor selector tests the bat/mist status
bits; normal crouching retains the default geometry. Unmorph transitions can retain
the previous sensors. Coordinate wrapping and negative quarter-step rounding are
explicit, and all probes and output updates fail atomically on invalid inputs.

The decision service emits original simulated PS1 controls from a simulation-tick
snapshot. It retains distance thresholds, startup reactions, original libc RNG
consumption, attack cadence, opening backdash, bat/mist requests and death/stone
reactions. It preserves the original repeated close-range crouch-attack lock.
The owning encounter must still consume those controls through the original body,
form, weapon and event state machines. The service owns no runtime tables or global
state and does not use rendering or wall-clock timing.

Public synthetic tests check floor/wall/ceiling resolution, normal/transformed and
retained geometry, fractional movement, top-boundary handling, coordinate wrapping,
all encoded slope/one-way/mist tile types, malformed data and atomic failure. Decision
fixtures check exact controls/timers for opening, approach, throws, close attacks,
all eight immediate weapon-reaction rolls, bat/mist/stone/death reactions and invalid
state. Private native LLDB checks use BO4's actual arena floor, traversal and sensors
in Debug and macOS ASan/UBSan. Arena exits remain open without the future encounter's
door actors; those checks do not claim door containment or boss gameplay.

These are component fixtures, not an uninterrupted encounter, input replay against
PS1, or evidence that the stage is complete. All 56 public test groups and release
packaging pass locally. Original sensor/timeline/palette data remain private.

The services adapt `src/boss/dop_collision.h`, `dop_check_st_collision.h` and
`src/boss/bo4/doppleganger.c:DopplegangerThinking` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` under AGPL-3.0-or-later. Native ownership,
checked private-data access, explicit integer behavior and failure paths replace
original global/entity memory dependencies. Collision temporaries start at zero;
no uninitialized C reads are reproduced.

## Normal body controls

The normal-body service now consumes original PS1 input for stand/walk/crouch,
fall/jump/double-jump, backdash and attack selection. Private transition profiles
and animation tables retain original cancellation windows, landing decisions,
velocities, gravity and timer order. The service emits bounded original factory,
sound and attack-cleanup requests for the owning encounter. It does not resolve
the requested punch/kick hitboxes, knives, smoke or other child actors. Do not count
these attack requests or rendered poses as implemented combat.

Body, animation and collision state commit atomically. Public tests use synthetic
private-format profiles and cover turning, release/double-jump behavior, attack
selection/completion, knife limits, crouch transitions, backdash and failure paths.
A private native LLDB fixture runs 1,800 normal-control ticks and 450 body renders,
producing twelve hand-attack requests, six knife requests and 168 airborne ticks
under Debug and macOS ASan/UBSan. It supplies explicit control inputs and an arena
start; it is not an AI-driven boss fight or console comparison.

Bat/mist, damage/death and the owning encounter remain pending. Normal-body ticks
reject unsupported form requests and steps instead of silently substituting normal
movement. All 57 public test groups and release packaging pass locally. This slice
adapts `bo4/dop_anim.c`, normal steps in `unk_465F0.c`, `unk_46040.c`, and transition/
attack helpers in `unk_45648.c` at the pinned AGPL reference above. Original control
and animation tables remain private inputs.

## Child factories and hand hitboxes

BO4 now has a bounded original factory service and punch/kick child decoder.
Factories read the original 115 private six-byte blueprints and ten slot ranges;
controller slots 4..15 and child slots 16..79 retain original forward/reverse and
forced allocation, spawn delays, parameter increments, origins and pool-exhaustion
behavior. Dynamic child IDs use an internal `0x100 + original ID` namespace to
keep their update functions separate from ordinary stage actors. Generation-checked
parent links and primitive cleanup replace unsafe global entity pointers.

The hand child follows the body's original animation variant and pose, reads its
private per-pose frame/hitbox words, applies original attack power/poison scaling,
and stops at its terminal word or when the parent leaves the attack. It requests
the original swing sound at the original pose/timer. No hitbox or attack table bytes
are embedded. These internal services are not yet connected to ordinary stage
updates or the player's damage path; decoded boxes do not establish working combat.

Public fixtures cover burst cadence, allocation order, critical/noncritical full
pools, forced replacement/primitive release, moving origins, parent identity,
private hitbox selection, poison scaling, sound timing and malformed inputs.
A private native LLDB component sequence runs 1,800 body ticks, resolves thirty
factory requests and completes twenty-four hand children, with 222 ticks of active
hand boxes. Six smoke/knife children remain explicitly skipped. Debug and macOS
ASan/UBSan pass all 58 public groups and this private sequence; packaging passes.
This is not a live fight or a console replay.

Factory/hand behavior adapts `bo4/unk_49A6C.c` and
`bo4/unk_50D4C.c:DopplegangerUnarmedAttack`, plus `boss/dop_anim.h:UpdateUnarmedAnim`,
at the same pinned AGPL reference. Smoke, knife and transformation children, the
body's forms/damage/death, original stage events and connected progression remain
pending. Ordinary arena travel remains disabled.

## Thrown knife child

The knife child now initializes from the original private profile and retains its
eight one-pixel collision probes per tick, poison scaling, hitbox, throw/impact
sound requests, enemy-hit hold, bounce, rotation, fade and expiry. Its two owned
primitives use original artwork/palette/UV metadata and a two-color additive trail.
The native renderer now has bounded integer line interpolation with the existing
four RGB555 blend operations. Exact GPU endpoint registration and ordering still
require console comparison; this does not establish pixel parity.

Public synthetic checks cover flight, both directions, crouch/poison initialization,
collision/recoil, fade/expiry, hit hold, malformed timers/speed and line clipping,
interpolation, degenerate endpoints, blend arithmetic and atomic invalid-input
failure. All 58 public groups pass in Debug and macOS ASan/UBSan. A dedicated private
LLDB fixture uses an explicitly inserted collision wall and validates the original
knife texture separately from its trail, then runs one complete bounce/fade/expiry
with 69 primitive draws under both builds. Three impact-effect children are
explicitly skipped. The earlier hand fixture's skipped knives are covered only by
this separate component check, not by an ordinary encounter replay. Packaging passes.

Knife behavior adapts `bo4/unk_50D4C.c:EntitySubwpnKnife` at the pinned AGPL revision;
original line/color commands are described in `dra/4DA70.c`. The native line sampler
is a bounded presentation adapter, not a PS1 GPU implementation. Impact effects,
smoke, transformation children, body forms/damage/death, live player contact and
the original encounter/connected progression remain pending. Ordinary arena travel
stays disabled.
