# Runtime Laboratory data

The normal `sotn-native` executable now decodes Alchemy Laboratory room and tile
metadata directly from the user's validated US `ST/NZ0/NZ0.BIN`. It also loads
the stage's tile graphics from `ST/NZ0/F_NZ0.BIN` on demand. Neither file is
generated into C, copied into source, or included in a package.

After `make`, inspect the runtime data under LLDB on macOS:

```sh
lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native \
  --game-data /path/to/private/extracted/us --inspect-laboratory
```

This inspection exits successfully after checking 32 room headers, including
3 cross-stage transition entries, and the graphics resource. It does not launch
gameplay or claim that the room's entities and graphics are implemented. At an
LLDB crash stop, use `thread backtrace all`; use `quit` to leave the debugger.

## Ownership and parsing

`SotnStage_Open` borrows the immutable NZ0 image and owns only a small metadata
object. Close the stage before closing its game-data snapshot. Decoded room
and layer byte views retain the backing snapshot's lifetime. The API validates
every room before publishing the stage and decodes fields without casting file
bytes to native structs. It does not execute PS1 code.

The stage format uses 32-bit PS1 addresses relative to `0x80180000`. Resolution
checks alignment, offset, and length inside the image's bounded region. Room
tables require a terminator and have a maximum of 256 entries. Layer rectangles
use the original four six-bit coordinates; dimensions determine the tile array
length. The maximum referenced tile ID determines the required extent of each
graphics-page, graphics-index, palette, and collision-code array. Every referenced
array must fit. Transition entries remain explicit and have no decoded layers.

These format facts come from pinned `include/game.h` (`RoomHeader`, `LayerDef`,
`TileDefinition`, `RoomDef`) and `config/splat.us.stnz0.yaml`. The parser is
original C; the repository includes no generated room or tile tables.

`SotnResource_Open` loads a fixed resource ID independently of the five-file
core snapshot. It owns bounded immutable bytes and uses the existing verified
read path: regular-file checks, exact length, full read, EOF, and SHA-1. Its
views remain stable if the source file changes later. Close resources after all
readers finish. Public callers cannot supply arbitrary resource filenames.

`config/reference-us.json` records the extra resource's 262,144-byte size and
SHA-1 `89c5e97cb917121255d3a1a810c1d8ae098ebf8b`, matching pinned
`config/check.us.sha`. This fingerprint identifies the expected US bytes; it
does not replace the parser's bounds checks or establish whole-game parity.
The existing reference-verifier script still verifies its five core files;
the runtime resource loader verifies additional resources when requested.

## Validation

Synthetic C tests cover room/layer decoding, transitions, collision integration,
truncation at every byte before the last required field, bad or misaligned
pointers, invalid dimensions and tile references, and output preservation.
Resource tests cover stable snapshots, missing inputs, altered bytes, wrong
sizes, invalid IDs, and allocation limits. CLI failures work without video.
These tests require no game dump and run under sanitizer CI.

The local executable passed full Laboratory data inspection under LLDB using
the user's private dump. That check validates this data boundary, not gameplay.
