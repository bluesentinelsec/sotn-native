# Castle Entrance integration

Castle Entrance is being integrated into this repository's C17/SDL3 executable.
The current first slice opens original stage 65 (NO3), reads verified private
room/placement/artwork tables, and runs bats, floor zombies/spawners, Wargs with
separate head hitboxes, and Bone Scimitars including their special patrol/drop
path. Shared candles, pickups, explosions, red doors, and camera locks use
Entrance tables. The Laboratory dispatcher remains isolated: matching numeric
actor IDs never select behavior from another stage.

Run the development checkpoint through LLDB:

```sh
make debug
lldb -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data local/game-data/us --entrance --entrance-room 2
```

Use `run`, then `thread backtrace all` if the debugger stops on a crash.
The room option accepts 0..27. This is the existing explicit development
character, not the original new-game or post-prologue profile. Entrance save rooms
now write stage-aware ordinary saves. The native launcher still offers Laboratory
as its fresh checkpoint; Continue loads either supported area.

NO3 is the first visit; NP3 (stage 7) is the later Entrance variant. The [connected stage service](connected-stages.md) loads it on return from
Laboratory with its own placements and shared-enemy tables. Its additional enemy
cast remains incomplete.
Both Merman variants now swim, jump, walk, lunge/backflip, attack, take damage,
and die; the water-room spawner, fireballs, jump trails and selected splash/death
effects run. [Death's selected opening event](entrance-death.md) now runs with original speech,
portraits, animations and targeted theft. Connected travel now follows the original
Entrance/Laboratory route; complete route obstacles remain incomplete. Fire Wargs have no placements
on this selected first-visit route and remain unsupported. Unsupported actors are logged
once by stage and original ID. Do not call Entrance complete.

The selected sound bank is `ST/NO3/SD_ZKNO3.VH`/`.VB`; exact size/SHA-1 metadata
identifies the local US dump without publishing sample bytes. Outdoor wind and
Dracula's Castle track constructors use private DRA intro/loop metadata. The
scene selects wind/quiet wind/castle music and performs Death's music transition.

Public synthetic tests cover profile/ID boundaries, bat proximity and flight,
zombie emergence/sinking/death and gated spawning, Warg attack/death transitions,
Merman projectile/recoil/spawner paths, splash ownership, invalid variants,
and invalid profiles. Private LLDB checks enter and draw eleven original rooms.
A separate 1,800-tick private water-room check used a 999-HP test fixture and
exercised both Mermen, attacks, damage, projectiles, and selected effects; it is
not an ordinary-profile playtest.
Those checks are explicit room starts, not an uninterrupted replay or parity proof.
Warg death retains its two-pass duration, but the original framebuffer-feedback
silhouette and recoil dust are not fully presented. Complete effects/audio,
original GPU ordering, drops and global timing/RNG parity remain incomplete.

Adapted behavior comes from AGPL-3.0-or-later `src/st/e_bat.h`, `e_zombie.h`,
`e_bone_scimitar.h`, `no3/e_warg.c`, `e_merman.h`, `e_merman2.h`, `e_misc.h`,
`entity_lock_camera.h`, shared collection,
breakable, door, and explosion-puff services at reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. All arrays stay in private images.
The special US Bone Scimitar's full piece parameter originally indexes beyond
its rotation array into adjacent room bytes. The native path preserves that
serialized-image read with checked image bounds, without an out-of-bounds C
array access. Related actors use native slot/generation ownership.

Merman death fragments, blue jump quads, and spray use the bounded native
primitive service. Exact PS1 primitive deformation, priority and color feedback
(including the second Merman's death quad and fire-spit flash) remain audiovisual
parity work. No game tables, sprites, or sound samples are embedded.

The default room-zero development entry now uses the private original gate-arrival
record, rather than a generic upper-floor position. It starts inside the castle;
the forest opening script remains separate. Fresh room entries retain opening gear
in the forest/bridge and pre-Death side rooms; live progression uses actual event flags.

The [water-room rocks](entrance-rocks.md) now break after original weapon strikes,
retain both broken sides and update both tile layers. Their later transformation
ritual remains gated by unavailable wolf/bat abilities.

The [breakable stair section](entrance-stairway.md) before Death now takes ordinary
weapon damage, opens its tiles, drops its food and restores its saved broken state.
