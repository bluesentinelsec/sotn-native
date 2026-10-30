# SOTN Native

A C17/SDL3 native-port project for personal and educational study of
*Castlevania: Symphony of the Night*. **Game integration is in progress.**
The project's executable now runs a limited combat checkpoint in the Alchemy
Laboratory: Alucard movement, adjacent room transitions, animated candles,
red-eye busts, globe tables, selected empty-hand/Basilard/Gladius/Mace combat, candle pickups,
the original Alucard HUD, physical death/game over, native save-room support,
and original sound effects/Laboratory music.
[Laboratory traversal](docs/laboratory-world.md) includes skeletons, Axe Knights, Spittle Bones,
room switches/spikes, movable boxes, lifts, doors, and cannon/secret obstacles.
Development rooms 17 and 22 expose the box puzzle and three-stop lift.
Normal camera follow/locks and the Slogra/Gaibon encounter are integrated.
Ordinary saves retain explored cells and permanent Laboratory events; this remains a selected checkpoint.
[Selected subweapon attacks](docs/laboratory-subweapons.md) add heart spending and projectile combat.
[Inventory and equipment](docs/character-inventory.md) connect owned pickups to combat stats;
[character progression](docs/character-progression.md) awards EXP and level/resource gains.
[Exploration map](docs/castle-map.md): Tab/M or Xbox Back shows saved exploration.
Other equipped weapons, complete damage/item/audio behavior, castle scripts, actor/effect
support, stage scripts, and full-game progression remain incomplete. This is not yet an
authentic playable game build. Its default mode still displays the bootstrap greeting.

[Castle stage loading](docs/castle-stage-loading.md) verifies and inspects 50 stage
overlay/artwork pairs on demand. Selected native paths now connect Entrance, Laboratory,
Marble Gallery, Outer Wall and selected Long Library paths; further areas remain unintegrated.
[Doppleganger arena adapters](docs/doppleganger-arena.md) prepare private BO4
resources. Selected [boss combat](docs/doppleganger-runtime.md#stage-combat-orchestration)
now joins its placed entrance, seals and reward through selected Outer Wall progression;
[scope and validation](docs/doppleganger-progression.md) remain explicit.
[Long Library travel](docs/library-connections.md) now connects to Outer Wall, with
selected enemies, Lesser Demon, Librarian buy/sell, furnishings and ordinary saves.
[Curse](docs/character-status.md) and [poison mechanics](docs/player-poison.md) run;
full ailment effects, shop panels, remaining interactions and whole-route parity
remain incomplete.
The [Royal Chapel foundation](docs/chapel-runtime.md) adds internal resource and
Winged Guard/Bat, breakable/debris and furnishing adapters; Chapel travel remains
disabled pending further integration.
Selected [chairs and confessional encounters](docs/chapel-confessional.md) now run
with original ghost/blade branches and private chime audio.
The [Chapel Maria event](docs/chapel-maria.md) also runs selected original dialogue,
speech, camera pan and departure, retaining its completion in ordinary saves.
The [packed sprite adapter](docs/doppleganger-sprites.md) renders its backed body/bat
frames without establishing boss gameplay or full effects parity.
Its [collision and decision services](docs/doppleganger-runtime.md) now preserve
boss-specific sensors and simulated control choices. Selected bat/return controls and
[blink effects](docs/doppleganger-runtime.md#transformation-controls-and-blink-child)
now feed the selected stage combat path; full encounter and console parity remain unproven.

[Native Continue flow](docs/native-session-flow.md): `--play` opens slot selection;
game over returns there without restarting the process. Original title/new-game
flow and full castle progression remain incomplete.

[Early castle progression](docs/early-castle-stages.md) connects selected Entrance,
Laboratory, Marble Gallery and Outer Wall paths, with original opening equipment,
Death's theft, collected Short sword/relic progression, Maria, selected encounters,
clock mechanisms and the Wall elevator. See the [batch review](docs/early-castle-review.md)
for validation evidence and remaining progression limits. The selected
[Armor Lord](docs/outer-wall-armor-lord.md) adds sword/fire-wave/shield combat;
its death rendering and elemental reactions retain documented parity gaps.
[Selected petrification](docs/player-petrification.md) adds stone landing,
directional escape and a selected game-over path; statue/fragment effects remain incomplete.
[Wall Medusa Heads](docs/outer-wall-medusa-heads.md) add blue/gold spawning,
flight and stone contact within those limits.

[Royal Chapel travel](docs/chapel-connections.md) now connects Laboratory to its
selected encounters, save rooms and [Hippogryph](docs/hippogryph-body.md), including
the reward, Maria dialogue/speech and return route. Other castle destinations,
whole-route replay and full audiovisual/timing parity remain incomplete.

An internal [Olrox's Quarters foundation](docs/olrox-quarter-runtime.md) adds its
own resources, selected shared actors, [prisoners and spikes](docs/olrox-prisoners-spikes.md).
Selected [Valhalla Knight combat](docs/olrox-valhalla.md) and articulated
[Hammer/Blade encounters](docs/olrox-giant-knights.md), plus NO2’s
[fifteen-weapon Spectral Sword](docs/olrox-spectral-sword.md) and selected
[secret wall/ceiling mechanisms](docs/olrox-secrets.md), also run.
[Selected connected travel](docs/olrox-connections.md) now links Marble Gallery,
Chapel and its original boss approach. The internal
[Olrox arena foundation](docs/olrox-arena.md) adds its resources and paired door
controls. Its selected [humanoid body](docs/olrox-first-form.md) now moves, takes
physical damage and issues attack commands, with selected
[orbs/portals/bats](docs/olrox-orbs-portals.md) and
[fireballs/effects](docs/olrox-fire-effects.md) and [skull columns](docs/olrox-skull-columns.md)
now active. The selected [Olrox encounter](docs/olrox-encounter.md) connects the real
transformation, monster combat, captured defeat/reward and door release.
[Owned offscreen rendering](docs/olrox-render-targets.md) preserves its captures
across presentation gaps. [Breakable furnishings and sitting](docs/olrox-furnishings.md)
now run. Full effects/audio/parity and further destinations remain incomplete.

No ROM, BIOS, extracted game data, artwork, or audio is distributed.
The combat checkpoint requires users to supply their own supported game dump.
This project is unofficial and is not affiliated with Konami or Sony.

The project uses **C17**. Phase 1 preserves the original game experience, with
desktop correctness established first and required web/mobile adaptations kept
explicit. Modern conveniences follow a proven authentic foundation. See
[project goals](docs/project-goals.md) for release direction and
[AGENTS.md](AGENTS.md) for engineering and collaboration standards.

The selected approach is a **decompilation-based native port**. The initial
[feasibility report and milestones](docs/native-port-plan.md) describe the
pinned reconstructed C reference, private input checks, and remaining integration
work. Build and playtest this repository's executable through CMake/Make.

Native integration has started with the reconstructed collision and graphics
decoding algorithms in `modules/core/`. These compile and run tests through
this repository's normal build. See
[game-core integration](docs/game-core.md) for scope, provenance, and validation.
The [runtime stage loader](docs/stage-data.md) decodes Laboratory room/layer
tables directly from the user's validated overlay and verifies its graphics file
on demand; `--inspect-laboratory` exercises that boundary without launching video.
The [Laboratory presentation diagnostic](docs/laboratory-presentation.md) now
shows original room tilemaps through this executable, using `--preview-laboratory`.
It does not run player or enemy gameplay.
The [actor integration](docs/laboratory-actors.md) loads original room placements,
graphics, and selected behaviors. Its inspection mode validates every layout.

## Laboratory combat checkpoint

Build and launch this project's Debug executable through LLDB (macOS):

```sh
make debug-build && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data /path/to/private/extracted/us --laboratory --laboratory-room 3
```

Use arrows to move/look/crouch, **X or Space** to jump, and Escape to quit.
Xbox controls use the d-pad or left stick and **A** to jump. **Z/D** or
Xbox **X/B** use the equipped hands (default: Gladius left, empty right).
In normal form, **A/Xbox Y** performs the original [backdash](docs/player-backdash.md),
with its movement/cancel timing, afterimages and selected movement puffs.
Cutting hits on supported blood-bearing enemies now render original
[blood particles](docs/enemy-blood.md). For [Dark Metamorphosis](docs/player-dark-metamorphosis.md), enter Back,
Up-back, Up, Up-forward, Forward, then either hand attack; directions follow the
facing captured at the first input. It costs 10 MP. Move into blood to absorb it.
[Summon Spirit and Tetra Spirit](docs/player-spirit-spells.md) now cast with their
original MP, portal/freeze, artwork and enemy damage. Summon costs 5 MP: Back, Up,
Down, then Down+attack. Tetra costs 20 MP: hold Up for about half a second, then
Up-forward, Forward, Down-forward, Down, then Down+attack. These remain selected
development paths. [Hellfire](docs/player-hellfire.md) costs 15 MP: Up, Down,
Down-forward, Forward, then either hand attack. Hold Up during casting for large
fireballs; Down cancels at the reappearance/attack boundary.
[Soul Steal](docs/player-soul-steal.md) costs 50 MP: Back, Forward, Down-forward,
Down, Down-back, Back, Forward, then either hand attack. Its healing orbs return HP
on contact. Ordinary starts retain their existing MP maximum. Other normal spells
and complete Alucard remain unfinished.

**Enter/Xbox Start** opens the supported pause/equipment menu when idle; X/Space
or Xbox A selects and keyboard A/Xbox Y goes back. **Q/Xbox LB** toggles an owned,
active [Mist form](docs/player-mist.md). Owned, active
[Gas Cloud](docs/player-gas-cloud.md) adds damage to that form; these starts grant
no abilities.
**E/Xbox RB** toggles owned, active [Bat flight](docs/player-bat.md), using
original steering, MP and real transformation effects. For [Wing Smash](docs/player-wing-smash.md),
hold Jump through Up, Up-back, Back, Down-back, Down, Down-forward and Forward,
then release Jump; back/forward follow facing. With owned, active
[Fire of Bat](docs/player-fire-of-bat.md), Z/D or Xbox X/B fires while flying.
With owned, active [Echo of Bat](docs/player-echo-of-bat.md), A or Xbox Y emits
a wave; owned Force of Echo adds its original attack. **R/Xbox RT** toggles owned
[Wolf](docs/player-wolf.md); Z/Xbox X bites. Owned Power of Wolf enables faster
double-forward sprinting; Skill of Wolf enables Down, Down-forward, Forward then
bite to charge. [Direct form conversions](docs/player-wolf-cross-forms.md) retain
original gates and snapshots. Wolf water paths, other spells and full ability parity
remain incomplete.
At a crash, enter `thread backtrace all` in LLDB and share its output.

[Entrance water actors](docs/native-water-actors.md) now supply original splashes,
wakes, drops, currents and retained depths. Selected [player water feedback](docs/player-water.md)
adds gravity/drag, form gates, Wolf swimming and bubbles. Original
[normal breathing bubbles](docs/normal-water-bubbles.md) now run too. The
[water damage/death controllers](docs/water-damage-death.md) have scoped component
checks; native water damage and death remain unfinished.

The command starts directly in development room 3, with zombies and candles.
Use `--laboratory-loadout dual` for Gladius/Mace, `mace` for Mace/empty,
`sword` for Gladius/empty, or `unarmed` for the earlier test. Inventory menus
and other weapons remain incomplete.

Omit `--laboratory-room 3` to use the earlier room-1 entry start. These starts
do not run the title screen or original progression. The development combat profile supports zombie damage/death, candle breaks,
and heart/gold/subweapon collection. Physical death now runs the
[death/game-over presentation](docs/laboratory-game-over.md); Enter/Xbox Start
advances after the reveal. The checkpoint exits successfully after its fade,
because title/file selection remain incomplete. Load an ordinary room save with
`--laboratory-continue SLOT`. See
[combat integration](docs/laboratory-combat.md) for the development loadout and limits.
The [original HUD](docs/laboratory-hud.md) shows HP, MP, hearts, and the subweapon.
Unsupported actor families emit a warning and are omitted from this checkpoint.
The camera and fixed 60-tick pacing are development
adapters. See [actor integration](docs/laboratory-actors.md) and
[movement integration](docs/laboratory-movement.md) for behavior and known gaps.
An owned, active [Leap Stone](docs/player-leap-stone.md) now enables the original
double jump. Development starts do not grant it; other abilities remain incomplete.
The selected [dive kick](docs/player-dive-kick.md) adds attacks and rebounds after
that jump; full ability and shared-factory parity remain incomplete.
Owned [Gravity Boots](docs/player-gravity-boots.md) now add the original command,
MP cost, high jump and selected ceiling recovery/beam. Ordinary relic acquisition
and complete ability/castle parity remain acceptance work.
The [Relics pause panel](docs/relic-menu.md) now switches owned Cube of Zoe,
Soul of Bat, Soul/Power/Skill of Wolf, Gravity Boots, Leap Stone, Jewel of Open,
Mist, Power of Mist and Gas Cloud, retaining active choices in saves.
Other relic abilities and familiar controls remain incomplete.
The [save-room checkpoint](docs/laboratory-save-rooms.md) restores HP/MP and saves
supported progress to fifteen native slots. Use `--laboratory-room 23` for a direct
save-room start, press Up at the point, then follow confirmation prompts. X/Space
or Enter confirms; A cancels (Xbox A/Y). Relaunch with `--laboratory-continue 1`
to load slot 1. See the [storage format and platform limits](docs/native-saves.md).
Public builds and tests need no game data.

The C app can now read and validate five core files from an extracted US dump:

```sh
# macOS; use build/debug/bin/sotn-native on Linux or sotn-native.exe on Windows.
lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native \
  --game-data /path/to/private/extracted/us --validate-only
```

This command exits before graphics initialization and returns failure for
missing, wrong-sized, or checksum-mismatched files. Omit `--validate-only` to
retain the validated snapshot while the bootstrap runs. This does not launch
gameplay. See [private data loading](docs/private-data.md) for the supported
files, ownership contract, and current limits. Build-only targets, tests and
packaging need no game inputs.

The OpenGL presentation slice can also display original save icons from those
validated files. It is an asset preview, not gameplay:

```sh
lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native \
  --game-data /path/to/private/extracted/us --preview-save-icons
```

Use Left/Right to select an icon, resize the window to check scaling, and Escape
to quit. Each icon shows its three original frames stacked vertically. Run
the executable with `--graphics-test` for real framebuffer checks without game
data. See [OpenGL presentation](docs/opengl-presentation.md) for requirements.

## Desktop

Requires CMake 3.24+, a C17 compiler, GNU Make, Python 3, and the platform SDK.
The first configure downloads checksum-pinned **SDL 3.4.18** from its official
release. On macOS, install Xcode's command-line tools; on Windows, use a Visual
Studio developer shell. Windows users can run the CMake commands directly.

```sh
make                 # Build Debug and start at Castle Entrance
make dev             # Same as make
make release         # Build Release and start at Castle Entrance
make run             # Build and start at Castle Entrance
make debug           # Same start through LLDB; print all-thread backtraces on a crash
make test            # Run the app's headless render smoke test and repository guard
make sanitizer       # Linux: run tests with ASan and UBSan, including SDL instrumentation
make package         # Build/test Release and produce a versioned ZIP in dist/
```

All launch targets use `local/game-data/us`; override it with
`SOTN_GAME_DATA=/path/to/your/data make run` (or `make debug`). LLDB is required
for `make debug`; closing the game returns to the shell. These targets start the
current Castle Entrance checkpoint with its opening event, without room/gear test
fixtures. The original title/prologue/new-game flow remains incomplete.
`make debug-build` and `make release-build` build without launching.

Use `make debug` or the LLDB command above for game playtests. On Linux, use
`build/debug/bin/sotn-native`; on Windows, use `build/debug/bin/sotn-native.exe`
(or the configuration subdirectory produced by your generator).

CMake is the source of build logic; Make is a convenience wrapper:

```sh
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug --config Debug --parallel 4
ctest --test-dir build/debug -C Debug --output-on-failure
```

SDL is statically linked, including its Android JNI implementation. Desktop
packages need no SDL DLL/dylib/so installation. OS libraries and frameworks
remain system requirements. Windows also uses the static MSVC runtime. macOS
packages contain an unsigned `.app`; signing/notarization is a separate step.

## Web and mobile

```sh
# Activate an Emscripten SDK first.
make web             # HTML/JS/Wasm plus a ZIP in dist/
python3 -m http.server 8000 --directory build/web/bin

# Android: JDK 17, Android SDK, accepted SDK licenses; Gradle downloads itself.
make android         # Debug APK and unsigned Release APK

# iOS: Xcode with an installed iOS Simulator runtime.
make ios             # Unsigned simulator app
python3 scripts/test_ios.py build/ios-simulator/bin/sotn-native.app
```

Android outputs are in `platforms/android/app/build/outputs/apk/`. Release
APK/device iOS bundles require your own signing credentials for installation
or distribution. Local Android SDK location can be set through `ANDROID_HOME`
or an ignored `platforms/android/local.properties` file.

## CI and releases

CI builds and runs the actual app on Linux, macOS, Windows, a headless browser,
an Android emulator, and an iOS Simulator. Desktop jobs cover Debug and Release.
Linux also runs the app and SDL under AddressSanitizer and UndefinedBehaviorSanitizer.
Desktop jobs run real OpenGL framebuffer checks; the browser also checks WebGL2.
Windows CI uses a checksum-pinned software driver isolated under `build/`;
it is never included in release packages. Mobile jobs compile the GLES path
and retain their platform rendering smoke tests.
Packages are uploaded as workflow artifacts. Tagged `vX.Y.Z` releases run the
same checks, require the tag to match `VERSION`, and attach the resulting ZIPs
and APKs to a GitHub Release. Device iOS signing remains a downstream task.

Tests need no game data. `SDL_VIDEODRIVER=dummy` drives the desktop smoke test;
browser/mobile tests render through their real platform backend.

## Private inputs and contribution policy

Keep private inputs outside the checkout or under ignored `local/`. Build and
package install rules explicitly select original app outputs and license files;
they never copy private inputs. `scripts/check_repository.py` checks the Git
index and blocks common ROM/BIOS/data formats, private directories, unexpected
binaries, symlinks, and oversized files. CI runs it on every change. The only
approved binary in the source tree is the checksum-verified Gradle wrapper.
These checks support source review; they cannot establish the provenance of
arbitrarily renamed text. Review every staged file before pushing.

Edit `VERSION` for the next release. Run `make fmt` for C formatting and
`python3 -m unittest discover -s tests` for the repository-policy tests.

Original scaffold code uses the [zlib license](LICENSE). See
[THIRD_PARTY.md](THIRD_PARTY.md) for dependency provenance.

The [native sound service](docs/laboratory-audio.md) now plays selected original
combat sound effects and Laboratory music; reverb and full audio parity remain incomplete.

Castle Entrance now includes selected enemies, Death’s opening event, and
[connected travel](docs/connected-stages.md) to/from Laboratory with stage-aware saves.
See [current scope](docs/entrance-runtime.md); this remains a development checkpoint.

The selected [Laboratory Maria encounter](docs/laboratory-maria.md) now runs its
original room script, actors, portraits and speech. [Relic collection](docs/relic-progression.md)
and permanent pickup bits survive ordinary saves and supported stage changes.

The [early castle development profiles](docs/early-castle-stages.md) connect Laboratory
to Marble Gallery and Outer Wall with shared actors, stage music, pickups and saves.
Their area-specific enemies, mechanisms and boss progression remain incomplete.
