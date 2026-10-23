# Project goals and release direction

SOTN Native exists to bring an authentic *Symphony of the Night* experience to
native PC platforms and create a clean foundation for modern conveniences.
The project is a personal and educational effort. It currently provides an
asset-free SDL3 foundation with private-data loading, OpenGL presentation, and
initial reconstructed game algorithms; player and stage gameplay remain in progress.

Users must supply their own game dump. Public source, tests, and releases must
contain no ROM, BIOS, extracted game binaries, or copyrighted game assets.

## Phase 1: establish the authentic foundation

Make the original game work correctly on desktop first. Preserve the experience
before extending it: gameplay rules, simulation timing, movement and combat,
input behavior, audiovisual presentation, progression, and ordinary game saves.
Use an explicitly identified source revision and region when comparing behavior.

Parity is a claim supported by evidence, not a successful launch or a single
playthrough. Grow deterministic replay tests, focused regression cases, and
documented manual checks as each subsystem becomes available. Keep expected
results and known differences explicit. Public CI uses synthetic or original
fixtures; private comparisons use the user's local dump and do not publish it.

Keep web and mobile authentic too. Adapt where the platform requires it: touch
controls, browser lifecycle/frame scheduling, sandboxed storage, and supported
graphics interfaces. Explain each concession in a feature/design document and
test that it preserves the underlying game behavior. Desktop is the initial
correctness reference, not permission to neglect the other targets.

All first-party native work uses C17 from the beginning. Where conversion needs
mechanical translation or generated C, isolate that code and establish tests
around it. Prefer readable code now; do not deliberately accumulate opaque code
just because a later cleanup milestone exists.

## Stable foundation and later enhancements

Keep releases on **0.x.x** while the project remains incomplete or unstable.
Release **1.0** when the authentic port is competent and stable, supported by
documented acceptance criteria and passing regression/platform checks. Do not
claim a stable game release for the bootstrap alone.

After proving that foundation, consider these enhancements in separate feature
branches, with their own requirements, tests, and compatibility decisions:

- Widescreen support without distorting gameplay or revealing invalid areas.
- Uncapped presentation without changing simulation timing or game logic.
- Embedded scanline and other presentation shaders.
- Save states and load states, distinct from the original game's save format.
- Cheats and other explicitly optional conveniences.

These are goals, not authorization to implement them during authenticity work.
Preserve a reference configuration and regression coverage when introducing them.
Decide feature defaults explicitly rather than silently changing the experience.

## Intended 2.0 milestone

The eventual goal is a human-readable, idiomatic C implementation throughout
the game core. **2.0** is the intended milestone for replacing transitional
generated or mechanically translated code while preserving proven behavior.
The project remains a C project throughout its life; 2.0 is a readability and
maintainability milestone, not a language migration from C++.

Apply the same authenticity tests during that rewrite. Keep save compatibility
explicit, document intentional breaking changes, and retain small, testable
module boundaries rather than replacing the code with an elaborate framework.

## Engineering reference

[AGENTS.md](../AGENTS.md) defines the working rules: secure C, initialized
state, arena lifetimes, SDL3 utilities and persistent paths, OpenGL with embedded
shaders, Xbox-layout gamepad support and optional vibration, sanitizer-backed CI,
Doxygen, and concise feature-branch pull requests.
