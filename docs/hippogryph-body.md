# Hippogryph main body: work in progress

The internal BO5 body now advances its original idle/activation, takeoff, hovering,
landing, distance-based attack choice, flame breath, ground run, lunge, grabbing
flight, object spawning, swoop, stagger and movement-particle states. Timelines,
choice tables, initialization and sensors remain private runtime inputs.

The original selected quirks remain intact: lunge does not decrement its local
timer, pose-ending transitions retain their original update order, and clearing
grab does not overwrite the previous raw damage value. A captured player follows
the boss's whole-pixel position while retaining its own fractions. The stage
adapter delivers the start, damage and clear commands to the normal-form player
service on the next simulation tick. The original visited event is index seven.

The auxiliary hurtbox routes damage and hit information to the matching owned
body. Both parts share hand/projectile cooldowns, including the original extra
tick for the later part. Hits through the auxiliary part suppress stun as in the
original; direct body hits share stun. Generation checks prevent a stale child
from damaging or following a replacement actor.

The original defeat now clears the grab, records the event, lands, draws its two-line
flash and 111-part ray/spark burst, fades and hands off to the reward/dialogue actors.
The shared reward service uses BO5's own initialization and private drop table to
produce its permanent Life Max Up. Optional effect allocation follows the original
fallbacks; actor-pool exhaustion retries reward birth, and fixed-slot ownership checks
protect unrelated actors. The original fractional velocity clear, asymmetric X tests,
separate-half reverse speed, retained timer and final opacity remain intact.

The original doors now seal/unseal their tile columns and move their owned children.
Their screen-relative closing limits and room-relative opening limits retain the
original fractions across camera offsets. The music manager waits for XA activity,
starts Death Ballad, and requests the original slow stop after defeat. The private
initial scene flag, graphics bank zero, sound-bank files and track metadata load
through verified BO5 resources; no game bytes enter public source or packages.

Maria's reward-watching manager now observes Life Max Up collection,
releases the encounter, walks/aligns Alucard, pans the camera, and restores control.
Her actor advances the original script-flag poses and exit animation/movement.
Generation checks prevent a replacement pickup from extending the earlier reward's
lifetime. The original dialogue/event interpreter now drives these actors, seven
BO5 speech cues and both flashback pictures, then records original flags 98/133 and
the completion word. Version-nine saves retain that word explicitly.

BO5's placed cloud and sky-land actors now run the shared original sky algorithms
with explicit BO5 initialization, UV, vertex, grid, profile and rotation offsets.
Their private resources retain their own stage identity. Sky packets render before
both room tile layers, preserving the original lower priorities; the same correction
keeps Chapel clouds behind its architecture.

The two flashback-picture actors now load their separate four-part coordinates, UVs
and palettes from BO5. They retain the original expansion, 192-tick hold and reverse
contraction, including alternating updates and the last part's extra vertical growth.
Their timer is explicitly 16-bit. Allocation pressure retries their initialization;
generation ownership releases their primitives on removal. The native skip path
removes the picture and releases its reservation. It does not reproduce the original
function's unused allocation after clearing its entity during a skip. This ownership
adaptation is not evidence of original allocation-pressure parity on that path.
Their original dialogue-driven birth now runs through the private event stream.

This branch is not a completed encounter. Connected Chapel
travel and full encounter/presentation parity still need verification. The BO5 profile remains disconnected, without a public
gameplay start. Do not merge or describe this checkpoint as a playable Hippogryph
milestone while those paths remain incomplete.

Public synthetic tests exercise activation/owned child creation, takeoff/flight,
landing, distance selection, breath pools, object initialization, lunge timing,
stagger, grab/pinning/drop, linked damage/cooldown/stun/generation handling and
invalid state/data/motion, defeat effects/pool pressure, reward conversion, arena tile/
door cycles, music waits and Maria poses/reward/control/camera flow. Private
component checks compare 12,160 ticks across 76 seeded sequences with the original US
main-body instructions. They execute the
original movement, animation, random, floor/sensor and particle-factory routines,
with isolated initialization, empty collision, audio, event-visit and player
acknowledgement adapters. The comparison includes spawned children and retained
grab values. It does not establish realistic tile/room traversal, full original
frame order, rendering/listening, a natural encounter or console parity.

A separate defeat comparison matches 36,000 original instruction ticks across 120
cases, including both facings, grabbed/free players, boundary positions, allocation
failures and a counterfactual timer rewind exercising the reverse-speed branch.
It compares parent/child state, all 111 effect records, geometry, fractional motion,
RNG, audio requests and event timing. It executes the original private SDK sine/cosine
routines with isolated allocation, initialization, empty-collision, audio and event
adapters. This is component evidence, not a rendered encounter or console replay.

A private native actor sequence also completes the projected reward and collects it
through normal contact handling, increasing maximum HP by five and releasing all
effect records. It uses verified BO5/DRA/SDK inputs with synthetic empty placements
and a flat floor; it does not load or overwrite ordinary saves.

Arena comparisons match 4,320 original instruction ticks across 24 cases, including
camera offsets, owned door births, tile changes and controlled audio-busy input.
Maria comparisons match 16,672 ticks across 168 seeded sequences, executing original
animation, movement, state and camera helpers with external reward/script/player
fixtures. These do not establish complete dialogue or original engine frame order.

A private native scene check runs 1,400 ticks and 350 renders through this project's
game/session code, with original placed body/doors, activation/music, selected attacks,
defeat effects, reward birth and XA stop. The player is explicitly protected and
defeat is forced for coverage. This check exposed shrinking particles with negative
signed scales; the native renderer now preserves their reflections and retains
bounds validation. Synthetic tests cover both reflected axes for multipart and packed
sprites. Captures and all runtime inputs remain private. The check is not an ordinary
encounter, full listening test, uninterrupted replay or console parity claim.

Provenance: US BO5 `func_us_801A1C14`, `func_us_801A19FC`, `CheckHippogryphFloorCollision`,
shared `src/st/e_life_up.h` and the linked-parent paths in `src/st/collision.h`, at
sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Only adapted native C algorithms and table metadata are public; instruction traces,
runtime tables, game artwork and comparison inputs remain private.

Arena/Maria provenance: BO5 `EntityRbo3Door`, `func_us_801A4494`, `func_us_801A5F88`,
`func_us_801A62B4` and `CutsceneCameraPan` at the same pinned revision. Music/XA
selection follows the pinned `src/dra/sound.c` and `lba_stage.c` metadata interpretation.
Sky provenance: shared `st/e_clouds.h` and `st/e_background_sky_land.h`, with separate
private BO5 metadata; see [sky validation](chapel-sky.md).
Flashback provenance: BO5 `func_us_801A66B0`, `func_us_801A666C`,
`func_801B1D68` and `BO5_RicSetDeadPrologue` in `e_richter_prologue.c` at the
same revision. These last two actor names select pictures; they do not run the
Richter player's death routine.

Flashback comparisons match 33,744 original instruction ticks across 240 seeded
cases in Debug and macOS ASan/UBSan. They exercise both picture selectors, every
normal state, 16-bit timer boundaries and delayed allocation, executing the
original geometry helper with isolated allocation/initialization/destruction.
A separate native LLDB sequence runs 580 ticks and 145 renders, explicitly spawning
both pictures. Each visibly contributes to the frame and releases after its
contraction. Both private captures were inspected. This protected, explicit-spawn
fixture and its dummy audio do not establish dialogue-driven timing, a natural
encounter, listening or console parity. All 99 test groups pass in both configurations.

The BO5 dialogue adapter keeps the eight original states, private 72-line opening
and color transition, portrait timing, five-row text cache/scroll, encoded nameplates,
24-tick closing, event wait/timer rewind and next-tick completion. Portrait and text
words load before their owned packets draw. Dialogue priorities 509–511 cover the
HUD's original 494–496 priorities. The initial seven owned packets remain seven;
the original helper's null-linked border state is represented as bounded shadow
fields, without dereferencing a host null pointer or drawing an extra unowned tile.
This is a selected native adaptation, not full PS1 GPU or allocation-pressure parity.
Scripts, glyphs, names, portraits, line speeds, selectors and speech stay private.

Public synthetic tests cover state timing, event-spawn coordinates and blocked timer
rewind, speech ranges, window/portrait/name/text resources, skip eligibility, invalid
selectors/data and allocation failures. Save tests retain frozen v1–v8 migrations
and independently checked v9 checksums. Actual integration evidence and remaining
parity checks must accompany the connected milestone; component tests alone do not
establish a complete ordinary encounter or authentic-game release.

Private BO5 dialogue comparisons match 4,946 original instruction ticks across
three runs of the actual private script: ordinary execution, legal skip and delayed
external actor flags. They compare script/event cursors, control/progress flags,
timers and active packet geometry, colors, UVs, palettes and priorities. Original
script/event/window/portrait/scroll/name helpers execute with isolated allocation,
graphics and audio calls and controlled actor/audio-busy input. This is component
evidence, not complete GPU, allocation-pressure, audio or original frame-order parity.

A private native LLDB sequence completes 3,527 ticks and 882 rendered frames in
Debug and macOS ASan/UBSan. It forces boss defeat with a protected player, then
collects Life Max Up through normal contact and lets the original event stream
run Maria, seven speech cues, both pictures and camera/control restoration. Captures
exposed and verified the HUD/nameplate ordering correction. A retained-completion
skip fixture also stops active speech and returns control; first-time skipping is
not granted. Independent decoding of all seven private speech clips produces
nonzero PCM; listening and console PCM parity remain unproven.

An explicit private transfer to a Chapel save room exercises its ordinary save UI,
fresh byte-identical v9 restore, in-session completion-mask unions and failed-load
owner retention. Selected-slot story flags, clear status, gold and boss records
come from the loaded slot. Real user slots remain untouched. This transfer is a
fixture, not connected traversal. The existing Chapel Maria scene also passes its
ASan/UBSan speech, save and Continue regression. All 100 public test groups pass
in Debug and macOS ASan/UBSan; the final speech-stop change has targeted tests in
both configurations. Connected travel, a natural boss encounter, whole-route
replay and full audiovisual/timing parity remain incomplete.
