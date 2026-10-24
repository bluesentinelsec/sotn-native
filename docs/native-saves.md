# Native Laboratory save storage

The native ordinary save retains supported character inventory, equipment, EXP/level,
base stats, current/max resources, permanent Laboratory pickups, gold/kills,
subweapon, room/time, and four owned gameplay/progression RNG streams. This is
a versioned development save, not a PS1 memory card or full-game save. No artwork,
samples, executable bytes, or runtime game tables enter these files.

The native store uses `SDL_GetPrefPath("bluesentinelsec", "sotn-native")` and fifteen
fixed one-based slots, `laboratory-01.sav` through `laboratory-15.sav`. It never
uses the working/install directory. This provides one native bank with the original
per-card file capacity; emulated memory-card ports/file menus remain separate work. Slot replacement requires confirmation from
the calling UI. Writers create an exclusive sibling temporary, check the complete
write, flush, close, and rename over the slot. Any failed write/close/rename leaves
the previous slot intact; owned temporaries are removed after failure. Concurrent
writers cannot truncate another writer’s temporary. Hardware power-loss durability
still depends on the platform/filesystem.

Readers require an exact-size regular file, bounded reads, end-of-file, the native
magic, explicit format version, US reference marker, checksum, and supported
state bounds. A failed load leaves the destination state and file unchanged.
Unknown versions fail with a migration error; never infer save compatibility from
application VERSION or reinterpret unknown reserved bytes. PS1 memory-card
import/export requires a separate format implementation.

## Version 3

New writes use a 768-byte, explicitly encoded little-endian record. No native
struct layout, padding, host pointers, artwork, or runtime tables are serialized.

| Offset | Value |
| --- | --- |
| 0 | Eight-byte magic `SOTNNSAV` |
| 8 | 32-bit format version, 3 |
| 12 | 32-bit record length, 768 |
| 16 | 32-bit supported US reference marker |
| 20 | 32-bit Laboratory save-room ID |
| 24 | 64-bit elapsed gameplay ticks |
| 32–40 | Three 32-bit player/actor/combat RNG states |
| 44–47 | Reserved zero bytes |
| 48–62 | Eight 16-bit values: HP/max, MP/max, hearts/max, level, subweapon |
| 64–70 | Four 16-bit base stats |
| 72–84 | Seven 16-bit equipment IDs |
| 86–87 | Reserved zero bytes |
| 88–100 | 32-bit EXP, gold, kills, and character/progression RNG |
| 104–272 | 169 hand-item inventory counts |
| 273–362 | 90 body-item inventory counts |
| 363–531 | 169 hand-item acquisition-order IDs |
| 532–621 | 90 body-item acquisition-order IDs |
| 622–623 | Eleven permanent Laboratory pickup bits; upper five bits zero |
| 624–763 | Reserved zero bytes |
| 764 | IEEE CRC-32 over bytes 0–763 |

Inventory counts exclude equipped copies. Counts, equipment ranges, resource
bounds, stat bounds, order permutations, and permanent bits are checked before
returning decoded state. Loading then resolves supported equipment against the
private runtime definitions and recalculates attributes/cape colors. Derived
stats and render caches are never trusted from disk. Saving requires a living
character in room 23, 24, or 25. Transient candles, enemies, projectile slots,
stopwatch state, and current execution position are intentionally recreated.
Map exploration, castle scripts, full progression, and PS1 memory cards remain
unintegrated. This format is an ordinary checkpoint save, not a save state.

## Legacy migration

Versions 1 and 2 remain exactly 80 bytes. Their original offsets and checksums
are validated before explicit migration. They store HP/MP/hearts/subweapon,
left/right hands, gold/kills, room/time; version 2 also stores the three gameplay
RNG streams at offsets 52–60. Version 1 requires zero reserved bytes 52–75;
version 2 requires zero at 64–75. Their CRC remains at offset 76.

Migration retains all stored fields and supplies the original development
maxima (70 HP, 20 MP, 50 hearts), level 1, base stats 6, empty body equipment,
empty inventory with identity acquisition order, no permanent pickup bits, and
zero character RNG/EXP. Version 1 initializes missing gameplay RNG streams to
zero. Progress never stored cannot be recovered. Reading does not modify the
file; the next successful ordinary save writes version 3 through the existing
checked replacement path. Unknown formats require explicit migration.

The checksum detects corruption; it does not authenticate user-owned files.
Native Android/iOS use SDL's sandbox preference path. Browser persistence still
fails explicitly pending a persistent-storage adapter and reload checks.

Public tests inject a build-local directory, never real user slots. Coverage
includes an independently generated version-three checksum, complete inventory
roundtrip, wide time/resource values, v1/v2 migrations, malformed counts/order,
reserved fields, checksum/version/length failures, fifteen slots, retained
corrupt files, safe replacement, failed rename, and migration on read/write.
A private native LLDB save-room run equipped Basilard/body gear/cape, gained
levels/vessels, saved 61/66 hearts and four RNG streams, and restored a fresh
session to byte-identical ordinary progress. A forced failed replacement
preserved the previous destination and the retry succeeded.
