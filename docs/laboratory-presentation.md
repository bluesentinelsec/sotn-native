# Runtime Laboratory presentation

This repository's executable can now render the Laboratory's original tile
layers directly from private runtime data. This is a graphics-only diagnostic,
not a playable game. It does not run Alucard, enemies, room objects, combat,
palette animation, sound, or progression.

Build with `make debug-build`, then launch the macOS project executable through LLDB:

```sh
lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native \
  --game-data /path/to/private/extracted/us --preview-laboratory
```

Arrows pan within the selected room. Page Up/Down selects another non-transition
room. Escape quits. At a crash stop, use `thread backtrace all`; use `quit` to
leave LLDB. `--smoke-test` exits after three presented frames for local checks.
Normal builds, CI, and packages require none of the private game inputs.

## Rendering services

The tile renderer adapts the reconstructed `RenderTilemap` data interpretation
and `LoadStageTileset` upload layout. It resolves original tile IDs, texture
pages, packed 4/8-bit pixels, and palette references from runtime-loaded bytes.
It clips room coordinates, retains background wrapping, and distinguishes
transparent color zero from opaque/STP black. Four PS1 semi-transparency modes
operate on RGB555 components with saturation. Supported addressing covers the
loaded stage/common texture and palette regions; references to framebuffer
textures or unloaded palette data fail explicitly.

The fixed 256x240 RGB555 frame expands to RGBA8 and is presented by the existing
OpenGL/ES presenter and embedded shaders. Tile rasterization is CPU-side; this
slice is not a complete PS1 GPU-command implementation. Room background and
foreground layers are composed for inspection; entity ordering and scripted
background objects still require game integration. See the
[PSX-SPX GPU reference](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/)
for texture addressing, STP behavior, and blending definitions.

The Alucard sprite service resolves frame metadata in DRA and pixel sheets in
ARC_F.BIN without generated C arrays. It preserves packed-pixel order, clipping,
and the original mirrored pivot convention. It currently supports the normal
untransformed sprite-sheet path and base palette; equipment palette changes,
afterimages, primitive transforms, and other entity effects remain separate work.
The tile preview does not insert a simulated player or pretend to run gameplay.

The animation cursor adapts `SetPlayerAnim`, `func_8010DA70`, and `PlayAnimation`.
Animation durations, poses, loops, transfers, subanimations, sprite IDs, and
hitbox properties come from bounded DRA views at runtime. It advances exactly
one original animation tick per call. Integrating the simulation clock and
player state machine remains gameplay work.

## Private inputs and provenance

On-demand resource metadata now includes:

| File | Bytes | SHA-1 |
|---|---:|---|
| `ST/NZ0/F_NZ0.BIN` | 262144 | `89c5e97cb917121255d3a1a810c1d8ae098ebf8b` |
| `BIN/F_GAME.BIN` | 270336 | `a88e8f88078fd42f7198bd6f3becbd0c798d5541` |
| `BIN/ARC_F.BIN` | 203756 | `3b1fafe6f6f14b2bda18510441e8c385edf07341` |

The stage hash comes from the pinned reference manifest. Common/player sizes
and hashes were recorded from the user's private US extraction, alongside the
matching five core files. These fingerprints identify the selected inputs;
they do not establish whole-disc integrity or gameplay parity.

Adapted algorithms retain AGPL identifiers and attribution; the full license
ships in packages. Original format parsers and app wiring retain their original
project license. No SDK headers, ROM bytes, generated assets, screenshots, audio,
or extracted tables enter source or release packages.

## Validation

Synthetic tests cover original animation durations/control records, subanimations,
end holds, malformed pointers, output preservation, packed texture order, stage
and common pages, palettes, wrapping, 4/8-bit addressing, transparent/STP pixels,
all four blend modes, camera extremes, sprite pivots, mirroring, and clipping.
CPU cases run under Linux ASan/UBSan without game inputs. Existing desktop/WebGL2
checks exercise real OpenGL presentation independently.

Local LLDB checks passed the project executable's three-frame Laboratory preview,
29 room tilemaps at both camera extremes, and 512 ticks across each of 16 normal
Alucard animations with sprite resolution. Private GPU captures were visually
reviewed and remain ignored. These results prove the presentation/data path;
they do not claim a playable or authentic full-game build.
