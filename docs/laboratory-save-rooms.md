# Laboratory save-room checkpoint

This repository's executable recognizes the original Laboratory save-room flag
and integrates rooms 23, 24, and 25. Stand near the center and press Up. Activation
restores HP/MP even if saving is cancelled, centers Alucard, and opens native slot
selection. Choose one of fifteen slots with Up/Down, confirm, then choose Yes/No.
An occupied slot explicitly asks to overwrite. The selected original grow, spin,
scatter, and closed-coffin clocks precede the checked write. Acknowledging success
returns control; failed writes show Retry/Back and retain the previous file.

Keyboard confirmation uses X/Space or Enter; A cancels. Xbox uses A to confirm
and Y to cancel. Left/Right selects Yes/No. Enter is a desktop concession; the
US reference uses Cross/Triangle. Escape still quits the application. Release
and press Up again to reactivate after cancellation or completion.

Save rooms stop Laboratory music and use the original heartbeat and selected
save/menu sound events. Leaving restarts Laboratory music. Save-point meshes,
UVs, texture pixels, coffin palette, and menu font are read from the user's
existing private DRA/F_GAME resources. No additional copyrighted input ships.

## Playtest and load

Build and launch this repository's executable through LLDB (macOS):

```sh
make && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data local/game-data/us --laboratory --laboratory-room 23 --laboratory-loadout dual
```

Use Up at the point, select a slot, confirm saving, acknowledge completion, then
quit. Start another session from slot 1 with:

```sh
lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data local/game-data/us --laboratory --laboratory-continue 1
```

At a crash, use `thread backtrace all`. Continue accepts slots 1..15 and cannot
combine with development room/loadout selection. Missing/corrupt/unsupported
records fail clearly and remain untouched.

## Persistent state and limits

The version-five [native format](native-saves.md) retains supported inventory,
seven equipment slots, EXP/level/base stats, current/max resources, permanent
Laboratory item bits, subweapon, gold/kills, room/time, four owned RNG streams, packed exploration, and permanent Laboratory
events including secret/cannon shortcuts and the defeated paired boss.
Loading recreates a fresh session at the saved point; temporary enemy/candle and
projectile state reset. This is an ordinary room save, not an execution snapshot.
Empty hands, Basilard, Gladius, Mace, and the selected supported body equipment
can be restored. Map UI, relic progression, castle scripts, other stages,
and PS1 memory-card import/export remain unintegrated.

The native fifteen-slot bank and text dialogs adapt memory-card hardware/file
selection to the filesystem. Full original card-port/title menus and save-file
portraits/metadata are not implemented. Physical game over now returns to the native Continue shell after its fade.
Use `--play` to open the shell directly; command-line Continue remains supported. Browser durable
persistence remains unavailable until an adapter and reload tests exist.

Mesh projection uses bounded CPU triangle rasterization in the existing RGBA/
OpenGL presenter. Full GTE precision, original lighting, coffin opening/wireframe
primitives, menu animation/layout, shared global RNG consumption, and audiovisual
parity remain unproven. These are stated checkpoint limits, not claims of a
complete authentic game or save implementation.

Selected activation/centering/coffin clocks derive from
`src/dra/62DEC.c:func_80105428` and `6DF70.c:func_8010E334`; font addressing derives
from `menu.c:MenuDrawChar`, at the pinned reconstructed revision under
AGPL-3.0-or-later. Runtime mesh/UV/palette data stay private. Native slot dialogs,
versioned storage, and ownership adapters are original project code.

Public synthetic tests cover activation bounds, centering, release/repress input,
slot wrapping, cancellation, confirmation, one write request, failure/retry,
success acknowledgment, renderer/resource bounds, version migration, corruption,
and replacement protection. Private native LLDB checks exercise healing,
coffin/write, failure/retry, and fresh-session restoration without touching real
user slots. Captures and private fixtures remain ignored. These checks establish
the exercised paths; they do not prove full original-game parity.
