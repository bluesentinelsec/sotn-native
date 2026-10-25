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
