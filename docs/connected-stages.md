# Connected Entrance and Laboratory

The native executable now follows the private US room-teleport records between
first-visit Entrance (NO3, stage 65), Laboratory (NZ0, stage 12), and later Entrance
(NP3, stage 7). The upper first-visit Entrance exit enters Laboratory room 10;
returning through Laboratory's east connector enters NP3 room 15. The return uses
NP3's original placements and tables, rather than replaying the first-visit event.
The [early castle profiles](early-castle-stages.md) also connect Laboratory to
Marble Gallery and onward to Outer Wall. Area-specific gameplay remains incomplete;
other destinations fail explicitly as unsupported native gameplay.

## Ownership and progress

A connector latches one arrival and stops simulation catch-up until the app
consumes it. Held movement continues through arrival; held menu/action buttons
require release to prevent accidental activation in the new scene. The destination
validates its overlay, artwork, sound bank, room,
and arrival coordinates before replacing the active session. Character inventory,
equipment, resources, EXP, world flags (including Death), exploration, elapsed
time, and the four owned RNG states carry across. Destination transient actors,
projectiles, camera state, and sound voices are recreated. This is ordinary stage
travel, not serialization of an executing scene.

The existing verified music container transfers to the destination, avoiding a
full-container hash on every crossing. Candidate scenes borrow it until success;
failed initialization cannot close the previous owner's stream. Save-store
ownership transfers only after initialization. A failed load retains the old
owner and every slot, then shows an error in the native Continue shell.

[Version-five ordinary saves](native-saves.md) retain the stage alongside the
room. All three save rooms in each supported stage use the existing fifteen-slot
bank and confirmation flow. Continue reconstructs the recorded stage. Versions
1–4 explicitly migrate to Laboratory without rewriting the source file on read.

## Return Entrance scope

NP3 has its own actor-ID dispatch, initialization/animation/collection/camera/door
and sound-bank tables. Shared Mermen, bats, zombies, Bone Scimitars, and Bloody
Zombies now use those private tables. An ID that selects a bat in NO3 selects a
Bloody Zombie in NP3. Spawned children retain the destination's original IDs.
NP3 does not run the NO3 Death event.

Owl Knights/Owls, NP3's Slogra/Gaibon variants, Hammer/Gurkha/Blade, and several
obstacle/background actors remain unintegrated and warn by stage/ID. The original
new-game/prologue profile, all route mechanisms, room-load fades, whole-route
replay parity, full effects/audio, Maria/castle scripts, and further stages remain
incomplete. Connected loading does not establish complete castle progression or
full Entrance authenticity. The existing 70-HP development profile remains explicit.

## Validation and provenance

Public synthetic tests cover source/target ordering, final record bounds,
truncation, misaligned room offsets, invalid coordinates/stages, NP3 actor identity
and animation selection, and every supported stage/save-room combination. Frozen
independent v3/v4/v5 checksums protect migrations. No private data is in these tests.

A private native LLDB sequence walked from an explicit Entrance room 23 start
into Laboratory, returned to NP3, exercised catch-up latching, then saved/loaded
NP3 progress in an isolated local slot. It explicitly supplied Death's completed
flag and test gold to check retention; it was not an uninterrupted opening replay.
A missing-input destination check retained the session and save-store owners and
showed the error shell. Private sanitizer checks exercise the same sequence and
NP3's shared water enemies. Automated audio uses SDL's dummy device; these checks
do not claim real-device listening or full timing/audiovisual parity.

Room-teleport decoding and facing follow AGPL-3.0-or-later reconstructed
`src/dra/5087C.c:func_800F14CC/func_800F16D0` at revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. The 131 US records stay in private
DRA bytes. NP3 shares the documented Entrance/common enemy algorithms and
`src/st/e_bloody_zombie.h` at that revision; its runtime tables remain private.
Transactional session/storage ownership is native C code in this repository.

Selected normal-player [vertical room crossings](early-castle-review.md) now use
the original upward/downward overlap adjustments and early edge probes. A direct
world-grid coordinate rebase made the Laboratory entrance jump too low; room
rebasing now preserves velocity/fractions while applying the original offset.
Camera-edge locks and transformation-specific crossing rules remain separate work.

[Selected Doppleganger progression](doppleganger-progression.md) now applies the
original Outer Wall boss override before ordinary room selection. Victory, reward,
return travel and an ordinary save/Continue retain original event records in v7.
Other stages' boss overrides remain outside this selected integration boundary.
