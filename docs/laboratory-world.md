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

## Room mechanisms

The room service owns a writable copy of foreground tile IDs. The player,
projectiles, actors, and tile renderer borrow that same copy, so opening an
obstacle changes both its appearance and collision. Definition/UV/animation
and patch tables remain in the user's private images. Room replacement validates
and allocates its tile storage before replacing the previous owner.

Normal floor buttons toggle their adjacent spikes. The movable-box puzzle has
its two switches, rising/retracting spikes, and pushable box. The Blood Skeleton
button operates the short lift; the main lift follows the three original stops
and Up/Down controls or call buttons. Original object sensor extents resolve
standing, ceiling, and wall contact after the player update. A moving platform
carries the player without resetting movement/animation.

The cannon lever fires its shot, opens the shortcut, and retains its permanent
flag within the session. Secret wall/floor obstacles require the original three
hits and reset interval, replace their private tile patches, and retain their
open flags across room reentry. Selected debris and palette-tagged explosions
run; complete intense explosions, camera shake, secret-map reveal effects,
and original GPU ordering/shading still need integration or parity checks.

Red doors change their collision tiles, animate opening/closing, and own short
simulated-pad/protection intervals for automatic traversal. The blue door stays
locked without the Jewel of Open, which the current character checkpoint cannot
obtain; its original message box is not yet presented (the condition is logged).
Other stages remain unsupported beyond their transition boundary.

Public synthetic checks exercise contact direction, bounded shoves/patches,
immutable source ownership, switch/spike toggling, lift carry/stops, cannon
activation/opening, three-hit secret persistence, door opening/crossing states,
and packed exploration validation. Native private-data checks exercised the
rendered cannon, switches, box puzzle, and lift with sound. These checks prove
those exercised paths, not full timing or audiovisual parity.

The cannon's original comparison-of-a-boolean is preserved. The box puzzle's
one-element-before-array read is replaced by an initialized zero sentinel;
this required memory-safety adaptation is explicit rather than an unchecked
native read. Original sensor math uses widened, bounded native intermediates.

Adapted logic: `src/st/nz0/e_nz0_room2.c`, `e_puzzle.c`, `e_elevator.c`,
`e_elevator2.c`, `e_cannon.c`, `e_left_secret_room_wall.c`, and shared
`st_common.h`, `e_red_door.h`, `e_sealed_door.h`, `e_collect.h`, plus packed
map visitation from `src/dra/5087C.c:func_800F2014`, at the same pinned revision
and AGPL-3.0-or-later license above. Progress is session-owned here; ordinary
save format remains v3 until the world-persistence slice lands.

## Camera integration

The normal camera now advances once during simulation, independently of draws.
It preserves the original horizontal anchor, vertical 136-pixel anchor, four-pixel
upward catch-up, and top/bottom margins. Camera-lock actors read their private
hitbox/axis/property tables, change bounds on player crossings, and settle over
the original sixteen-tick interval. Scene-owned constraints remain bounded to the
current room. The standard player/entity drawing center uses the original 0x94.

Synthetic tests cover follow direction, immediate follow, margins, invalid bounds,
lock crossing/settling, and table rejection. A private comparison matched 200,000
normal follow cases against the reconstructed camera block at the pinned
revision (`src/dra/5087C.c:func_800F0CD8`). Lock logic follows
`src/st/e_lock_camera.h`; its tables remain private. These do not yet prove room
loading/fade, scripted event, background-animation, or full camera parity.

## Paired boss encounter

The original room manager now seals the Laboratory encounter, starts Slogra and
Gaibon in their reserved slots, records the visited/defeated event, reopens its
blocks, and requests the original boss/area music transitions. Slogra walks,
pokes/fires his spear, reacts to hits, loses his spear at low health, and joins
Gaibon's pickup/plunge attack. Gaibon flies, fires small/large projectiles,
lands/shoots, transforms at low health, and completes his death sequence. Their
physical hitboxes, damage/death, EXP, and supported sound requests use the shared
combat/character services. Attached spear/leg entities validate their parent generation.

The victory effect retains the original radius/angle/scale updates and its
conversion to the original Life Max Up. Each of the original drawable/data pairs
uses one native record: 193 owned primitives represent 385 PS1 records. Exhaustion
retains the original fallback that still produces the gameplay drop. Negative
scales and reversed projected corners are supported. Completion and room replacement
release every owned primitive. Projectile scaling, rotation, fading, opaque puffs,
and reward tinting draw from the user's original artwork/palettes.

The quantized SDK angle table remains in the user's verified boot executable.
Private comparisons matched 200,000 bounded directions and 20,000 reward projection
cases against the pinned reconstructed implementations. Public synthetic tests cover
the paired state transitions, pickup/plunge, low-health phases, projectiles, defeat,
room unlocking, reward/drop ownership, parent expiry/bounds, scale/color rendering,
and boss music decoding. A native LLDB run exercised both kills, the music changes,
and reward collection, increasing maximum HP by five. That integration run used
an explicit private high-health/strength test profile; ordinary starts retain the
existing development character.

Development room 16 enters the boss room from its safe left entrance. Room 14
exposes secret obstacles. These remain checkpoints, not original new-game starts.
Temporary destruction bits reset on room entry, matching the original; they must
never suppress unrelated rooms' actors or enter ordinary saves. Permanent progress
belongs to character/world state instead.

Whole-encounter input/replay parity, full GTE/GPU effects/ordering, camera shake,
reverb/positional mixing, complete status ailments, Maria/castle scripts, room-load
fades, and other stages remain incomplete. The checkpoint is not a complete area
or full-game parity claim.

## Remaining work in the authorized milestone

- Map exploration and original permanent event flags retained by ordinary save/load,
  with explicit migration from existing native saves.
- A repeatable native traversal/boss/save/load run, sanitizer checks, platform CI,
  and a local LLDB playtest helper. Stacked PRs remain unmerged until playtested.

Other stages, complete new-game/prologue progression, and complete game parity
remain outside this Laboratory milestone. Unsupported paths must stay explicit.
