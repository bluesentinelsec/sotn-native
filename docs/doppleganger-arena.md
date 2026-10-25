# Doppleganger arena adapters

BO4 (stage 26) now has explicit native resource adapters: 24 sprite banks, the
original bounded common-actor tables, two red-door layouts, private VAB header/body
verification, and silent stage initialization matching the original stage record.
Its layout pointer tables retain the same 53-entry format as the earlier areas.
It has no ordinary prize list or save rooms. No original table bytes are embedded.

This is a resource/service checkpoint, not an integrated encounter. Boss,
entrance/reward/effect actors remain explicitly unsupported instead of falling
through to Laboratory behavior or table addresses. The profile's connected-travel
gate stays false, and the public gameplay CLI still accepts only NO0/NO1. Ordinary
travel cannot enter an empty boss arena. Its AI, movement/forms, attacks, intro,
defeat, reward, music events and progression remain subsequent integration work.

Common BO4 adapters reject missing table mappings. Camera-lock metadata uses the
selected direct style and the seven fully stored property records; the original
header declares eight hitbox/axis entries, but provides only seven full property
records. The checked room layouts contain no camera-lock placements. Unsupported
property selection fails explicitly rather than reading adjacent data.

The data manifest adds filenames, lengths and SHA-1 fingerprints for the user's
BO4 sound bank. The existing stage catalog already identifies its overlay/art.
No private inputs enter public CI, packages or repository fixtures. Stage sound
selection now uses explicit profiles and fails for missing mappings.

Synthetic regressions check metadata, disabled travel/saves, common table
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
