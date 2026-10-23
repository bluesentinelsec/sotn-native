# OpenGL presentation and private graphics preview

`modules/render/` is an original C presentation layer using OpenGL 3.2 core
(GLSL 150) on desktop and OpenGL ES 3.0 (GLSL ES 300) on mobile/WebGL2. Shader
strings are embedded in `shaders.h` and compiled at runtime. No shader files,
game assets, or private generated headers are required in release packages.

This is a graphics foundation. It does not implement PS1 GPU commands, game
logic, movement, combat, or stage progression. The project app can preview
original save icons; gameplay remains in the separate upstream comparison
build until reviewed C logic and runtime-loaded game tables are integrated.

## Local checks

Build with `make`, then run the native executable:

```sh
build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --graphics-test
build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native \
  --game-data .tools/sotn-decomp/disks/us --preview-save-icons
```

On Linux use `build/debug/bin/sotn-native`; on Windows use `sotn-native.exe`.
The first command needs a working OpenGL driver and uses synthetic pixels.
The second requires the privately extracted, validated US reference files.
Left/Right cycles the 16 icons, Escape quits, and window resizing checks
presentation scaling. The three 16x16 frames of each icon appear stacked;
the viewer does not invent animation timing or alter the game.

## Rendering and ownership

The presenter owns its window, context, function pointers, and GPU objects.
SDL video initialization and shutdown belong to the app. Use the API only on
the main thread, and destroy presenters before `SDL_Quit`. Each context loads
its own function pointers and validates its minimum GL version before drawing.
Calls make their owning context current. Partial failures release resources
and preserve the operation's error message.

The caller supplies bounded top-to-bottom RGBA8 bytes. Uploads consume those
bytes before returning. Invalid inputs and failed uploads preserve the previous
texture. A presenter owns no game files, file paths, or simulation state.
The image budget is 64 MiB and the driver's texture limit is also enforced.

Presentation uses nearest filtering, integer scaling where the image fits,
and centered letterboxing. Smaller windows preserve the aspect ratio to the
nearest available pixel; even a 1x1 window has a valid viewport. Minimized
windows skip drawing. Straight-alpha blending handles preview transparency;
it does not stand in for PS1 primitive semi-transparency or dithering. SDL's
callback loop owns frame pacing, independently of this presentation layer.

The save-icon decoder uses pixel offsets from pinned `config/assets.us.yaml`
in sotn-decomp and the palette stride used by `src/save_mgr.h:GetSavePalette`.
These offsets, dimensions, and counts are metadata; all actual color and pixel
bytes come from the user's validated DRA view. It checks input/output bounds,
decodes little-endian RGB555 colors and low-nibble-first 4-bit indices, and
leaves output untouched on error.

## Automated checks

`make test` covers synthetic icon decoding, palette/alpha cases, truncation,
invalid arguments, integer/fractional scaling, tiny windows, and integer bounds.
These CPU tests also run under Linux ASan/UBSan without a graphics driver.

`python3 scripts/test_graphics.py <executable>` requires a real GL context and
checks compiled shaders, texture orientation, RGB values, transparency,
letterboxing, invalid-upload preservation, and sequential context recreation.
Both process success and the explicit completion marker are required. CI runs
this for Debug and Release on desktop: Xvfb/Mesa on Linux, native OpenGL on
macOS, and checksum-pinned Mesa llvmpipe on Windows. CI-only driver files live
under ignored `build/`, use a process-local library/PATH selection, and are
excluded by explicit package install rules.

The browser test runs both the bootstrap smoke test and `--graphics-test` under
WebGL2 in headless Chromium. Mobile jobs compile GLES headers and retain their
existing rendering smoke tests; the private preview currently has a desktop
keyboard interface. No CI check needs the user's game data. Shader compilation,
linking, context creation, and GL failures report actionable SDL errors.

## Separate upstream OpenGL comparison

The existing private comparison checkout now supports macOS OpenGL through
`tools/patches/psyz-macos-opengl.patch`. For a clean pinned checkout, apply it
from the root of the PSY-Z submodule with `git apply`, then configure the game
with `-DPSYZ_RENDERER=sdl3_gl -DENABLE_ASAN=OFF -B build/pc-gl`. Full extraction
prerequisites and pinned versions remain in [native-port-plan.md](native-port-plan.md).
The patch chooses existing C backend/loader sources and SDL's Khronos headers.
It imports no backend or game code into this project's app.

The current local checkout already has the patch and compiled comparison build:

```sh
(cd .tools/sotn-decomp && ./build/pc-gl/sotn --disk disks/sotn.us.cue --stage nz0 --player alu)
```

This is the Alchemy Laboratory comparison build with original game controls:
arrows move, X jumps, Z/D use the two hands, Enter pauses, Escape quits.
Compare colors, transparency, layering, room transitions, and responsiveness
with the previous SDL GPU prototype or the original game. Scripted demos 6
(the lab) and 0 (prologue) completed locally using dummy audio. This is an
incomplete upstream experiment containing private generated data; it must not
be published as our release or treated as whole-game parity evidence.
