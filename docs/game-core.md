# Native game-core integration

The `sotn-core` C17 library starts integrating reconstructed game behavior into
this repository. It is compiled by the normal CMake/Make build on every target.
The first slice contains tile collision and compressed graphics decoding;
it does not yet implement player updates, a game loop, or a playable room.

## Provenance and distribution

Both implementations derive from the explicitly AGPL-3.0-or-later C files at
sotn-decomp commit `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`:

- `src/dra/collider.c:CheckCollision` supplies the tile collision rules.
- `src/dra/4AEA4.c:DecompressData`, `DecompressReadNibble`, and
  `DecompressWriteNibble` supply the graphics stream semantics.

The adapted files retain SPDX identifiers and identify their origin. The full
license is in [licenses/AGPL-3.0-or-later.txt](../licenses/AGPL-3.0-or-later.txt).
Original scaffold code retains its zlib notice; distributions linking the game
core must comply with the core's AGPL terms and provide corresponding source.
No SDK headers, generated game tables, artwork, audio, or game bytes are imported.
Normal builds do not require the reference checkout or private extraction tools.

The adaptations replace global state with explicit borrowed byte views and
per-call state. They decode little-endian tile IDs without native pointer casts,
check map/stream/output bounds, initialize local state, and preserve caller
output after validation failures. Collision neighbor probes retain the original
one-level limit. World coordinates use 64-bit intermediates for safe neighbor
arithmetic. Original unsigned shifts and 32-bit collision result bits are
explicit, including counterintuitive high bits in certain ceiling slopes.
The decoder validates the stream before writing output and preserves opcode,
dictionary, termination, and nibble-order behavior. No game constants or
algorithms are intentionally adjusted for modern gameplay.

## Validation

`make test` exercises all 256 collision codes at every pixel in a synthetic tile,
floor-slope geometry, one-way flags, original wrapping arithmetic, out-of-room
coordinates, bad definitions, and recursive probe failures. Graphics cases cover
every opcode class and dictionary slot, run limits, odd output, truncated input,
undersized output, overlapping buffers, and output preservation. Public tests
use synthetic bytes. The same tests run in Linux ASan/UBSan CI.

Local differential checks against the unchanged pinned C functions matched
16,777,216 synthetic collision queries (varying neighboring tile codes) and
10,000 synthetic graphics streams with no differences. Reference wrappers and
their outputs remain ignored development artifacts; these checks do not claim
visual or gameplay parity for a room that is not integrated yet.

## Playtest workflow

Keep the integration PRs stacked and unmerged pending the user's playtest and
verification. Provide LLDB launch commands for this project's eventual gameplay
mode. A macOS launch has this shape:

```sh
lldb -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native [game arguments]
```

Use `run` at the LLDB prompt and `thread backtrace all` after a crash. Current
core tests are CPU checks, not a gameplay build; no playable mode is claimed.
