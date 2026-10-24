# Private core-data loading

The C17 app reads five core reference files from a user-selected, privately
extracted US PS1 dump. Additional [stage resources](stage-data.md) load on demand
with the same verified read path. This boundary does not implement disc extraction
or validate the entire disc. Public builds and packages contain filenames, sizes,
and hashes; they contain no game bytes. The data library is original C.

## Usage

Build with `make`, then run the native executable with:

```sh
sotn-native --game-data /path/to/private/extracted/us --validate-only
```

Use the executable path reported by the desktop build. On macOS, it is
`build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native`. Validation runs
before graphics initialization, logs a result, and exits nonzero on failure.
Without `--validate-only`, the app retains the validated bytes and displays
their ready status in the bootstrap. `--smoke-test` still runs three frames;
it can be combined with `--game-data` for a local integration check.

The existing ignored upstream extraction from the feasibility experiment can
be used locally with `--game-data .tools/sotn-decomp/disks/us`. Keep all inputs
and extraction products under ignored directories or outside the checkout.
Never copy them into source, fixtures, package install rules, or CI artifacts.

## Reference and validation

[config/reference-us.json](../config/reference-us.json) is the single metadata
source for the C loader. CMake validates it and generates a private build header
with the five filenames, expected byte lengths, and upstream SHA-1 values:

| Logical file | Relative path | Bytes |
|---|---|---:|
| Boot executable | `SLUS_000.67` | 563,200 |
| Main gameplay | `DRA.BIN` | 1,153,136 |
| Richter | `BIN/RIC.BIN` | 236,120 |
| Prologue | `ST/ST0/ST0.BIN` | 271,812 |
| Alchemy Laboratory | `ST/NZ0/NZ0.BIN` | 309,120 |

These hashes identify the pinned upstream US reference. SHA-1 is used only for
revision fingerprints, not cryptographic authentication. Its implementation
follows [FIPS 180-4 sections 5 and 6.1](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf).
The expected sizes were recorded from the locally verified matching files.

The loader requires a readable directory and regular files, opens files in
read-only mode, checks exact sizes before reading, handles partial reads,
requires end of file, and verifies hashes before publishing a snapshot.
Errors name the affected file and operation. Partial failures release all
allocations and return no usable snapshot. It writes or extracts nothing.

The directory is explicitly selected by the user. SDL follows filesystem
symlinks; this C API does not promise filesystem sandboxing. Its public read
interface accepts logical file IDs, not arbitrary relative paths. External
input parsing must still check field bounds and formats before using bytes.

## Ownership and limits

`SotnGameData_Open` creates an immutable snapshot using one bounded byte arena
and a small owning object. The current snapshot is 2,533,388 bytes; the combined
allocation budget is 16 MiB. This budget applies to this core-file boundary,
not a future cache for every stage or audio resource.

`SotnGameData_GetFile` returns borrowed byte views. Reads after initialization
perform no further filesystem access, so changing the source files cannot
change an existing snapshot. Every view expires at `SotnGameData_Close`.
Finish all readers before closing; never free a borrowed view independently.

Treat views as on-disc bytes. Decode byte order, integers, offsets, and lengths
explicitly. Do not cast a file to native structs containing pointers, and do
not execute the PS1 executable or overlays as host machine code. A matching
hash is not evidence that a future parser handles every input safely.

No persistent game files are written in this milestone. Ordinary save/load
will need a separate `SDL_GetPrefPath` adapter with migration and failed-write
tests. Raw BIN/CUE reading, additional game resources, native game logic,
player updates and platform input selection remain later integration work.

## Validation

`make test` includes C tests with original synthetic data: padding/block-boundary
and long-message hash vectors, successful snapshots, stable views after source
changes, bad file IDs, missing files, wrong sizes/hashes, short and failed reads,
unknown sizes, size changes, and allocation bounds. CLI checks require clear
failures and reject false success without a working graphics driver.

The same tests run under Linux ASan/UBSan in CI. Public tests require no game
data. Real-file validation and combined loading/rendering checks run locally
with the user's dump and do not establish whole-game parity.
