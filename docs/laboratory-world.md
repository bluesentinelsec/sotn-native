# Laboratory traversal milestone

The next native milestone is one complete Laboratory route with working room
mechanisms, the original camera/events, the paired boss encounter, and persistent
map/event progress. The executable and all tests belong to this repository.
Private runtime files remain the source of every animation, actor definition,
placement, graphic, and game table. No reference executable is a deliverable.

## Enemy runtime

Skeletons, Bone Scimitars, Blood Skeletons, Axe Knights, Spittle Bones, and their
selected projectiles/death effects now run alongside Bloody Zombies. Their
motion, attack selection, recovery, hitboxes, original sound requests, player
contact, defeat, and EXP path use the reconstructed US behaviors. Blood Skeletons
collapse and reassemble instead of disappearing permanently. Enemy projectiles
without the original kill-count flag do not increase the kill counter.

Stage sensors use the original floor/slope/ledge/wall routines. Spawned children
use bounded native slots and generation checks; a stale parent cannot be
followed after a room change or replacement. Each room owns a bounded primitive
pool for multipart deaths and acid splashes. Actor replacement, culling, and
room entry release those primitives. Native presentation rotates multipart
bones/axes and draws private death-piece artwork without advancing simulation.
Original affine rasterization/GPU ordering, color effects, drop probabilities,
elemental ailments, and global RNG parity still need separate validation.

Development rooms 10, 18, and 20 expose the added families. These are explicit
checkpoints with the existing development character, not original stage entry.
The original saved character/inventory format is unchanged by this enemy slice.

Public synthetic checks cover skeleton attack/bone creation, sword hitboxes,
collapse/reassembly, axe rotation/return/arc, attached-parent expiry, acid splash
ownership, primitive bounds, rotated/mirrored sprites, and corruption failures.
Private runtime inspection exercised all Laboratory rooms and over 37,000
multipart draws using the user's dump. This establishes exercised data/render
paths; it does not establish full area or audiovisual parity.

Adapted logic: `src/st/e_skeleton.h`, `e_bone_scimitar.h`,
`e_blood_skeleton.h`, `entity_axe_knight.h`, `st_common.h`, `e_misc.h`, and
`src/st/nz0/e_spittle_bone.c` at reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
The original field-sensor helper omits a return when horizontal speed is zero;
its native no-collision result is initialized explicitly. Parent ownership and
private-data bounds replace raw entity/data pointers.

## Remaining work in the authorized milestone

- Original doors, lifts, floor buttons/spikes, movable blocks, cannon and secret
  obstacles, and the associated mutable collision/tile state.
- Original camera constraints and room event ordering; Slogra/Gaibon coordination,
  projectiles, room sealing, victory/unlocking, reward, and music transitions.
- Map exploration and original permanent event flags retained by ordinary save/load,
  with explicit migration from existing native saves.
- A repeatable native traversal/boss/save/load run, sanitizer checks, platform CI,
  and a local LLDB playtest helper. Stacked PRs remain unmerged until playtested.

Other stages, complete new-game/prologue progression, and complete game parity
remain outside this Laboratory milestone. Unsupported paths must stay explicit.
