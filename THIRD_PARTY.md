# Third-party components

- **SDL 3.4.18**, zlib license. Downloaded from the official release archive by
  CMake FetchContent, with SHA-256 verification. The source remains in the
  ignored build tree. Packages include SDL's license under `licenses/SDL3.txt`.
  Source: https://github.com/libsdl-org/SDL/releases/tag/release-3.4.18
  On macOS, `cmake/PatchSDLAudio.cmake` applies an original, hash-guarded playback
  shutdown adjustment. It avoids scheduling a redundant asynchronous stop before
  queue disposal, which reproduced a CoreAudio `-66671` warning under LLDB.
  Recording and other platforms retain the release implementation.
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
  `modules/core/room_actors.c` adapts scrolling spawns from `src/st/create_entity.h`
  and stage persistence from `src/st/st_common.h`; the runtime layout parser
  is original project code. See [actor integration](docs/laboratory-actors.md).
  `modules/render/actor_graphics.c` adapts bounded graphics-bank uploads,
  bulk palettes, and multipart drawing from `src/dra/4AEA4.c`, `4A538.c`,
  and `4B758.c`.
  `modules/core/actors.c` adapts selected intact object and active Bloody Zombie
  paths from `src/st/nz0/e_breakable.c`, `e_red_eye_bust.c`, `e_nz0_room2.c`,
  `src/st/e_bloody_zombie.h`, `animate_entity.h`, `st_common.h`, and `st_update.h`.
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

The empty-hand player dependency additionally adapts the selected
`EntityUnarmedAttack` and `UpdateUnarmedAnim` paths in `src/dra/7879C.c` and
`alu_anim.c` at the same pinned reconstructed C revision, under
AGPL-3.0-or-later. Their animation/hitbox tables remain private runtime data.

The selected Laboratory combat paths additionally adapt `src/st/collision.h`,
`e_collect.h`, `collect_subweapon.h`, and `src/dra/5D5BC.c` (`DealDamage`),
including original physical player-hit paths in `71830.c`. They retain
AGPL-3.0-or-later attribution at the same pinned revision. Game tables and
sound samples are not included.

## Selected native sound service

`modules/audio/sound.c` adapts selected original event/script/channel paths in
`src/dra/sound.c` under AGPL-3.0-or-later at the pinned game reconstruction.
Its pitch/volume service follows PSY-Z's MIT-licensed `decomp/src/libsnd/vm_n2p.c`
and `vm_nowon.c`; see `licenses/PSY-Z-decomp-MIT.txt`.

`modules/audio/voice.h` adapts selected decoding/resampling/envelope code from
[PSY-Z](https://github.com/Xeeynamo/psyz) revision
`b52003e55c0d1b20719e984dd19bd4d760eeb14a`,
`psyz/src/psyz/spu_voice.h`, under MPL-2.0. The full license ships as
`licenses/MPL-2.0.txt`. `gauss.h` retains the source's public-domain declaration
for its mathematical coefficients. No SDK/platform headers, original samples,
or game instrument/event tables are included.

`modules/audio/music.c` additionally adapts selected XA decoding paths in
PSY-Z's `psyz/src/psyz/libcd.c` at the same backend revision under MPL-2.0.
The native stream loader and SDL rate-conversion boundary replace platform/SDK
services. No game music, CD sectors, or soundtrack metadata arrays are included.

Selected Gladius/Mace integration additionally adapts `src/weapon/w_000.c`,
`w_002.c`, `shared.h`, and selected `src/dra/menu.c:CalcAttack` and
`5D5BC.c:GetEquipProperties` paths at the pinned reconstruction revision under
AGPL-3.0-or-later. Equipment records, weapon overlays, frame/hitbox tables,
palettes, and pixels remain private runtime inputs.

Original HUD presentation adapts selected `src/dra/5F60C.c` HUD/counter paths
and `4A538.c` palette behavior at the same pinned reconstruction revision under
AGPL-3.0-or-later. Layout, glyphs, artwork, palettes, and animation definitions
remain private runtime data; none are compiled into public binaries.

Selected physical death and game-over presentation additionally adapt
`src/dra/71830.c:PlayerStepKill` and `game_handlers.c:HandleGameOver`/
`func_800E5358` at the pinned reconstruction revision under AGPL-3.0-or-later.
Game-over bitmaps, palettes, animation frames, and music remain private inputs.

Selected save-point activation/centering/coffin clocks and mesh presentation adapt
`src/dra/62DEC.c`, `6DF70.c:func_8010E334`, and `menu.c:MenuDrawChar` at the same
pinned reconstruction revision under AGPL-3.0-or-later. Meshes, UV arrays, font,
texture pixels, and palette records remain private runtime inputs. Native storage
and slot-selection adapters are original project code. See
[save-room scope](docs/laboratory-save-rooms.md) for parity and platform limits.

`modules/core/character.c` adapts selected inventory acquisition/order, equipment
exchange, and unbuffed attribute calculations from `src/dra/5D5BC.c` and `menu.c`
at the pinned reconstruction revision under AGPL-3.0-or-later. Item records,
names, descriptions, and icons remain private runtime inputs. Unsupported item
behaviors are rejected at the equipment boundary.

Selected native equipment/pause panels, fonts, portrait, and icon loading adapt
`src/dra/menu.c` and `4AEA4.c`; cape palette selection adapts `5F60C.c`. Original
equipment drops, persistent room pickups, and breakable globe paths additionally
adapt `src/st/e_collect.h`, `nz0/e_nz0_room2.c`, and `st_common.h`. Basilard adapts
selected `src/weapon/w_031.c` behavior. All use the same pinned reconstruction
under AGPL-3.0-or-later. Private runtime definitions/pixels/palettes remain untracked.
