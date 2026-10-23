# Third-party components

- **SDL 3.4.18**, zlib license. Downloaded from the official release archive by
  CMake FetchContent, with SHA-256 verification. The source remains in the
  ignored build tree. Packages include SDL's license under `licenses/SDL3.txt`.
  Source: https://github.com/libsdl-org/SDL/releases/tag/release-3.4.18
- **Gradle Wrapper 8.10.2**, Apache License 2.0. The wrapper scripts and JAR are
  build tooling from Gradle. The distribution is pinned with its official
  SHA-256 in `platforms/android/gradle/wrapper/gradle-wrapper.properties`.
  Source and license: https://github.com/gradle/gradle/tree/v8.10.2

No Konami game code, artwork, audio, disc images, BIOS images, or extracted
game data is included. The project license applies to original scaffold code;
third-party components retain their own licenses.

## Testing and private comparisons

- **mesa-dist-win 26.2.4 MSVC release**, used only for Windows CI's software
  OpenGL framebuffer check. Downloaded into ignored `build/test-tools/mesa`
  with SHA-256 `351fc8c8b695878ffb3eaa044b3ead08672a48b1a045e3c3e3975811df0f6695`.
  No driver binaries are tracked, installed system-wide, or packaged. The
  archive retains its upstream notices. Source and distribution:
  https://github.com/pal1000/mesa-dist-win/releases/tag/26.2.4
- `tools/patches/psyz-macos-opengl.patch` contains an original build adjustment
  for PSY-Z commit `b52003e55c0d1b20719e984dd19bd4d760eeb14a`, used only in the
  ignored upstream comparison checkout. It selects existing upstream files;
  it does not import those files or their dependencies into this app. PSY-Z's
  license inventory lists MPL 2.0 platform code, MIT decomp code in its own
  directory, and unlicensed header sections. Resolve each proposed import's
  provenance before integration. Inventory:
  https://github.com/Xeeynamo/psyz/blob/b52003e55c0d1b20719e984dd19bd4d760eeb14a/LICENSE
