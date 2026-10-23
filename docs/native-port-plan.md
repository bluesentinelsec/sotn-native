# Decompilation-based native port

## Decision and current state

Build on reconstructed C game logic, with native implementations of the PS1
services it needs. Preserve the US PS1 experience before adding enhancements.
Keep original simulation timing, integer/fixed-point behavior, audiovisual
presentation, game rules, and ordinary saves as the reference.

The repository's app remains the C17 bootstrap, now with an original
[private core-data loader](private-data.md). No game code or assets have been
imported. An isolated upstream build under ignored `.tools/`
provides an executable baseline for evaluating the route; it is not our port
and must not be packaged or published as a project release.

## Pinned reference

[config/reference-us.json](../config/reference-us.json) records:

- [sotn-decomp](https://github.com/Xeeynamo/sotn-decomp) commit
  `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
- Its PSY-Z backend at `b52003e55c0d1b20719e984dd19bd4d760eeb14a` and SDL
  submodule at `fa2c02bb6e21974a89ea9824bc53c9932abe5f9c` (SDL 3.4.16).
- North American PS1 reference `SLUS-00067`, with five core file SHA-1 values
  from upstream's pinned `config/check.us.sha`.

The local user's dump matches all five selected hashes. This establishes
compatibility for the selected executable/overlays, not complete disc integrity,
ownership, an unmodified asset set, or behavioral parity. Hashes identify inputs;
they are not game assets. Never publish extracted bytes or private logs.

After extracting a user's own disc privately, run:

```sh
python3 scripts/verify_reference.py /path/to/private/extracted/us
```

The verifier reads files without copying or modifying them. It rejects missing,
modified, non-file, and out-of-directory inputs. Public unit tests use original
synthetic bytes; they do not need a disc.

## Feasibility findings

| Area | Evidence at the pinned revisions | Integration work |
|---|---|---|
| Game logic | The PC source lists compile reconstructed C and native stage/weapon modules. | Bring in a reviewed subset with provenance; preserve upstream attribution and license notices. |
| Completeness | The upstream PC-port guide says no complete end-to-end port exists. PC stubs and unbuilt overlays remain. | Track missing functions, stages, weapons, progression, and endings explicitly. |
| SDL | The backend links SDL3 statically. Our bootstrap pins SDL 3.4.18. | Use our single pinned SDL dependency when integrating; test the version change. |
| Graphics | macOS explicitly rejects the OpenGL backend and selects SDL GPU. GL source already embeds shader strings for other targets. | Supply a tested macOS OpenGL path and embed its shaders; do not change our renderer direction to match the experiment. |
| Assets | `extract_assets` and `build_assets` generate data and C includes from a local dump. | Load private game data at runtime; generated C arrays are still game data and cannot enter public source or release binaries. |
| Saves | PSY-Z creates `bu00` and `bu10` under the working directory. | Route storage through `SDL_GetPrefPath`, preserving the original save behavior and testing failed writes. |
| Portability and safety | Upstream documents pointer-width, scratchpad, alignment, and null-pointer issues; its build suppresses several C warnings. | Fix issues at boundaries, add regression tests, and retain our warnings-as-errors and sanitizer checks. |
| Licensing | Game sources identify AGPL-3.0-or-later. PSY-Z's inventory lists MPL 2.0 platform code and labels some headers unlicensed; its `decomp/` tree has an MIT notice. | Resolve the provenance and license of each proposed import before publishing it; do not assume one top-level license covers every file. |

Sources: [upstream PC-port guide](https://github.com/Xeeynamo/sotn-decomp/wiki/PC-Port),
[pinned game CMake](https://github.com/Xeeynamo/sotn-decomp/blob/7bc54d8157b6bed77e6bd15e5db9461a734fe1e5/CMakeLists.txt),
[pinned PC stubs](https://github.com/Xeeynamo/sotn-decomp/blob/7bc54d8157b6bed77e6bd15e5db9461a734fe1e5/src/pc/stubs.c),
[backend CMake](https://github.com/Xeeynamo/psyz/blob/b52003e55c0d1b20719e984dd19bd4d760eeb14a/psyz/CMakeLists.txt),
[backend license inventory](https://github.com/Xeeynamo/psyz/blob/b52003e55c0d1b20719e984dd19bd4d760eeb14a/LICENSE),
[backend decomp license](https://github.com/Xeeynamo/psyz/blob/b52003e55c0d1b20719e984dd19bd4d760eeb14a/decomp/LICENSE).

## Reproduce the private upstream experiment

This is a developer experiment on macOS, separate from normal `make`, CI,
packaging, and releases. It requires Xcode command-line tools, Git, CMake,
Ninja, Go 1.24 or newer, and Rust/Cargo. The tested Rust compiler was 1.99.1.
Go and Rust build host tools; the native game source is C.

The pinned source built successfully on macOS arm64 with AppleClang 21.0.0,
and scripted prologue playback (`--demo 0`) exited successfully using the
local reference dump and dummy audio. Runtime logs include unimplemented
service messages and a tileset-size warning, so successful playback is not
an assertion of visual, audio, or gameplay fidelity. Audio output was not
listened to during this automated check.

```sh
git clone https://github.com/Xeeynamo/sotn-decomp.git .tools/sotn-decomp
git -C .tools/sotn-decomp checkout --detach 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5
git -C .tools/sotn-decomp submodule update --init tools/psyz
git -C .tools/sotn-decomp/tools/psyz submodule update --init --depth 1 external/SDL
```

Place or link the user's own BIN tracks and CUE in the ignored upstream
`disks/` directory. Name the CUE `sotn.us.cue`; keep its referenced track names
valid relative to that directory. Do not download game inputs or obtain
upstream's private CI dependency archive.

From the upstream checkout:

```sh
make extract_disk
make extract_assets build_assets
python3 ../../scripts/verify_reference.py disks/us
cmake -S . -B build/pc -G Ninja -DENABLE_ASAN=OFF
cmake --build build/pc --parallel 6
SDL_AUDIODRIVER=dummy ./build/pc/sotn --disk disks/sotn.us.cue \
  --demo 0 --replay-fast --replay-and-exit
```

`ENABLE_ASAN=OFF` reproduces upstream's desktop CI configuration for the
baseline only. It does not relax this project's sanitizer requirements or
establish that upstream is memory-safe. Keep generated includes, binaries,
screenshots, saves, and logs inside ignored directories. The experiment uses
upstream's SDL GPU renderer on macOS and its own entrypoint; neither becomes
an alternate entrypoint or renderer in this project's build.

## Milestones and acceptance

1. **Feasibility baseline (this pass).** Pin upstream and reference inputs,
   compile privately, exercise the scripted prologue, and record real gaps.
   A successful scripted run proves that path runs; it does not prove parity.
2. **First playable project build.** Keep `src/main.c` as the entrypoint. Import
   only reviewed C interfaces/logic with resolved provenance. Load private data
   at runtime and implement the minimum native services for the prologue or
   one supported Alucard room. Require keyboard/Xbox controls, correct timing,
   OpenGL presentation, sound, and explicit failures for missing/unsupported
   data or unimplemented behavior. Compare a repeatable sequence with the
   original. Provide a local playtest command; publish no game data.
3. **Desktop progression.** Expand through castle entry, transitions, combat,
   inventory, bosses, ordinary save/load, inverted castle, and endings. Track
   unsupported paths instead of silently stubbing them. Add replay/checkpoint
   comparisons and sanitizer regression cases as each path becomes available.
4. **Authentic desktop release.** Complete documented playthrough and parity
   checks on macOS, Windows, and Linux. Asset-free packages must work with a
   supported user dump, have safe storage, and pass all platform CI checks.
   Keep enhancements out of this milestone. Web/mobile builds stay green;
   game-specific adaptations follow the proven desktop foundation.

The private data-loading boundary is implemented for the five core reference
files. The next implementation slice is reviewed C/OpenGL compatibility and
game logic with runtime-loaded data. Do not bulk-vendor the upstream
repository or mark a full game port complete after a demo or isolated room.
