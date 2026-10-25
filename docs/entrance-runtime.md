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
character, not the original new-game or post-prologue profile. Entrance saves
are not enabled by this slice. The native launcher still offers Laboratory.

NO3 is the first visit; NP3 (stage 7) is the later Entrance variant. Loading its
assets through the stage catalog does not implement its different enemy cast.
Mermen, Fire Wargs, Death's event/dialogue, route obstacles and connected-stage
progression remain subsequent work in this batch. Unsupported actors are logged
once by stage and original ID. Do not call Entrance complete.

The selected sound bank is `ST/NO3/SD_ZKNO3.VH`/`.VB`; exact size/SHA-1 metadata
identifies the local US dump without publishing sample bytes. Outdoor wind and
Dracula's Castle track constructors use private DRA intro/loop metadata. The
scene starts with wind; Death's music transition requires event integration.

Public synthetic tests cover profile/ID boundaries, bat proximity and flight,
zombie emergence/sinking/death and gated spawning, Warg attack/death transitions,
and invalid profiles. Private LLDB checks enter and draw eleven original rooms.
Those checks are explicit room starts, not an uninterrupted replay or parity proof.
Warg death retains its two-pass duration, but the original framebuffer-feedback
silhouette and recoil dust are not fully presented. Complete effects/audio,
original GPU ordering, drops and global timing/RNG parity remain incomplete.

Adapted behavior comes from AGPL-3.0-or-later `src/st/e_bat.h`, `e_zombie.h`,
`e_bone_scimitar.h`, `no3/e_warg.c`, `entity_lock_camera.h`, shared collection,
breakable, door, and explosion-puff services at reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. All arrays stay in private images.
The special US Bone Scimitar's full piece parameter originally indexes beyond
its rotation array into adjacent room bytes. The native path preserves that
serialized-image read with checked image bounds, without an out-of-bounds C
array access. Related actors use native slot/generation ownership.
