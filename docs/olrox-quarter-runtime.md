# Olrox's Quarters foundation

NO2, original stage 4, now has an internal native resource profile and selected
shared actor adapters. Ordinary castle travel remains disabled until its remaining
enemies, mechanisms and Olrox encounter are integrated. This is not a completed
area, a new public gameplay start, or proof of authentic full-game progression.

The profile keeps its own actor identities, initialization, animation, palette,
prize, door, camera and sine tables. It includes common pickups and red doors,
selected camera locks and dust, all ten retained breakable profiles and owned
debris, foreground/background decorations, Skelerang and owned boomerangs, and
selected blood drips/splatter. The existing selected Bloody Zombie adapter also
uses NO2's tables. Ordinary NO2 room layouts place Skelerangs and seven Bloody
Zombie bodies. Zombie identity 0x2D is distinct from blood 0x2E/0x2F and the Owl
Knight sword/target 0x2B/0x2C; registry tests protect that boundary. Blood children
have controlled component coverage; complete zombie blood factories remain incomplete.
Shared adapters retain their documented limitations; this is not new parity proof
for every imported behavior.

NO2 candelabra use their own hit sound and debris palette. The boomerangs read
NO2's original private 256-entry sine profile instead of substituting host math.
Common prize/explosion offsets identify their actual tables inside `e_collect`,
not the beginning of that data section. A native check caught that distinction:
the initial incorrect offset read unrelated bytes as explosion velocity. The
corrected mapping has a regression test. Explosion motion also checks signed
fixed-point addition before writing, and actor bounds errors identify the stage,
actor, slot, step and position.

## Private resources and storage

The existing canonical stage catalog supplies `NO2.BIN` and `F_NO2.BIN`. Two
additional on-demand resources fingerprint `ST/NO2/SD_ZKNO2.VH` and
`ST/NO2/SD_ZKNO2.VB`. These files and their stage counterparts were compared
byte-for-byte with the user's own disc before preparing ignored runtime copies.
Public source contains only metadata and adapted C algorithms, with no game data,
artwork or sound. Public builds, packaging and CI require no private inputs.

The music service selects the original Dance of Pales intro and loop. Default
graphics bank zero loads independently of room-specific graphics. Selected actors
read their own tables and preserve the existing checked child/primitive ownership.
The ordinary save-room identity uses the existing v9 format and fifteen-slot bank;
v1–v8 migrations and failed-write protection remain unchanged. No transient actor
state or executing-scene save state is added.

## Validation and remaining work

All 101 public test groups pass in Debug, Release and macOS ASan/UBSan;
`make package` produces the self-contained macOS Release ZIP. Synthetic tests
cover the bounded profile, both decoration kinds, all ten breakable/drop/debris
paths, invalid types/indices and absent Cube of Zoe, separate sound/palette,
owned Skelerang identities and private sine samples, zombie animations/registry boundaries,
blood streams/drips, explosion offsets/overflow, music selection and ordinary
save-stage round trips. These fixtures contain no game tables or assets.

Private LLDB sequences pass in Debug and macOS ASan/UBSan, rendering 297 frames.
They explicitly start in NO2 rooms, exercise all ten retained breakable types
with injected hit signals, then use an ordinary 70-HP development character to
attack and collect a placed room-15 lamp at tick 104 after its original door
arrival. A protected, explicit room-zero start observes placed Skelerangs and
thrown boomerangs for 1,000 simulation ticks. Two private captures were inspected.
These are controlled components, not uninterrupted traversal or ordinary combat
balance/console-parity evidence. Remaining actors still warn as unintegrated.

A separate native LLDB sequence verifies an original placed zombie's identity,
artwork and 24 HP, then deals damage with Gladius, completes death and awards
27 EXP at tick 290. Explicitly spawned blood drips/splatter then render and finish
their owned lifetimes. Both Debug and macOS ASan/UBSan pass with 116 rendered
frames. This explicit-start fixture protects the player without stat/equipment
grants; it does not force enemy damage/death, exercise the missing original blood
factories or establish ordinary encounter/console parity.

An independent 80-tick Dance of Pales decode produces nonzero PCM. An explicit
transfer to the NO2 save room exercises its ordinary UI, isolated slot 15 and
fresh byte-identical v9 restoration. The transfer is not walking evidence.
Existing user slots remain untouched. Dummy SDL audio validates decoding and
ownership, not real-device listening, reverb or console PCM parity.

[Prisoners and selected spike behavior](olrox-prisoners-spikes.md) now extend this
foundation with owned shadows, original phases, temporary environmental contact
and equipped tile destruction. Selected [Valhalla Knights](olrox-valhalla.md) add
charge/leap combat, linked damage, independent weapon contact and death pieces.
[Hammer and Blade](olrox-giant-knights.md) now add placed articulated combat and
staged deaths. Ordinary NO2 layouts do not place the compiled Owl/Owl Knight or
Gurkha families. NO2’s [Spectral Sword](olrox-spectral-sword.md) now adds its own
fifteen-weapon encounter. The [secret wall and ceiling](olrox-secrets.md) now
retain attacks, collision patches, selected debris and discovery across saves.
[Selected stone doors](olrox-stone-doors.md) now add the original forced walking,
collision columns and shaded panels. The [courtyard scenery](olrox-courtyard.md)
now adds house models, backgrounds, lamps and glow. Its framebuffer effect and
Olrox remain outside these batches. Other abilities, complete effects/audio, connected area progression
and whole-route timing/console comparisons remain incomplete. Keep the connected
profile disabled until that progression has meaningful native validation.

Provenance: shared `st/e_skelerang.h`, `st/e_bloody_zombie.h`,
`st/e_breakable_with_debris.h`, `st/e_collect.h`, `st/entity_lock_camera.h`,
`st/e_room_bg.h`, `st/e_room_fg.h`, `st/e_misc.h` and NO2-specific identities at
pinned sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`
(AGPL-3.0-or-later). All original runtime arrays remain private.
