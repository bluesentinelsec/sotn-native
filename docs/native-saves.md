# Native Laboratory save storage

`modules/save/` stores only the currently supported Laboratory profile: save-room
ID, HP/MP, hearts/subweapon, selected hand equipment, gold, kill count, and elapsed
simulation ticks. This is a versioned native development save, not a PS1 memory
card or a claim that unintegrated inventory, map, scripts, or progression persist.
No artwork, samples, executable bytes, or runtime game tables enter these files.

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

## Version 1

Records contain 80 bytes, encoded explicitly in little-endian order:

| Offset | Value |
| --- | --- |
| 0 | Eight-byte native magic `SOTNNSAV` |
| 8 | 32-bit format version, currently 1 |
| 12 | 32-bit record size, currently 80 |
| 16 | 32-bit supported US reference marker |
| 20 | 32-bit Laboratory save-room ID |
| 24–34 | Six 16-bit values: HP, MP, hearts, subweapon, left/right equipment |
| 36–40 | 32-bit gold and kill count |
| 44 | 64-bit elapsed simulation ticks |
| 52–75 | Reserved zero bytes |
| 76 | IEEE CRC-32 over the preceding 76 bytes |

The checksum detects corruption; it does not authenticate user-owned files.
Current bounds deliberately match the explicit development profile and selected
weapon support. Extend the model and introduce explicit migration when integrated
state grows. Stage-session temporary actor flags are not ordinary save state;
candles/enemies should respawn when loading rather than becoming permanent deaths.

Native Android/iOS use SDL’s sandbox preference path. Browser storage currently
fails explicitly: a persistent-storage adapter and browser reload checks must
precede claiming browser save support. Other browser build/runtime checks remain
available independently of this desktop-first gameplay milestone.

Public tests inject a build-local directory and synthetic state. They cover
byte order, wide tick counts, version/corruption/size failures, fifteen slots,
replacement, failed rename protection, invalid state, and retained corrupt files.
Tests do not read or modify real user slots. This module is original project C;
save-room gameplay/UI integration follows separately.
