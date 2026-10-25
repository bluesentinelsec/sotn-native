# Early castle stage integration

The development executable now accepts Marble Gallery (NO0, stage 0) and Outer
Wall (NO1, stage 1), alongside Laboratory and both Entrance variants. These new
profiles establish shared services; their area-specific gameplay is incomplete.
A verified stage resource or a room rendering successfully does not prove area parity.

## Connected services

Original private room-teleport records connect Laboratory's east exit to Marble
Gallery and Marble Gallery's east connector to Outer Wall, including return trips.
Character resources/equipment, relic ownership, original permanent pickup flags,
exploration, time and owned RNG streams transfer through the existing transactional
session replacement. Missing destination data leaves the current session owned
and the existing saves intact.

Each profile sets its original actor-ID bound, private table locations, permanent
pickup range, red-door tile count, sprite-table count, and ordinary save rooms.
Marble Gallery saves use rooms 31/32; Outer Wall uses 14/17. Version 7 is the
explicit save format. Continue labels these slots MAR and OUT.

The shared Skeleton behavior uses the original stage-specific body, thrown-bone
and death-part IDs and private tables. Marble Gallery also reuses the original
green Axe Knight behavior with its local axe ID. Marble's Skeleton sprite bank 24
is now supported with an explicit 25-entry table bound; earlier profiles retain
24 entries. Music uses the original stage intro/loop records: Marble Gallery and
Tower of Mist, with checksum-verified stage sound banks loaded on demand.

Selected [Outer Wall ranged encounters](outer-wall-ranged-enemies.md) now run Bone
Muskets and Bone Archers with selected projectiles and death behavior. The
[Spear Guard](outer-wall-spear-guard.md) adds selected melee, blocking, throw and death rules.
[Sword Lord](outer-wall-sword-lord.md) adds its weapon hitbox, attacks and staged death.
[Blue Axe Knight](outer-wall-blue-knight.md) adds armored/damaged variants, charges and axes.

Selected shared pickups, relics, doors, camera locks and breakables run against
those profiles. Outer Wall's background-door flag is decoded separately from its
tile index; complete background-door routing/presentation and breakable debris
remain incomplete. The Wall camera uses its immediate lock variant, while Marble
retains the smoothed variant. Their different data and algorithms are not aliased.

## Development entry and remaining work

Build through Make/CMake and launch this repository's executable through LLDB:

```sh
make && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data local/game-data/us --castle NO0
```

`--castle NO1` starts the Outer Wall connector; `--castle-room INDEX` selects an
explicit development room. These entries use the existing development character,
not original prologue-derived initialization. Private data must be supplied locally.

The [early-route clock room](marble-clock-room.md) now runs timed mechanisms and selected presentation.
The [Outer Wall elevator](outer-wall-elevator.md) now supports selected rides, remote calls and saved activation.

Selected [Marble enemies](marble-gallery-enemies.md), including
[Skelerang and Diplocephalus](marble-linked-enemies.md), now run. Other area-specific
enemies, the [late clock ritual](marble-clock-room.md), hidden routes, boss transfer
and full Doppleganger parity remain incomplete. Selected [boss progression](doppleganger-progression.md) now connects its arena and reward. The existing unsupported-actor diagnostics stay
visible. Do not call these stages complete or this a complete authentic game.

## Validation and provenance

Public synthetic regressions cover stage-aware actor IDs/factories, bone velocity,
invalid shared-table accesses, 24/25 sprite-bank bounds, all explicit save rooms,
and CLI failure handling. Private LLDB sequences cover five room starts, walking
across each new stage boundary in both directions, progress retention, and an
isolated Outer Wall save followed by fresh load. They use explicit starts and
protected fixture steps, so they do not establish continuous-route or console parity.
Dummy audio verifies decoding rather than real-device listening.

Shared behavior derives from `st/e_skeleton.h`, `st/entity_axe_knight.h`,
`st/e_lock_camera.h`, `st/entity_lock_camera.h`, `st/no1/e_red_door.c` and shared
collection/breakable routines at the pinned sotn-decomp commit, under AGPL-3.0-or-later.
Stage metadata names offsets and dimensions only; private definitions, tables,
artwork, audio, dialogue and game binaries remain outside the public source.
