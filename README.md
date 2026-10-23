# SOTN Native

An original, asset-free SDL3 bootstrap for a future personal and educational
native-port study of *Castlevania: Symphony of the Night*. **This is currently
a Hello world app; it does not run or convert the game.** It logs `Hello world`
through `SDL_Log` and renders the same greeting in a window or browser canvas.

No ROM, BIOS, extracted data, game code, artwork, or audio is distributed.
Future game functionality will require users to supply their own game dump.
This project is unofficial and is not affiliated with Konami or Sony.

The project uses **C17**. Phase 1 preserves the original game experience, with
desktop correctness established first and required web/mobile adaptations kept
explicit. Modern conveniences follow a proven authentic foundation. See
[project goals](docs/project-goals.md) for release direction and
[AGENTS.md](AGENTS.md) for engineering and collaboration standards.

## Desktop

Requires CMake 3.24+, a C17 compiler, GNU Make, Python 3, and the platform SDK.
The first configure downloads checksum-pinned **SDL 3.4.18** from its official
release. On macOS, install Xcode's command-line tools; on Windows, use a Visual
Studio developer shell. Windows users can run the CMake commands directly.

```sh
make                 # Debug build in build/debug
make run             # Open the app; Escape or the window close button quits
make test            # Run the app's headless render smoke test and repository guard
make sanitizer       # Linux: run tests with ASan and UBSan, including SDL instrumentation
make package         # Build/test Release and produce a versioned ZIP in dist/
```

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
