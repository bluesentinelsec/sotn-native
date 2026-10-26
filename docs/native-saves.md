# Native save storage

Ordinary saves retain supported character inventory/equipment, EXP/level, base
stats, current/max resources, permanent pickups and relic ownership, gold/kills, subweapon, stage/room/time,
learned spell ownership/order, the completed-game unlock flag, original cutscene completion,
four gameplay/progression RNG streams, explored map cells, and permanent
Laboratory and Entrance events. This is a versioned development save, not a PS1 memory card or
full-game save. No artwork, samples, executable bytes, or runtime tables enter it.

The store uses `SDL_GetPrefPath("bluesentinelsec", "sotn-native")` and fifteen
one-based slots, `laboratory-01.sav` through `laboratory-15.sav`. The calling UI
confirms replacement. Keep these legacy filenames so existing slots stay discoverable;
the versioned contents now distinguish stages. Writers create an exclusive sibling temporary, check the
complete write, flush, close, and rename over the slot. Failed writes/close/rename
retain the previous destination and remove owned temporaries. Concurrent writers
cannot truncate another writer's temporary. Hardware power-loss durability still
depends on the platform/filesystem.

Readers require an exact-size regular file, bounded reads, end-of-file, native
magic, explicit version, US reference marker, checksum, reserved fields, and
supported state bounds. Failed loads leave output and files unchanged. Unknown
versions require explicit migration; application VERSION does not determine save
compatibility. The checksum detects corruption, not malicious modification.

## Version 9

New writes use a 4096-byte little-endian record. Native struct layout, padding,
host pointers, artwork, and runtime tables are never serialized.
BO5 currently records its original `0x300` completion bits. Other scenes' complete
completion-word/skip behavior remains separate integration work.

| Offset | Value |
| --- | --- |
| 0 | Eight-byte magic `SOTNNSAV` |
| 8 | 32-bit format version, 9 |
| 12 | 32-bit record length, 4096 |
| 16 | 32-bit supported US reference marker |
| 20 | 32-bit stage-local save-room ID |
| 24 | 64-bit elapsed gameplay ticks |
| 32–40 | Three 32-bit player/actor/combat RNG states |
| 44 | 16-bit original stage ID: 12 Laboratory, 65 first Entrance, 7 later Entrance, 0 Marble, 1 Outer Wall, 2 Library, 6 Chapel (internal) |
| 46–47 | Reserved zero bytes |
| 48–62 | Eight 16-bit values: HP/max, MP/max, hearts/max, level, subweapon |
| 64–70 | Four 16-bit base stats |
| 72–84 | Seven 16-bit equipment IDs |
| 86–87 | Reserved zero bytes |
| 88–100 | 32-bit EXP, gold, kills, character/progression RNG |
| 104–272 | 169 hand-item inventory counts |
| 273–362 | 90 body-item inventory counts |
| 363–531 | 169 hand-item acquisition-order IDs |
| 532–621 | 90 body-item acquisition-order IDs |
| 622–623 | Eleven permanent Laboratory pickup bits; upper five bits zero |
| 624–1391 | 768 original-sized castle progress flags |
| 1392–3439 | 2048 packed original/reversed-castle exploration bytes |
| 3440 | 16-bit explored-cell count |
| 3442–3447 | Retired Laboratory-only boss fields, now reserved zero bytes |
| 3448–3477 | Thirty original relic ownership bytes (0 absent, 1 found, 3 active) |
| 3478–3479 | Reserved zero bytes |
| 3480–3607 | Thirty-two 32-bit original HHMMSS time-attack records |
| 3608 | 32-bit original event-visited mask |
| 3612 | 32-bit original learned-spell mask |
| 3616–3623 | Eight encoded spell acquisition-order bytes |
| 3624 | Original completed-game unlock boolean |
| 3625–3627 | Reserved zero bytes |
| 3628 | 32-bit original cutscene-completion word |
| 3632–4091 | Reserved zero bytes |
| 4092 | IEEE CRC-32 over bytes 0–4091 |

Inventory excludes equipped copies. Counts, equipment/resource/stat bounds, order
permutations, permanent pickup bits, known event flags, original HHMMSS records (minute/second digits below 60), and packed
map/count consistency are checked. Loading resolves equipment from private
runtime definitions and recalculates attributes/cape colors. Derived stats and
render caches are never trusted from disk.

Each supported scene reveals cells once per simulation update using foreground
castle coordinates and the original hidden-map flag. Repeated visits count once.
The map preserves its original two-bit packing; the current scene adds exploration
bits only. The shop transaction service adds purchased-map high bits without
increasing visited-room counts. The [map screen](castle-map.md) now presents those bits and secret passages using
private original artwork; full map/presentation parity remains incomplete.

Continue installs world and character progress before actors initialize, then
restores owned RNG streams after initialization. Opened secret wall/floor and
cannon shortcuts stay open; the boss record suppresses another Slogra/Gaibon
encounter; the Doppleganger record suppresses its Outer Wall arena transfer;
the Lesser Demon record suppresses its Library manager/body spawn. Transient enemies, candles, projectiles, switches/lift positions, and
execution state are recreated. Saving requires a living character in Laboratory
rooms 23/24/25, first Entrance rooms 25/26/27, later Entrance rooms 22/23/24, Marble Gallery rooms 31/32, Outer Wall rooms 14/17, or Library room 14. This is an ordinary checkpoint save, not a save state.

## Explicit migration

Version 8 retains all its existing character/world/spell records. Its reserved bytes
3625–4091 must remain zero. Version 9 initializes the new completion word to zero
for v1–v8 inputs; older encounter flags do not fabricate a missing completion word.
The original `save_mgr.h:ApplySaveData` retains both completion masks across loads
in the same running session. Native Continue preserves that union for the visited
and cutscene words before the loaded actors initialize; other loaded progress still
comes from the selected slot. A fresh process starts without previous session masks.


Version 7 retains all its character/world/time records. Its reserved bytes
3612–4091 must remain zero. Migration initializes learned-spell ownership/order
and completed-game unlock to zero, because that format did not retain them.
Version 8 stores these fields explicitly and validates the acquisition-order/mask
agreement, duplicates, gaps and boolean bounds. Spell-scroll ownership does not
imply integrated casting. Existing slots and the 4096-byte size stay unchanged.


Versions 4–6 stored a Laboratory-only visited boolean at 3442 and elapsed seconds
at 3444, with zero padding at 3443. Version 7 migrates those fields to original
event 6 and converts seconds to HHMMSS without modifying the source file. Other
event records remain zero because those formats never stored them. Version 6
retains its relic bytes at 3448–3477; versions 4/5 require that region to be zero.
The retired fields must be zero in new v7/v8/v9 records. Existing slot paths and the
4096-byte file size remain unchanged.


Version 5 retains its stage identity and all supported progress. Versions 1–5
migrate with Cube of Zoe active because those checkpoints already granted its
candle drops. Their eleven Laboratory pickup bits also migrate into the original
castle-flag range; new writes retain the legacy mirror for character compatibility.
New Entrance starts collect Cube from its original room instead.

Version 4 remains 4096 bytes and requires zero bytes 44–47. It migrates to stage
12 without changing its existing world/character/time/RNG state. All older formats
also migrate to stage 12; none stored a stage identity.

Version 3 remains exactly 768 bytes. Its character fields at offsets 0–623 match
version 4, bytes 624–763 must be zero, and its CRC is at 764. Migration preserves
all stored character/time/RNG fields and initializes missing world progress to
zero. It cannot recover exploration or events that the older format never stored.

Versions 1 and 2 remain exactly 80 bytes. Their original offsets/checksums are
validated before migration. They retain HP/MP/hearts/subweapon, left/right hands,
gold/kills, room/time; v2 also retains gameplay RNG streams at 52–60. Version 1
requires zero bytes 52–75; version 2 requires zero bytes 64–75; both CRCs are at 76.
Migration supplies development maxima (70 HP, 20 MP, 50 hearts), level 1, base
stats 6, empty body equipment/inventory with identity order, zero EXP/character
RNG, and no permanent pickup/world progress. Version 1 supplies zero gameplay RNGs.

Reading a legacy save never changes its file. The next successful ordinary save
writes v9 through the checked replacement path. Unknown formats fail explicitly.
Native Android/iOS use SDL's sandbox preference path. Browser durable persistence
still requires an adapter and reload tests. PS1 memory-card import/export, further
stages, relic ability/menu integration, complete castle scripts, and full-game saves remain separate.

Public tests use a build-local directory and synthetic state. They check independent
v3/v4/v5/v6/v7/v8/v9 checksums, stage/save-room roundtrips, all eight legacy migrations, malformed character
and world state, reserved fields, length/version/checksum failures, fifteen slots,
retained corrupt files, and safe replacement/failed rename. Private native LLDB
checks use isolated local slots, never the user's real save bank. Private captures
and fixtures remain ignored.

The supported [Outer Wall elevator](outer-wall-elevator.md) adds original boolean flags
16/17 to the existing world payload. Version 6 stays unchanged; both flags roundtrip
through all supported stages/save rooms, and malformed values fail validation.

The [Entrance rock puzzle](entrance-rocks.md) retains original four-bit flag 51 and
boolean flag 58 in v6. Both side breaks survive stage changes and ordinary Continue;
the unsupported transformation prerequisites are never granted by breaking rocks.
