# Working in sotn-native

## Purpose and priorities

Build a native, personal and educational *Symphony of the Night* port that
preserves the original experience and creates a foundation for later improvements.
The C17 SDL3 app includes a limited Laboratory actor checkpoint, not a complete
working game port. `--laboratory` launches our executable with runtime data;
`--laboratory-room 3` selects the zombie corridor. Selected actors animate and
Bloody Zombies run active motion/attack animations. Selected empty-hand, Basilard/Gladius/Mace and ordinary Alucard sword/shield combat, physical damage, candle drops, and collection are
integrated; read [combat scope](docs/laboratory-combat.md). Other equipped weapons,
complete effects/audio, castle scripts, full map parity, and full progression remain incomplete.
Its original `modules/data/` library validates private core files; read
[docs/private-data.md](docs/private-data.md) before extending that boundary.
Its `modules/render/` library presents RGBA images through embedded OpenGL/ES
shaders; [docs/opengl-presentation.md](docs/opengl-presentation.md) describes
the asset preview and graphics checks. It does not implement PS1 game logic.
The `modules/core/` library now contains adapted reconstructed collision and
graphics-decoding algorithms. Read [docs/game-core.md](docs/game-core.md) for
their provenance and scope. Normal Alucard movement and collision sensors are
integrated; [movement integration](docs/laboratory-movement.md) describes their limits.
The core stage parser and on-demand data resources are documented in
[docs/stage-data.md](docs/stage-data.md); runtime tables must stay in private files.
Actor placement decoding and scrolling/persistence state are described in
[docs/laboratory-actors.md](docs/laboratory-actors.md), including selected behavior
and rendering integration. Do not describe visual attack animations as implemented combat.
Runtime tile/sprite presentation and the animation cursor are described in
[docs/laboratory-presentation.md](docs/laboratory-presentation.md). Its tile
preview is a graphics diagnostic, not a gameplay playtest.
Read [docs/project-goals.md](docs/project-goals.md) before planning conversion work.
The selected route is a **decompilation-based native port**; read the current
[feasibility report](docs/native-port-plan.md) and pinned
[reference manifest](config/reference-us.json) before importing code or data.

- **Phase 1: authenticity first.** Preserve gameplay, timing, controls, audiovisual
  behavior, progression, and save behavior. Prove the desktop experience first.
  Keep web and mobile authentic too; make only required platform concessions,
  such as touch controls where a physical controller or keyboard is unavailable.
  Document each concession and test that it preserves game behavior.
- Defer widescreen, uncapped rendering, scanline shaders, save/load states,
  cheats, and other enhancements until the authentic foundation is proven.
  Do not silently fix original behavior or mix enhancements into parity work.
- **C throughout.** Generated or mechanically translated C may be a temporary
  bridge during conversion. Isolate it, document its provenance and limitations,
  and test its behavior. The planned **2.0** milestone replaces such transitional
  code with human-readable, idiomatic C; it is not a future switch from C++ to C.
- This task history does not authorize every future conversion step. Scope work
  to the current request and report the actual implemented state accurately.

## Language, ownership, and safety

- Write first-party native code in **C17**, not C++. Use `src/main.c` as the
  single entrypoint. Platform-required Java/Objective-C glue and build/test
  scripts are acceptable; keep game logic in C. A dependency's platform backend
  may require C++ internally (SDL's Android HID backend does); this does not
  authorize first-party C++. Keep any required runtime statically linked.
- Favor simple, explicit code that a human can read and change safely. Apply
  SOLID through focused modules, small interfaces, explicit ownership, and
  injected dependencies. Use structs and function pointers where variation
  requires them; avoid speculative abstractions and object-system emulation.
- Initialize variables at declaration to useful defaults. Use `{0}` or checked
  zeroed allocation for aggregates; use explicit invalid sentinels where zero is
  a valid resource. Never rely on uninitialized storage or implicit ownership.
- Prefer arena allocation for groups of objects with a shared lifetime, such
  as a level or frame. Check capacity and alignment, define reset boundaries,
  and never retain pointers beyond an arena's lifetime. A small, separately
  owned allocation is appropriate when an arena would add needless machinery.
- Prefer SDL3's standard-library, allocation, logging, and I/O functions for
  portable behavior. Match each allocation with its allocator's release API.
- Use explicit types: fixed-width integers or SDL integer types for defined
  widths, `size_t` for memory sizes, and SDL's specified types at API boundaries.
  Do not assume the width of `int`, `long`, pointers, or enum storage. Validate
  narrowing conversions, byte order, alignment, and signed/unsigned arithmetic.
- Follow secure C practices. Validate external input at boundaries; check sizes,
  overflow, offsets, indices, allocation results, and I/O results before use.
  Avoid unchecked copies, unbounded formatting, format-string injection,
  use-after-free, and accidental aliasing or lifetime dependencies.
- Fail fast with clear, actionable error messages and explicit failure results.
  Include the operation and relevant context; clean up partial initialization.
  Do not continue with corrupt state or hide errors behind a successful result.
- Keep warnings as errors for project code. Do not add warning suppressions to
  conceal defects. Use the checked-in Microsoft clang-format layout.

## Platform behavior

- Link SDL3 statically from an integrity-checked, pinned source release. The
  Android JNI shared library contains static SDL3; OS libraries remain external.
- Use **OpenGL** for the game renderer, with OpenGL ES/WebGL adaptations where
  the platform requires them. The bootstrap's SDL debug-text display is not the
  final renderer. Do not replace the rendering direction with another backend
  without a specific requirement to do so.
- Embed shader source as C `.h` headers or byte arrays and compile/link it at
  runtime. Ship no filesystem-dependent shaders. Validate capabilities and
  report shader compile/link errors clearly; keep desktop and ES variants explicit.
- Keep original simulation timing independent from presentation. Browser frame
  pacing belongs to SDL's callback loop; do not block the browser with a game loop.
- Put saves, settings, and persistent state under a location obtained through
  **`SDL_GetPrefPath`**. Never assume the working directory or install directory
  is writable. Check directory and file operations, protect existing saves
  against failed writes, and test format migrations. Web persistence needs an
  explicit persistent-storage adapter and reload tests.
- Use SDL's gamepad abstraction with a ubiquitous **Xbox-layout controller** as
  the default reference. Preserve the original game actions, document mappings,
  and support rebinding. Prefer A/B/X/Y, shoulders, triggers, d-pad, and sticks
  over raw device indices. Use vibration when supported; lack of vibration must
  never prevent play. Keep desktop keyboard and required mobile touch fallbacks.

## Tests, CI, and builds

- Automated tests are essential. Add regression coverage for behavior changes,
  failure paths, parsing, ownership, and platform adaptations. Fix failures;
  do not weaken tests to obtain a green build.
- Use reproducible input/replay comparisons to establish authenticity. Preserve
  simulation results independently of render rate. Use synthetic or original
  test fixtures in public CI. Asset-dependent parity tests use the user's local
  dump; report missing-input tests as skipped, never as proof that parity passed.
- Keep Linux, macOS, Windows, browser, Android, and iOS build/runtime checks
  working. Run **AddressSanitizer in Linux CI**, with UndefinedBehaviorSanitizer
  alongside it. Treat sanitizer reports as bugs and retain failure diagnostics.
- CMake owns build logic; GNU Make wraps it. Use `make`, `make test`,
  `make sanitizer` (Linux), `make package`, and platform targets. List sources
  explicitly; reusable code belongs in focused modules as the project grows.
- Pin dependencies and avoid competing utility stacks where SDL3 suffices.
  Build and release packaging must not require private game inputs.

## Source and release policy

- Never commit or publish ROMs, BIOS images, extracted game binaries, original
  game source dumps, or copyrighted game assets. Users supply their own game
  dump. Keep private inputs outside the checkout or in ignored `local/`.
  Document source/tooling provenance and compatible licensing for public code.
- Run `python3 scripts/check_repository.py` and review the staged diff before
  committing or publishing. Do not weaken the guard to admit game data. Keep
  private data out of CI artifacts, logs, release archives, and test fixtures.
- Use **Semantic Versioning**. Edit root `VERSION` only. Stay on **0.x.x** until
  the project provides a competent, stable release; **1.x.x** represents that
  stable foundation. The intended **2.0** milestone is the idiomatic C rewrite.
  Version saves and state formats explicitly; do not infer their compatibility
  from the application version alone. Annotated release tags `vX.Y.Z` must match
  `VERSION`; a feature branch does not itself require a release or version bump.

## Documentation and collaboration

- Maintain an idiomatic, accurate README for setup, build, play, tests, and
  packaging. Put feature requirements, design decisions, parity criteria, and
  platform concessions under `docs/` and link them from relevant entrypoints.
- Use professional **Doxygen** comments for source interfaces: purpose,
  parameters, returns, ownership/lifetimes, preconditions, and failure behavior.
  Internal comments should explain durable invariants or tradeoffs, not narrate
  statements or serve as a change log.
- Work on **feature branches** and use **`gh` to open pull requests**. Keep each
  PR focused and reviewable; do not merge unless the user authorizes it.
- Current game-integration work uses stacked PRs. Leave them unmerged until the
  user playtests and verifies the project build; merge dependencies first only
  after that approval. This supersedes earlier blanket permission to merge on CI.
  Exception: for the overnight batch explicitly authorized on 2026-10-25, the user
  waived manual playtests and authorized merging passing PRs bottom-up. Continue
  automated/private validation; this exception is not an authenticity claim and
  does not waive the normal gate for later work after that batch.
- Give game build/run instructions through **LLDB**, so a crash leaves a useful
  stack trace (`thread backtrace all`). Playtests must launch this repository's
  executable built through its CMake/Make workflow.
- Be succinct and use active voice in responses and PR descriptions. Generally
  write PRs as **context > problem > solution**, emphasizing user experience.
  Include a brief validation result. Leave implementation details in the code
  unless a reviewer needs them to understand a material tradeoff.

The [physical death/game-over flow](docs/laboratory-game-over.md) now runs the selected
original presentation and returns to the native Continue shell after its fade. Original title/file selection,
elemental deaths and complete effects remain incomplete. The
[save-room checkpoint](docs/laboratory-save-rooms.md) now restores HP/MP and persists
supported progress to fifteen native slots; `--laboratory-continue SLOT` loads one.
The native slot browser supports Continue; original title/card menus, full-game progression
persistence, and PS1 cards remain incomplete.

The [original HUD](docs/laboratory-hud.md) presents HP, MP, hearts, and the collected
subweapon from private artwork/tables. Supported progression now updates maxima and MP recovery.

The [native sound service](docs/laboratory-audio.md) now plays selected original
combat sound effects and Laboratory music; reverb and full audio parity remain incomplete.

The [character inventory](docs/character-inventory.md) now connects original pickups,
selected pause/equipment panels, body stats, physical defense, and cape colors.
Spells/relic/settings/familiar menus remain unavailable. Version 8 saves retain this
supported progress; versions 1/2/3/4 migrate explicitly from their earlier profiles.

[Character progression](docs/character-progression.md) now awards level-adjusted
enemy EXP, original level/resource/stat gains, selected MP recovery, and permanent
Heart/Life Max Up collection. Complete level-up effects and familiar EXP remain incomplete.

The [selected subweapon service](docs/laboratory-subweapons.md) integrates dagger, axe,
holy water, and stopwatch; other collected subweapons remain explicitly unsupported.
Named `--laboratory-test-profile` fixtures are development grants, not original new-game starts.

Version 8 [ordinary native saves](docs/native-saves.md) now retain supported character
inventory/equipment, progression/maxima, permanent Laboratory pickup bits, and four
owned RNG streams, explored map cells, and permanent Laboratory events. Versions 1/2/3/4/5
migrate explicitly. Transient actor/projectile state, full castle scripts, and PS1
memory-card support remain incomplete.

The [Laboratory traversal milestone](docs/laboratory-world.md) now adds Skeleton,
Bone Scimitar, Blood Skeleton, Axe Knight, and Spittle Bone behavior/projectiles.
Room switches/spikes, boxes, lifts, selected doors, cannon/secret obstacles now run.
Normal camera follow and room locks now advance independently of rendering.
The paired boss encounter now includes attacks, coordination, defeat, reward, and music changes.
Ordinary saves retain map exploration and permanent Laboratory events. Full map parity, Maria/castle
scripts, whole-encounter replay parity, full effects/audio, and other stages remain incomplete;
do not call the area complete.

[Castle stage loading](docs/castle-stage-loading.md) now verifies 50 canonical stage
overlay/artwork pairs on demand. `--inspect-stage CODE` is a headless data diagnostic,
not native gameplay support or permission to send the player to an unintegrated stage.

The [exploration map](docs/castle-map.md) now opens with Tab/M or Xbox Back/Select.
It uses private blueprint/palette/passage data and saved exploration; full map and
paused-stage presentation parity remain incomplete. Missing F_MAP.BIN fails explicitly on first use.

The [native session shell](docs/native-session-flow.md) opens with `--play` and
returns after game over. It supports read-only slot selection and transactional
Continue, not the original title/new-game flow. Failed loads preserve the current
owner and display an error. Future stage destinations still need native actor/script integration.

The first [Entrance integration slice](docs/entrance-runtime.md) adds a stage-65
development entry (`--entrance`, `--entrance-room 0..27`), bats, floor zombies,
Wargs, both Mermen and their selected projectiles/splashes, and shared actors.
[Death's selected event](docs/entrance-death.md) now runs with original speech,
private portraits/text, targeted theft, and return of player control. Unused Fire
Wargs and additional later-Entrance enemies remain unintegrated.
[Connected stages](docs/connected-stages.md) now travel Entrance → Laboratory →
later Entrance with character/world/time/RNG retention and stage-aware saves. Keep stage identities
and original actor IDs distinct; do not describe the Entrance as complete.

[Original opening equipment](docs/opening-equipment.md) now equips the first Entrance
checkpoint with Alucard's six original items and integrates ordinary sword/shield
behavior, Dragon Helm defense reduction, Twilight glow, and actual equipped theft.
Prologue-derived stats, starting relics, command attacks, shield-rod spells and full
presentation remain incomplete; do not call this original new-game initialization.

The [relic progression service](docs/relic-progression.md) collects original stage orbs,
retains found/active ownership in v6 saves, and gates candle drops on Cube of Zoe.
Selected permanent pickups retain original castle bits across stage changes.
Relic movement abilities, complete acquisition effects, and the relic menu remain incomplete.

The [Laboratory Maria encounter](docs/laboratory-maria.md) now reads private scripts,
animates Maria and her door, streams five speech clips, pauses/resumes music, and
returns control. The met-Maria flag persists in ordinary saves. CD timing parity,
complete dialogue effects, and full castle scripts remain incomplete.

The [early castle profiles](docs/early-castle-stages.md) add Marble Gallery/Outer Wall
(stage 0/1), shared Skeletons, Marble green Axe Knights, stage music, ordinary saves
and connectors. New per-area enemies, mechanisms, boss transfers and whole-route
parity remain incomplete; these are development profiles, not completed areas.

Selected [Marble encounters](docs/marble-gallery-enemies.md) and
[linked enemies](docs/marble-linked-enemies.md) now include Flea Men, Slingers, Ouija
Tables, Stone Roses, Skelerang and Diplocephalus with selected projectiles and death
behavior. Public synthetic and private explicit-room LLDB checks cover these paths;
continuous-route/console parity and complete area actors/effects remain incomplete.

The [early Marble clock room](docs/marble-clock-room.md) now runs timed statues,
stopwatch gating, hands, birdcages, selected floor/shadow/pendulum behavior, and clock
audio with original volume/pan. The later ring ritual and full effects/priority/timing
parity remain incomplete. Stage services consume elapsed ticks, not whole seconds.

Selected [Outer Wall ranged encounters](docs/outer-wall-ranged-enemies.md) now run
coordinated Bone Musket groups and Bone Archers, shots/arrows, and selected death
paths. Other Wall enemies/mechanisms, status damage and whole-route parity remain
incomplete. Private native checks use explicit protected room fixtures.

The [Outer Wall Spear Guard](docs/outer-wall-spear-guard.md) now adds selected melee,
blocking, thrown-spear and death rules. The copied player state exposes the original
selected four-tick subweapon startup interval for enemy decisions. Other unsupported
actions/status effects remain absent; private native checks use a protected fixture.

The [Outer Wall Sword Lord](docs/outer-wall-sword-lord.md) now runs selected attacks,
its separate weapon hitbox, bounded dust, death pulse and falling/fading fragments.
Private explicit-room checks use a protected player; whole-route and console parity
remain incomplete. Original tables and assets stay private.

The [Outer Wall Blue Axe Knight](docs/outer-wall-blue-knight.md) now runs selected
armored/damaged attacks, its separate weapon actor, axes and both death variants.
Green/blue fragment deaths retain paired primitive reservations. Private protected
fixtures are not continuous-route or console parity evidence.

The [Outer Wall elevator](docs/outer-wall-elevator.md) now runs selected three-stop
rides, remote calls, door tile/geometry changes, strike activation, gears and pressure
platform, with two persistent v6 mechanism flags. Sound supports channel 22/23 gain
updates without retriggering loops. Complete elevator effects/priority and whole-route
parity remain incomplete. Private protected fixtures are not console parity evidence.

The [Entrance water-room rocks](docs/entrance-rocks.md) now integrate two-hit breaks,
selected debris, the food drop, foreground/background tile changes and persistent
NO3/NP3 broken-side flags. Background tiles have their own owned room copy. The
later wolf/bat ritual remains unavailable; never grant its prerequisite bits implicitly.

The [Entrance stair section](docs/entrance-stairway.md) now runs its sixteen-HP break,
food drop, selected tile/debris effect and permanent restoration in NO3/NP3. Tile
art metadata is borrowed per room; room entry clears it. Private protected fixtures
remain distinct from route/console parity evidence.

The [Outer Wall Skeleton Apes](docs/outer-wall-skeleton-ape.md) now run selected
punches, carried/thrown barrels, bouncing fragments and staged deaths. Child
ownership uses slot/generation checks; optional actor pools have bounded fallbacks.
Full effects, whole-route and console parity remain incomplete.

The [early Short sword](docs/short-sword.md) now equips an ordinarily collected
Entrance item and runs selected original attacks in either hand. The family-37
view loads five palettes. Jewel/Stone sword special paths remain unavailable;
ordinary travel and Continue grant no replacement gear.

Selected normal-player [vertical room crossings](docs/early-castle-review.md) use
original early thresholds and upward/downward overlap adjustments. Horizontal
crossings keep priority. Camera-edge locks and transformation rules still require
parity work; a successful route is not proof of complete area or console parity.

The selected [Outer Wall Armor Lord](docs/outer-wall-armor-lord.md) now runs sword,
fire-wave, shield and staged death paths with checked actor/primitive ownership.
Its killed flag lasts for the loaded overlay, not forever in ordinary saves.
The copied player state distinguishes four-tick hand-weapon and subweapon startup:
Armor Lord observes the hand-weapon signal; Spear Guard observes both.
Contact retains element information and applies equipment element modifiers;
ailments, special damage reactions, elemental death presentation and original
contact priority remain incomplete. Death ribbons/lighting are bounded native
adaptations with explicit parity gaps. Protected component fixtures are not
uninterrupted-route or console evidence.

Selected [petrification](docs/player-petrification.md) now connects stone contact,
recoil/landing, directional escape, private palettes/wiggle and a selected lethal
completion to game over. Pause, weapons and save-room activation stay disabled.
Statue/dissolve children, rare variants, full lethal branches, fragments and
complete presentation/parity remain incomplete. The protected native fixture
injects hits; it is not evidence for ordinary Medusa encounters or full-game parity.

The selected [Outer Wall Medusa Heads](docs/outer-wall-medusa-heads.md) now use
private spawner profiles, bounded pools, original blue/gold flight and stone
contact. Gold heads reach the selected petrification service, whose statue and
complete death presentation remain incomplete. Native checks use protected room
starts and positioned contact, not ordinary traversal or console parity evidence.

The [Doppleganger arena](docs/doppleganger-arena.md), [runtime](docs/doppleganger-runtime.md)
and [entry/reward flow](docs/doppleganger-entry.md) now run selected native AI/body,
hand/knife/wing combat, physical defeat, placed entrance effects, block seals,
music and Life Max Up collection. Original Outer Wall boss overrides now select
BO4 before ordinary rooms; v7 saves retain all 32 original time-attack records and
visited bits. Versions 1–6 migrate explicitly; v4–6 Laboratory seconds convert to
event 6 HHMMSS. Continue starts supported save rooms at their center. BO4 has no
save rooms or separate public gameplay start. Mist/palette, high jump, stone,
complete elemental effects and whole-route/console parity remain incomplete.
Private protected room/route fixtures are not an unmodified-character playtest.

The [Long Library runtime](docs/library-runtime.md) now includes its own private
resource/table profile, original music, selected lamps/urns/jugs/busts, Flea/Flea
Armor, Dhuron, Thornweed/Corpseweed, Ectoplasm and Spellbook/Magic Tome encounters.
Bodies, owned children, beams, staged deaths and selected effects use Library
identities/tables. Re-entry replaces only matching owned children. Retained Flying
Zombie/body/blood adapters have explicit fixtures; original first-Library layouts
contain no ordinary Flying Zombie placements. Do not count those fixtures as route
coverage or reuse another overlay's raw IDs/tables.

[Lesser Demon](docs/library-lesser-demon.md) now has selected AI/attacks/summons,
physical defeat and [placed arena flow](docs/library-demon-entry.md): interpolated
camera lock, sealing books, music, defeat record and Life Max Up. Selected
[children](docs/library-demon-attacks.md) and [minions](docs/library-minions.md)
retain owned state and simulation-frame phase. The US initial bolt vertex has an
explicit safe-initialization concession. Protected/injected checks do not prove
whole-encounter or console parity.

The [Librarian introduction](docs/library-librarian.md) runs alignment/step49 hold,
chair, six initial voices and meeting/time record. [Buy/sell](docs/library-shop.md)
adds private catalogues/unlocks, quantities, paging, atomic equipment/gem/relic/map/
scroll transactions, speech, exit and returning greeting. v8 ordinary saves retain
learned-spell mask/order and game-clear unlock with explicit v1–v7 migrations.
Full detail/preview panels, tactics, enemy list, sound test, spell casting and
missing equipment effects remain incomplete. Chair gifts have injected-contact
checks; Gravity Boots/forms are separate. The [SEQ service](docs/library-sequence-audio.md)
keeps sequence and XA owners independent; console PCM/reverb parity remains unproven.

[Library furnishings](docs/library-furnishings.md) include the candle table/shadow,
permanent Stone Mask pickup, selected equipment metadata and private decoration
records. [Backgrounds](docs/library-backgrounds.md) include repeating models,
shadows, distant lighting grids, owned backdrop and foreground occlusion. The
[secret bookcase](docs/library-secret-bookcase.md) retains original push delay,
rotation/collision/lighting/subdivision and discovery flag 112. v8 stores discovery,
not transient pose/momentum. Keep all model/lighting/lookup data private. Original
ordering tables advance forward: higher priorities draw later; do not restore the
old descending renderer assumption. Stable native ties are not full GPU emulation.

[Curse](docs/character-status.md) and [poison](docs/player-poison.md) retain original
constitution-adjusted duration and independent transient timers. Curse blocks
equipped weapons/shields with original failure animation/sound; empty hands and
selected subweapons remain available. Poison halves new hand-attack power and
doubles ordinary incoming damage. Inventory pause freezes timers; room/stage
arrivals retain them; fresh ordinary Continue clears them. Full ailment visuals,
medicine and general elemental/damage-kind parity remain incomplete.

[Connected Library travel](docs/library-connections.md) now follows the original
Outer Wall connector in both directions. Teleport arrivals initialize actor-owned
red-door tiles before first player collision; direct development starts retain
their existing priming order. Missing destination resources preserve the previous
session/save store and show an error. Ordinary-character private checks cover
three crossings, stairs/doors, ownership and isolated save/Continue, with explicit
start/save-room transfer/discovery fixture. They are not whole-castle or console
replays. Sitting, Mist interaction, full shop services, full effects/audio and
complete Library/game progression remain incomplete. Do not call the area complete.

The [Royal Chapel foundation](docs/chapel-runtime.md) adds a stage-6 resource profile,
selected shared actors, Winged Guard/body pieces/spawner and Bat combat. Chapel
travel remains disabled; other enemies, lamps, mechanisms, Maria and boss remain
unintegrated. Private controlled native fixtures are not ordinary-route/console parity.
