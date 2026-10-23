# OpenGL checkpoint — 2026-10-23

Paused at the user's request on `feat/opengl-asset-preview`. This work is a
draft graphics foundation, not a completed room integration or gameplay build.
Resume here before extending the implementation or merging its draft PR.

## Saved work

- Original C OpenGL 3.2 / GLES 3.0 presentation code in `modules/render/`, with
  embedded shaders, context-local function pointers, integer image scaling,
  alpha blending, and deterministic framebuffer checks.
- An original bounds-checked decoder for the US DRA save-icon palettes and
  pixels. Only layout metadata is in source; actual graphics remain private.
- App flags `--graphics-test` and `--game-data DIR --preview-save-icons`.
  Left/Right selects icons in the preview; Escape quits. This previews assets,
  not gameplay. The original bootstrap remains the default.
- A small build patch in `tools/patches/psyz-macos-opengl.patch` enables the
  pinned PSY-Z OpenGL backend on macOS for a separate private comparison build.
  It selects the existing C backend/loader and SDL's Khronos include directory;
  it contains no game data. Upstream sources keep their own license notices.

## Verified locally

- `make test JOBS=6`: all six CTest groups passed on macOS arm64, including the
  existing data checks and synthetic image-decoding tests.
- The app's `--graphics-test` passed on the M1 Max: embedded shader compilation,
  framebuffer colors/orientation, transparency, letterboxing, and sequential
  context recreation. No game data was required for this check.
- The private upstream OpenGL comparison build compiled and scripted demos 6
  (Alchemy Laboratory) and 0 (prologue) both exited successfully with dummy audio.
  Logs identify OpenGL 4.1 on Apple M1 Max. This is not proof of whole-game parity.

## Resume tasks

1. Review presenter cleanup and error preservation, texture-upload failures,
   GL state assumptions, and tiny/minimized-window behavior. Verify the new
   preview against the local validated DRA data and inspect it visually.
2. Add real GL runtime coverage to CI. The current dummy-driver smoke test does
   not exercise OpenGL. Plan Linux Xvfb/Mesa, macOS, and checksum-pinned software
   OpenGL for the Windows runner. Do not package CI-only driver binaries.
3. Wire a WebGL2 graphics check into the browser test. The build now targets
   WebGL2, but browser/Android/iOS compilation and runtime checks have not been
   repeated for this branch. Check GLES compatibility and sanitizers.
4. Document the renderer/preview and upstream patch provenance, update release
   checks, and keep private inputs and diagnostics out of all public artifacts.
5. Finish review and CI before marking the draft PR ready. The next gameplay
   step still requires reviewed C logic and runtime-loaded data tables; no
   upstream game or SDK code has been imported into the project app here.

## Local commands

```sh
make test JOBS=6
build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native --graphics-test
build/debug/bin/sotn-native.app/Contents/MacOS/sotn-native \
  --game-data .tools/sotn-decomp/disks/us --preview-save-icons
```

The existing ignored upstream comparison checkout already has the patch applied
and a separate OpenGL build at `.tools/sotn-decomp/build/pc-gl`. It remains
outside the normal project build and release packaging. A future clean checkout
must apply the saved patch to PSY-Z commit
`b52003e55c0d1b20719e984dd19bd4d760eeb14a` before selecting `PSYZ_RENDERER=sdl3_gl`.
The pinned game sources and private extraction are described in
[native-port-plan.md](native-port-plan.md).
