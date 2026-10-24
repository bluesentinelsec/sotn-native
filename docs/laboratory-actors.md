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

This is a placement/lifecycle dependency. It does not yet update or draw actors
in `--laboratory`, implement their collision or combat, or provide saved-game
persistence. Behavior-driven culling, sprite rendering, objects, and enemies
must use these slots in subsequent integration changes. Current movement
limitations still apply; see [movement integration](laboratory-movement.md).

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
