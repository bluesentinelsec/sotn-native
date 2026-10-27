# Decompilation-based native port

## Decision and current state

Build on reconstructed C game logic, with native implementations of the PS1
services it needs. Preserve the US PS1 experience before adding enhancements.
Keep original simulation timing, integer/fixed-point behavior, audiovisual
presentation, game rules, and ordinary saves as the reference.

This repository builds its own C17/SDL3/OpenGL Laboratory checkpoint with normal
Alucard movement, selected weapons/enemies, mechanisms, camera follow/locks,
Slogra/Gaibon, HUD, audio, death/game over, inventory/progression, and ordinary
saves. Native gameplay also connects selected first/later Entrance, Marble Gallery
and Outer Wall paths. The current stack adds original opening equipment, collected
Short sword/relic progression, Maria, selected area enemies, clock mechanisms and
the Wall elevator. The fifty-stage catalog remains a resource diagnostic, not fifty
playable stages. The native Continue shell and exploration map also remain selected
adaptations, with original title/new-game flow incomplete.

Selected Doppleganger combat/transfer/reward and connected Long Library travel,
Lesser Demon, Librarian buy/sell and furnishings now run. The
[Royal Chapel](chapel-connections.md) now connects to Laboratory and its selected
[Hippogryph encounter](hippogryph-body.md): owned attacks, grab, physical defeat,
reward, Maria dialogue/speech/flashbacks and return travel. Its component and
protected native checks do not establish ordinary-character, whole-route or console
parity. Further Chapel exits still fail explicitly until their stages are integrated.
The internal [Olrox's Quarters foundation](olrox-quarter-runtime.md) adds NO2's
resources, selected shared actors, [prisoners and spikes](olrox-prisoners-spikes.md),
[Valhalla Knights](olrox-valhalla.md), and articulated
[Hammer/Blade encounters](olrox-giant-knights.md), and NO2’s
[fifteen-weapon Spectral Sword](olrox-spectral-sword.md), and the selected
[secret wall/ceiling mechanisms](olrox-secrets.md), plus the selected
[stone doors](olrox-stone-doors.md) and
[courtyard scenery](olrox-courtyard.md);
[Selected connected travel](olrox-connections.md) now links Marble Gallery,
Chapel and the original eastern boss approach. An internal
[Olrox arena foundation](olrox-arena.md) now adds BO0 resources, common services
and paired door controls. Its selected [humanoid body](olrox-first-form.md) adds
original movement, physical damage and attack commands; selected
[orbs/portals/bats](olrox-orbs-portals.md),
[fireballs/effects](olrox-fire-effects.md) and [skull columns](olrox-skull-columns.md)
now advance. The selected [native encounter](olrox-encounter.md) connects real
transformation, articulated movement, head/arm attacks, shared damage and captured
defeat/reward. [Owned offscreen rendering](olrox-render-targets.md) preserves
one-tick captures when presentation skips a simulation frame. Selected
[furnishings and sitting](olrox-furnishings.md) now run. Full
presentation/audio/parity and further destinations remain incomplete.
This is not a complete authentic game. Further stages, complete area actors/scripts,
prologue initialization and full audiovisual/timing parity remain incomplete. Read the
[early castle batch review](early-castle-review.md) for the actual validation scope,
and [stage loading](castle-stage-loading.md), [Laboratory scope](laboratory-world.md),
[map](castle-map.md), and [session flow](native-session-flow.md).

The selected [Leap Stone double jump](player-leap-stone.md) now consumes saved,
active relic ownership through the normal-form player. Full dive-kick/Gravity Boots
parity, Bat/Wolf and full ability/progression parity remain incomplete. The selected
[dive-kick attack/rebound](player-dive-kick.md) now runs; original shared-factory
and attack-group contention still need integration.
Selected [Gravity Boots](player-gravity-boots.md) now add original command/MP,
high-jump/ceiling recovery and beam paths. A private-grant native route reaches
the NO2 secret ceiling through movement and ordinary Gladius attacks; it does
not establish ordinary acquisition, connected travel or whole-route parity.

The internal [transformation resource service](transformation-resources.md) now
preserves Bat/Mist/Wolf relic eligibility, strict MP thresholds and original
simulation-timer drain cadence. The selected
[Mist integration](player-mist.md) now connects original movement, collision,
capture/cloud/palettes and restoration through owned Q/Xbox LB controls.
Ordinary starts grant no relics. [Gas Cloud combat](player-gas-cloud.md) now runs
within its selected scope. Fire/Echo of Bat, Wolf, complete effects
and ordinary acquisition remain incomplete; component comparisons do not prove
whole-route or console parity.
The [Bat presentation foundation](player-bat-presentation.md) now has separate
artwork and real effect-controller checks. [Selected playable Bat](player-bat.md)
now connects owned entry, ordinary flight, collision, MP and actual restoration;
[Selected Wing Smash](player-wing-smash.md) now connects commands, owned attacks,
flash/trails, native damage/EXP and real return. Fire/Echo, Wolf and full Bat
parity remain incomplete.

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

## Next game-integration batches

1. Extend the initial Castle Entrance in this executable: stage-specific actors,
   entry/camera/audio behavior, and the Death event. Reuse verified stage assets;
   do not route players into an empty geometry-only area and claim it is complete.
2. Integrate original cross-stage routing and stage-aware ordinary progression,
   then connect Entrance/Laboratory and the next contiguous castle route. Keep
   unsupported destinations explicit until their behavior is available.
3. Bring in relic/ability gates, remaining weapons/status rules, Maria/castle
   scripts and bosses as connected progression batches. Each batch needs owned
   state, ordinary save compatibility, regression coverage, and private route checks.
4. Complete prologue/new-game/title flow, inverted-castle progression and endings,
   then establish complete desktop playthrough and audiovisual/timing parity.

Development starts and the native filesystem UI remain explicit scaffolding.
Use one batched validation pass per coherent integration, keep public tests free
of private inputs, and retain focused feature PRs. User playtest approval is
required before merging new game-integration PRs unless the user explicitly
changes that rule. Do not bulk-vendor the reference repository.

The [native sound service](laboratory-audio.md) now plays selected original
combat sound effects and Laboratory music; reverb and full audio parity remain incomplete.

The selected [Olrox monster body component](olrox-monster-body.md) now connects
articulated movement and head/arm commands. The selected encounter now supplies shared combat and captured defeat/reward;
full presentation/parity and connected travel remain incomplete.

The selected [Olrox encounter](olrox-encounter.md) now connects both reconstructed
forms, real morph/body/attacks, shared damage and captured defeat/reward. Pending
offscreen writes survive presentation gaps. Furnishings, full effects/audio/parity
and connected NO2 travel remain incomplete; this does not complete the castle or Alucard.
