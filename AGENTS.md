# Working in sotn-native

## Purpose and priorities

Build a native, personal and educational *Symphony of the Night* port that
preserves the original experience and creates a foundation for later improvements.
The C17 SDL3 app includes a limited Laboratory actor checkpoint, not a complete
working game port. `--laboratory` launches our executable with runtime data;
`--laboratory-room 3` selects the zombie corridor. Selected actors animate and
Bloody Zombies run active motion/attack animations. Combat/damage, drops, sound,
other actor/effect families, saves, scripts, and full progression remain incomplete.
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
- Give game build/run instructions through **LLDB**, so a crash leaves a useful
  stack trace (`thread backtrace all`). Playtests must launch this repository's
  executable built through its CMake/Make workflow.
- Be succinct and use active voice in responses and PR descriptions. Generally
  write PRs as **context > problem > solution**, emphasizing user experience.
  Include a brief validation result. Leave implementation details in the code
  unless a reviewer needs them to understand a material tradeoff.
