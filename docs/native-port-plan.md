# Decompilation-based native port

## Decision and current state

Build on reconstructed C game logic, with native implementations of the PS1
services it needs. Preserve the US PS1 experience before adding enhancements.
Keep original simulation timing, integer/fixed-point behavior, audiovisual
presentation, game rules, and ordinary saves as the reference.

This repository now builds its own C17/SDL3/OpenGL actor checkpoint in
Alchemy Laboratory. It integrates reviewed reconstructed collision, animation,
and normal Alucard movement paths, with original runtime data supplied privately
by the user. Read [movement integration](laboratory-movement.md) for the supported
subset, [actor integration](laboratory-actors.md) for selected room behaviors,
and [presentation](laboratory-presentation.md) for graphics limitations.

Selected combat and candle drops/collection are integrated in a development profile;
see [combat limitations](laboratory-combat.md). Complete actors/effects, sound,
saves, original camera/timing,
scripts, and full-game progression remain incomplete. The first authentic playable
project milestone is still incomplete. Build and playtest our executable;
the earlier isolated comparison checkout is only source/reference material
and is not a project deliverable.

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

## Earlier feasibility experiment

The initial private experiment compiled the pinned comparison source and ran
its scripted prologue, exposing missing services and portability issues. That
proved the selected reconstruction route could run native C; it did not
integrate a game into this project or establish parity. Earlier comparison
playtests must not be counted as validation of our executable.

Keep the comparison checkout, generated files, and private inputs ignored.
Use the project's normal CMake/Make build and the LLDB playtest command in the
README for subsequent integration work. Do not package or publish a separate
reference executable as this project.

## Milestones and acceptance

1. **Feasibility baseline (completed).** Pin upstream and reference inputs,
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

The private data boundary, native collision/decompression, stage tables,
tile/sprite presentation, animation cursor, normal movement, actor placements,
and selected Laboratory actor behaviors are implemented in this repository.
These changes form a dependency stack through movement, actor data, actor
graphics, and scene integration. Leave PRs unmerged until the user playtests
our executable and approves; merge dependencies first afterward.

Next, extend the selected combat profile with equipped weapons and ordinary death/continue,
complete the remaining Laboratory actors/effects and audio services, and replace
development camera/timing and stage-entry behavior with reviewed
original paths. Use synthetic public regression tests and private input
comparisons. Do not bulk-vendor the reference repository or mark a full game
port complete after a demo or isolated room.

The [native sound service](laboratory-audio.md) now plays selected original
combat sound effects and Laboratory music; reverb and full audio parity remain incomplete.
