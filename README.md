# SOTN Native

A C17/SDL3 native-port project for personal and educational study of
*Castlevania: Symphony of the Night*. **Game integration is in progress.**
The project's executable now runs a limited combat checkpoint in the Alchemy
Laboratory: Alucard movement, adjacent room transitions, animated candles,
red-eye busts, globe tables, selected empty-hand/Gladius/Mace combat, candle pickups,
the original Alucard HUD, physical death/game over, native save-room support,
and original sound effects/Laboratory music.
Other equipped weapons, complete damage/item/audio behavior, full inventory/progression saves, actor/effect
support, stage scripts, and full-game progression remain incomplete. This is not yet an
authentic playable game build. Its default mode still displays the bootstrap greeting.

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
make && lldb -o run -- build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --game-data /path/to/private/extracted/us --laboratory --laboratory-room 3
```

Use arrows to move/look/crouch, **X or Space** to jump, and Escape to quit.
Xbox controls use the d-pad or left stick and **A** to jump. **Z/D** or
Xbox **X/B** use the equipped hands (default: Gladius left, empty right). Ability inputs remain unsupported.
At a crash, enter `thread backtrace all` in LLDB and share its output.

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
files, ownership contract, and current limits. Normal builds and tests need no
game inputs.

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
make                 # Debug build in build/debug
make test            # Run the app's headless render smoke test and repository guard
make sanitizer       # Linux: run tests with ASan and UBSan, including SDL instrumentation
make package         # Build/test Release and produce a versioned ZIP in dist/
```

Use the LLDB command above for game playtests. On Linux, use
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
