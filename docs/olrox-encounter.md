# Olrox encounter integration

The BO0 development arena now advances both reconstructed forms: humanoid
movement/attacks, the original morph mesh, articulated monster movement and
head/arm requests, shared physical damage, independent hand breaks/recovery,
laser/fireball/drool attacks, captured-fragment defeat, Life Max Up creation and
arena release. A recorded original defeat time prevents the boss and doors from
respawning in a fresh arena with that retained progress.

This remains a selected native encounter. Complete flash/shake,
Alucard afterimage suppression, full effects/audio/reverb, connected NO2 travel
and uninterrupted original-console parity remain incomplete. The castle,
Alucard and game are not complete. BO0 is still an explicit development start.
Selected [breakable furnishings and sitting](olrox-furnishings.md) now run.

## State and ownership

The monster's body, feet, head, tail and knees form the original seven-part hit
ring. The head owns their shared HP. Arms and independently breakable hands do
not join that ring. Infinite-HP contact parts preserve the source's special
cooldown propagation; head hits propagate around the full ring, with the
original later-slot delay. Damage severity controls selected retreat decisions.

Reserved actors retain a validated slot, identity, generation and parent lineage.
Actor updates use copied working state, so ownership checks compare those values
rather than C addresses. Captured defeat packets survive their body's retirement;
only the same live generation receives body signals. Child readiness always
comes from the real controller. Room entry resets owned articulation/morph caches.

The final body image uses packet 16 in the original 17-packet allocation. The
first capture fragments and releases body phase 2; the body takes its slow steps
and requests the final capture. The second fragments, flash and independent rays
finish before the reward manager opens the doors. The first-form transition
alone never awards final victory. Version-nine world state already retains the
original defeat record; this work does not change its format or write user slots.

## Rendering and audio

Offscreen writes belong to each simulation tick. If presentation skips a frame,
the next game step renders pending owned target commands before advancing actors.
That preserves the morph's one-tick 127x256 capture and its later 128x128 mesh,
as well as laser targets, without adding a simulation or audio tick. Normal
presentation consumes the pending flag; target bounds and ownership checks stay
in force. This is selected software presentation, not a complete PS1 GPU.

A fresh BO0 texture owner starts with the original menu-cleared actor-atlas
rectangle: x 128..511, y 256..511. Original arena uploads leave part of that
rectangle untouched, and a late fire-effect frame samples its transparent zero.
Other unuploaded regions remain invalid. This supplies the documented fresh
baseline; general cross-stage VRAM history and console texture-cache parity are
not established.

Defeat requests the original 0x92 XA fade with multiplier 4. The later reward
flow stops any remaining stream and starts 0x317, Dance of Pales, from the private
track metadata. A newly requested track restores its stream gain after the fade.
Sound-effect events and XA commands remain separate. Automated runtime checks
use SDL's dummy audio driver; ordinary playback settings remain unchanged.

## Mandatory C adaptations

- The source's defeat burst can walk beyond the end of its 17-packet list. Native
  code consumes the same selected RNG requests and omits that optional visual
  burst instead of dereferencing a missing packet. It never replaces a boss or
  unrelated actor to obtain effect capacity.
- Original subdivision copies color control bytes over primitive metadata. Three
  midpoint control bytes are unwritten scratch memory. Native code initializes
  those tags to zero and retains the final corner's tag explicitly. Private
  comparisons use a zeroed scratch baseline; they do not prove arbitrary stale
  scratch behavior on a console.

## Validation

Public authored fixtures cover shared/independent ownership, infinite-HP cooldown
rules, copied actor updates, actual attack release waits, bounded movement,
capture release/final retirement, ray retirement, original reward selection,
saved defeat gating and the bounded cleared-atlas baseline. Unrelated missing
texture references still fail. Fixtures contain synthetic tables and artwork.

All 123 tests pass in Debug, Release and macOS ASan/UBSan; Release packaging
passes. Private Debug comparisons match 4,672 selected body-defeat snapshots,
11,200 captured-fragment snapshots and 6,400 ray/visible-packet snapshots. ASan/
UBSan emits byte-identical traces for all three. Body comparisons execute original
body, articulated helpers, geometry and factories with supplied child/body signals
and isolated optional burst allocation. Capture comparisons execute original
controller/subdivision/factories with authored rectangles and a supplied final
body signal. Ray comparisons execute original controller/primitive-helper
instructions with the pinned SDK/GTE reference. Initialization/allocation/audio
and retirement boundaries use explicit adapters. These are component comparisons.

A focused muted native Debug and sanitizer integration sequence advances the placed
introduction, three ordinary development Gladius hits, an explicit first-form
death-flag fixture, the real transformation, 160 second-form damage contacts,
physical defeat, reward conversion and door release. Both exit successfully
through LLDB. That fixture does not claim a naturally defeated first form or collected reward.

A second native sequence defeats both forms using ordinary development Gladius
attacks, follows the actual transformation/defeat, collects the Life Max Up
(original +5 maximum HP), checks restored area music and door release, and exits
successfully. It uses an explicit arena start and contact protection, with no
stat, equipment, relic or death-flag grants. The actual CMake Debug and ASan/UBSan executables
independently pass this sequence through muted LLDB at a seven-tick presentation
cadence and close through normal SDL quit without writing ordinary user slots.
A focused three-versus-seven-tick presentation comparison also matches checkpoint
image hashes, RNG streams, damage contacts and final simulation tick through the
fixture's reward conversion. These checks are not uninterrupted castle traversal
or original-console encounter parity.

Provenance: BO0 `313A8.c` monster/death routines, `e_olrox_drool.c`, selected
`e_laser_attack.c`, shared `st/collision.h`, `st/e_misc.h`, `st/e_life_up.h`,
`dra/sound.c`, `dra/menu.c` and `dra/d_CD80.c`, at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later. Original
profiles, sprites, textures, animations, SDK inputs and audio remain private.
Authoring disassembly, generated analysis, runtime traces and captures are not shipped.
