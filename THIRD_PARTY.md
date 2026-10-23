# Third-party components

- **SDL 3.4.18**, zlib license. Downloaded from the official release archive by
  CMake FetchContent, with SHA-256 verification. The source remains in the
  ignored build tree. Packages include SDL's license under `licenses/SDL3.txt`.
  Source: https://github.com/libsdl-org/SDL/releases/tag/release-3.4.18
- **Gradle Wrapper 8.10.2**, Apache License 2.0. The wrapper scripts and JAR are
  build tooling from Gradle. The distribution is pinned with its official
  SHA-256 in `platforms/android/gradle/wrapper/gradle-wrapper.properties`.
  Source and license: https://github.com/gradle/gradle/tree/v8.10.2

- **Reconstructed SoTN game algorithms**, AGPL-3.0-or-later, adapted from
  sotn-decomp commit `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`.
  `modules/core/collision.c` derives from `src/dra/collider.c`;
  `modules/core/decompress.c` derives from the graphics decoder in
  `src/dra/4AEA4.c`. `modules/core/animation.c` adapts `src/dra/alu_anim.c`.
  `modules/core/player.c` adapts normal-form movement paths from
  `src/dra/6DF70.c`, `6E42C.c`, `6FDF8.c`, `71830.c`, and `692E8.c`;
  `modules/core/player_sensors.c` adapts `6BF64.c`, `70394.c`, and the
  normal-form collision path in `692E8.c`. See
  [movement integration](docs/laboratory-movement.md) for scope and validation.
  `modules/render/tilemap.c` adapts the tile renderer in `src/dra/4CE2C.c` and
  tile upload layout in `src/pc/sim_pc.c`. `modules/render/alucard_sprite.c`
  adapts the sprite-sheet path in `src/dra/4B758.c` and ARC pointer loading in
  `src/pc/pl_arc.c`. Each file identifies its origin. Adaptations are described
  in [game-core integration](docs/game-core.md) and
  [runtime presentation](docs/laboratory-presentation.md). Packages include the
  [full license](licenses/AGPL-3.0-or-later.txt); binaries linking this code are
  subject to AGPL requirements, including corresponding source availability.
  Source: https://github.com/Xeeynamo/sotn-decomp/tree/7bc54d8157b6bed77e6bd15e5db9461a734fe1e5

No original game source dumps, artwork, audio, disc images, BIOS images, or
extracted game data are included. The project license applies to original
scaffold code; adapted components retain their stated licenses.

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
