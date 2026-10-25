# Doppleganger arena adapters

BO4 (stage 26) now has explicit native resource adapters: 24 sprite banks, the
original bounded common-actor tables, two red-door layouts, private VAB header/body
verification, and silent stage initialization matching the original stage record.
Its layout pointer tables retain the same 53-entry format as the earlier areas.
It has no ordinary prize list or save rooms. No original table bytes are embedded.

Selected boss root/child combat now runs in an explicit native integration fixture;
see [runtime scope](doppleganger-runtime.md#stage-combat-orchestration). The [entry/reward service](doppleganger-entry.md) now runs selected original
entrance, seals, music and reward behavior. Remaining effects stay unsupported
rather than fall through to Laboratory behavior or table addresses. The [connected progression service](doppleganger-progression.md) now selects BO4
from original Outer Wall overrides. The public gameplay CLI retains its existing
development stage starts; BO4 has no separate gameplay start. Its [backed packed sprites](doppleganger-sprites.md) now have a native graphics
adapter. Selected AI, body/forms, combat and physical defeat are integrated;
full elemental effects and whole-route/console parity remain subsequent work.

Common BO4 adapters reject missing table mappings. Camera-lock metadata uses the
selected direct style and the seven fully stored property records; the original
header declares eight hitbox/axis entries, but provides only seven full property
records. The checked room layouts contain no camera-lock placements. Unsupported
property selection fails explicitly rather than reading adjacent data.

The data manifest adds filenames, lengths and SHA-1 fingerprints for the user's
BO4 sound bank. The existing stage catalog already identifies its overlay/art.
No private inputs enter public CI, packages or repository fixtures. Stage sound
selection now uses explicit profiles and fails for missing mappings.

Synthetic regressions check metadata, no arena saves, common table
selection/failure, boss dispatch isolation and direct camera bounds. Private LLDB
checks load the original arena, update/draw the ordinary development character
initialize the private sound service and exercise a supplied heart pickup and
shared explosion in Debug and macOS ASan/UBSan. They
verify adapter safety, not a boss battle, complete route or console parity.

Metadata and selected adapters follow `src/boss/bo4/{header,e_init,e_breakable,
e_red_door_tiles,e_lock_camera}.c`, shared `st/entity_lock_camera.h` / `e_collect.h`
and `src/dra/lba_stage.c` at sotn-decomp revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Original art, tables, executable overlays and audio remain private runtime input.
