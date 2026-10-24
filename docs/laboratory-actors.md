# Laboratory actors

## Placement and scrolling foundation

The core decodes actor placements directly from the user's verified NZ0 overlay.
The supported US profile has 53 horizontal and 53 vertical lists, addressed
through the overlay header at offset 28. Ten-byte records contain room-local
X/Y, packed entity ID/mode/variant, packed slot/persistence, and parameters.
No placement tables or generated game arrays are compiled into this project.

`SotnActorLayout` borrows immutable overlay bytes. Opening checks both lists,
pointer alignment and bounds, start/end sentinels, ordering, supported modes,
entity IDs, stage-slot bounds, and matching multisets of placements. Each list
has a 512-record budget. Weighted layouts fail explicitly; all 53 layouts in
the selected US Laboratory are static. Close metadata before the backing data.

`SotnRoomActors` owns 192 stage-relative slots, bounded directional cursors,
and stage-session persistence flags. It borrows one validated layout at a time.
Entry uses the original X/Y visibility margins; scrolling traverses the original
sorted lists with their inclusive edge and direction-reversal rules. Normal
placements retain occupied slots; forced placements replace them. A spawn
generation lets behavior code distinguish a replacement from an existing actor.
Releasing an actor inside a stationary view does not make it respawn every tick.
Persistent destruction suppresses its original flag across room entries.

This component supplies placement/lifecycle state. Behavior and rendering use
these slots in the native scene described below. It does not itself implement
combat or saved-game persistence. Current movement limitations still apply;
see [movement integration](laboratory-movement.md).

Inspect actual private actor data through the project's executable:

```sh
make && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data /path/to/private/extracted/us --inspect-laboratory
```

## Provenance and validation

The original format parser follows the pinned `LayoutEntity` format and NZ0
layout-header extent. The scrolling/persistence implementation adapts
`src/st/create_entity.h` and `src/st/st_common.h` at reconstructed C revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, retaining AGPL-3.0-or-later notices.
No PS1 pointers become host callbacks and no SDK headers are imported.

Public synthetic tests cover paired layouts, sorting and bounds failures,
truncation, empty layouts, inclusive spawn edges, direction reversals, vertical
scrolling, occupied/forced/ignored slots, stationary-view removal, stage
persistence, and invalid camera/output requests. No private inputs are needed.

A local comparison against the unchanged pinned spawn routines matched
108,544 scrolling updates across all 53 private layouts, including reversals,
simultaneous X/Y changes, slot removals, and persistence. This establishes the
tested placement behavior, not actor/gameplay parity. Private inputs, reference
test tools, and their generated files remain ignored.

## Actor graphics dependency

`SotnActorGraphics` owns a bounded 1024x512-word texture-memory arena and tracks
which words have been loaded. It reads initial stage/common textures and common
palettes from the existing verified graphics resources. Original bulk palette
copies and room graphics-bank uploads come from bounded NZ0 views. Raw 4/8/16-bit
and dictionary/RLE transfers validate and decode into scratch memory before
changing texture memory. Unloaded texture or palette references fail explicitly.

Multipart frames resolve through runtime common/stage sprite banks. The renderer
preserves part pivots, mirroring, cropping, packed texture addressing, palette
selection, transparent versus STP black, and semi-transparency arithmetic.
It renders reverse part order within their original relative priority groups.
The shared OpenGL presenter displays the completed RGB555 frame. This is not a
complete GPU-command renderer: packed Alucard sheets use their existing service;
primitive transforms, full cross-entity ordering, and animated palettes remain
separate work. The graphics dependency alone does not enable actors in the demo.

The adapted implementation in `modules/render/actor_graphics.c` identifies
`src/dra/4AEA4.c`, `4A538.c`, and `4B758.c` at the same pinned revision and retains
AGPL-3.0-or-later attribution. Public synthetic tests cover raw/compressed uploads,
4/8/16-bit texels, palettes, mirroring, clipping, STP blending, part ordering,
truncated/unsupported references, and atomic failures. A local private check
successfully loads all 25 Laboratory graphics banks and draws normal candle,
skeleton, and zombie frames. No source or package contains those textures.

## Native actor checkpoint

`--laboratory` now runs selected original actor behaviors on the same fixed
simulation ticks as the player. The shared room camera drives original spawn
and culling margins; drawing does not advance actors. It initializes sprite
attributes and lifetime flags from runtime EInit and enemy-definition records,
reads byte-pair animation streams directly from NZ0, and decodes graphics as
rooms change. Normal and forced slot generations reset behavior state correctly.

Supported families are candles (intact normal animation), red-eye busts, intact
globe tables, and Bloody Zombie initialization, landing, walking, turning,
chasing, and attack animation. Zombie movement uses original floor/slope/wall
sensors and velocities. Random consumption is preserved in its active path.
Actors render with stage/common palettes and multipart textures in priority
order around Alucard. Known unsupported families are exposed as unsupported
and emit one native warning per ID; malformed supported data still stops the session.

This checkpoint does **not** implement combat/damage, item drops/collection,
audio, zombie hit/death behavior, blade-blood particles, globe glow primitives,
the remaining actor families, puzzle/door interaction, complete GPU ordering,
ordinary saves, or original startup/camera/timing. Enemy attacks are visual;
they do not damage Alucard. An omitted actor is not evidence of completed parity.

For a direct actor playtest, choose the verified development start in room 3:

```sh
make && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data /path/to/private/extracted/us --laboratory --laboratory-room 3
```

Arrows move/crouch, X or Space jumps, and Escape quits. Z/D remain unsupported
actions and warn once without ending the session. Use `thread backtrace all`
at an LLDB crash stop. Room 1 remains the default entry checkpoint; these
development starts are not original game progression or entry scripts.

`modules/core/actors.c` adapts selected paths in `src/st/nz0/e_breakable.c`,
`e_red_eye_bust.c`, `e_nz0_room2.c`, `src/st/e_bloody_zombie.h`,
`animate_entity.h`, `st_common.h`, and `st_update.h`, retaining AGPL notices.
Offsets identify tables in the pinned US image; no animation or enemy tables
are imported into source. The core borrows immutable images/layout/maps and
owns its slots. Close the core before closing its current layout or game snapshot.

Synthetic tests cover initialization, byte-animation timing, landing, walking,
chase/attack transitions, unsupported actors, culling/reentry, bad references,
and camera/player bounds. A private comparison matched 32,768 ticks of the
unchanged active Bloody Zombie routines with damage, audio, and effect allocations
excluded. It compared positions, velocities, flags, steps, timers, poses,
frames, facing, and random state. This is active-path evidence, not complete
enemy or combat parity. A private native 600-tick room-3 sequence showed both
candles and zombies; framebuffer captures were inspected locally and stay ignored.
Additional private checks advanced selected behaviors for 7,424 updates and
12,678 draw calls across all 29 non-transition rooms and camera corners.
Unsupported actor families were excluded explicitly; this is not full-room parity.
