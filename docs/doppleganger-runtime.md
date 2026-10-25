# Doppleganger runtime foundation

BO4 now connects the original boss AI, body, selected child actors, combat and
physical defeat through native stage updates. A private integration fixture
explicitly inserts the root and exercises the fight; the original entry sequence,
seals, reward, stage record and ordinary travel remain pending. The arena stays
disconnected. Earlier standalone component checks below establish narrower coverage
than the current integrated fixture described at the end.

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
The native encounter consumes those controls through the selected original body,
form and weapon services; the original event state machines remain pending. The service owns no runtime tables or global
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
sound and attack-cleanup requests for the owning encounter. Its requests resolve through the child services below. Attack requests and rendered
poses alone do not prove contact damage.

Body, animation and collision state commit atomically. Public tests use synthetic
private-format profiles and cover turning, release/double-jump behavior, attack
selection/completion, knife limits, crouch transitions, backdash and failure paths.
A private native LLDB fixture runs 1,800 normal-control ticks and 450 body renders,
producing twelve hand-attack requests, six knife requests and 168 airborne ticks
under Debug and macOS ASan/UBSan. It supplies explicit control inputs and an arena
start; it is not an AI-driven boss fight or console comparison.

The damage/physical-defeat and stage-orchestration sections below extend this
service. Body ticks reject unsupported steps rather than substitute normal movement. All 57 public test groups and release packaging pass locally. This slice
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
are embedded. These services now connect to BO4 stage updates and the player's damage path when
the root is explicitly instantiated. The earlier standalone decoded-box check alone
does not establish combat.

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
at the same pinned AGPL reference. Additional child types, complete effects, original stage events and connected
progression remain pending. Ordinary arena travel remains disabled.

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
is a bounded presentation adapter, not a PS1 GPU implementation. The orchestration below extends knife contact and impact effects. Complete effects,
the original entry event and connected progression remain pending. Ordinary arena travel
stays disabled.

## Transformation controls and blink child

The body service now advances the original bat, mist-control and return-to-human
steps. Bat steering, wing-smash inputs/speed/timer, sensor expansion on return,
pose replacement and per-pose horizontal adjustments retain simulation timing.
The blink child owns eight textured strips or fan triangles and drives the original
17-tick ramp / 17-tick fade handshakes. Private packed artwork is uploaded to a
bounded native display cache, preserving retained-pose requests. Gouraud vertex
colors and original quantized SDK trigonometry, direction and square-root lookups
supply the selected effect geometry without embedding game or SDK tables.

Public regression cases cover both form controls, blink handshakes/expiry,
duplicate and stale-parent cleanup, missing profiles, mutable pose records,
per-pose bat pivots, retained display pixels, vertex-color interpolation and math
bounds. A private native LLDB component sequence drives the actual blink children
through bat transformation, steering, wing smash and return: 45 active bat ticks,
146 blink-child ticks and 240 native renders. Four palette/effect/other requests
remain explicitly skipped. Debug and macOS ASan/UBSan run the same component check;
all 58 public groups and release packaging pass locally.

Mist controls currently require an owning mist child's phase signals; its actual
child and palette animation service remain pending. The public mist fixture supplies
those signals explicitly. The orchestration below adds wing trails/smoke and physical hit/death integration.
Additional elemental/stone effect children remain pending,
and native rasterization has not been compared pixel-for-pixel with console output.
Ordinary arena travel remains disabled; this is not a complete boss fight.

Controls/blink adapt `bo4/unk_465F0.c`, `unk_49A6C.c:EntityDopplegangerBlinkWhite`
and the selected quad transform in `dra/4B758.c:func_800EB758` at the pinned AGPL
revision. SDK sine/cosine reconstruction uses the pinned MIT `geo_00.c`/`geo_01.c`;
`game_sqrt.c` adapts PSY-Z's MPL-2.0 `libgte_sw_sqrt.c:SquareRoot12_impl` at
`b52003e55c0d1b20719e984dd19bd4d760eeb14a`. Notices ship with the app.

## Hit reactions and physical defeat

The body now consumes externally resolved hit metadata through an atomic update
interface. Small hits retain the original stand/crouch/air reactions; losses of
at least ten HP launch the boss using the original knockback, RNG draws, animation
selection, impact sounds and palette timers. Death preserves its own timer order,
physical launch, color change, terminal-pose retention and 64-tick nibble-erasure
pattern. A simulation-owned bit mask describes the original 64x64 scratch-image
clears; it contains no artwork. Presentation captures the actual final private pose
before applying that mask, even when intervening renders were skipped.

Synthetic tests cover threshold/direction differences, stand/crouch recovery,
RNG ownership and atomic failure, invulnerability timing, physical death, all
4,096 erased nibbles, terminal timing, cold display-cache capture, partial/full
masks, color multipliers and malformed inputs. All 58 public groups pass in Debug
and macOS ASan/UBSan; release packaging passes. A private LLDB sequence resolves
two explicitly supplied physical hits and death over 407 native renders. Its
mid-dissolve frame matches a 26-render version at the same simulation tick; the
ASan/UBSan run produces the same hash. Seven child/effect/profile requests are
explicitly skipped. These are component checks, not an ordinary boss fight.

Selected elemental hit/death branches emit their original requests and shading,
but complete elemental children/effects remain unintegrated and lack private
encounter validation. Stone status/shatter death remains rejected. The owning
encounter must supply the original stage hit-effect selector, resolved HP loss,
libc RNG and child services. The orchestration below adds selected live player contact. Intro, reward and
ordinary arena progression remain pending. This slice adapts `bo4/unk_465F0.c` damage/reset/kill
logic and `bo4/doppleganger.c` timer/status ordering at the pinned AGPL revision.

## Stage combat orchestration

The native BO4 root now initializes original attributes, priority, sensors,
64 libc RNG draws/debris vectors and opening backdash, then advances the original
AI/body and ascending child slots through the stage service. Contact damage,
weapon/subweapon damage, HP-loss reactions, invulnerability, child culling and
physical defeat share the ordinary player/actor path. Hit effects read the private
stage selector tables in original priority order; raw element bit positions are
not reaction indices. Player snapshots expose the selected original crouch,
airborne, hit, dead and stone status signals used by the boss AI.

Smoke reads private motion/scale/sensor profiles and its original timeline.
Wing-smash trails shrink/fade and a separate original child carries its hitbox.
Impact/death particles retain private profiles, libc draws, fixed-point motion,
color wrapping, delays, double-decrement timer cases and original contiguous
allocation. The native pool now provides 1,024 allocatable primitive records, with
16-bit per-actor counts bounded to 384 for the upcoming original intro effect.
Public tests distinguish fragmented from consecutive allocation and check large
counts/cleanup. This does not model every original engine/UI primitive reservation.

BO4's stage-start upload now loads default graphics bank zero even though its
room has no additional object-graphics bank. The integrated run found the missing
hand-effect texture; a regression verifies the initial upload and failure cleanup.
Untextured particle tiles use the shared four RGB555 blend modes. Graphics errors
identify the actor and missing texture/palette coordinate.

All 59 public groups pass in Debug and macOS ASan/UBSan, and packaging passes.
A private LLDB stage sequence inserts the root explicitly, grants Alucard 999 HP
and STR 30, and completes physical defeat at tick 3,466. It records five actual
boss-hit ticks, 169 net player HP lost, 120 bat-control ticks, 7,720 child ticks
and 867 native renders. The child activity includes smoke, knives, impact/death
particles, blink, wing trails/boxes and hand attacks, without skipping requested
children. Debug and ASan/UBSan run this same sequence. This is not an ordinary
new-game route, an unmodified-character playtest, intro/reward validation or a
console replay.

Mist/palette service, high jump, stone and full elemental effects remain pending.
Unsupported child requests fail explicitly. The [entry/reward service](doppleganger-entry.md)
now adds the placed manager, plates/light/shock, block doors, music transitions and
selected Life Max Up. Durable arena records and ordinary transfer selection remain
the next boundary. The stage profile stays disconnected until that boundary works.
